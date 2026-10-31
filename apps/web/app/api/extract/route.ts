
import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const ENGINE_URL =
  process.env.PAGEGOAT_ENGINE_URL?.trim() ||
  "http://127.0.0.1:8787";

const ENGINE_TIMEOUT_MS = 204_000;

const ENGINE_SECRET = process.env.PAGEGOAT_ENGINE_SECRET?.trim();

function getErrorMessage(payload: unknown): string | null {
  if (
    !payload ||
    typeof payload !== "object" ||
    Array.isArray(payload)
  ) {
    return null;
  }

  const data = payload as Record<string, unknown>;

  for (const key of ["error", "message", "detail"]) {
    const value = data[key];

    if (typeof value === "string" && value.trim()) {
      return value.trim();
    }
  }

  return null;
}

export async function POST(request: Request) {
  let body: unknown;

  try {
    body = await request.json();
  } catch {
    return NextResponse.json(
      { error: "Request body must be valid JSON." },
      { status: 400 }
    );
  }

  if (
    !body ||
    typeof body !== "object" ||
    Array.isArray(body)
  ) {
    return NextResponse.json(
      { error: "A JSON object containing a company URL is required." },
      { status: 400 }
    );
  }

  const url = (body as { url?: unknown }).url;

  if (
    typeof url !== "string" ||
    !url.trim() ||
    url.trim().length > 2048
  ) {
    return NextResponse.json(
      { error: "Enter a company website or domain." },
      { status: 400 }
    );
  }

  try {
    const upstream = await fetch(
      `${ENGINE_URL.replace(/\/+$/, "")}/extract`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Accept: "application/json",
          "X-PageGoat-Secret": ENGINE_SECRET || "",
        },
        body: JSON.stringify({ url: url.trim() }),
        cache: "no-store",
        signal: AbortSignal.timeout(ENGINE_TIMEOUT_MS),
      }
    );

    const raw = await upstream.text();

    let payload: unknown;

    try {
      payload = JSON.parse(raw);
    } catch {
      if (!upstream.ok) {
        return NextResponse.json(
          {
            error:
              raw.trim() ||
              `The extraction engine returned HTTP ${upstream.status}.`,
          },
          {
            status:
              upstream.status >= 400 && upstream.status < 600
                ? upstream.status
                : 502,
          }
        );
      }

      return NextResponse.json(
        {
          error:
            "The extraction engine returned an invalid JSON response.",
          upstreamStatus: upstream.status,
        },
        { status: 502 }
      );
    }

    if (!upstream.ok) {
      return NextResponse.json(
        {
          error:
            getErrorMessage(payload) ||
            `The extraction engine returned HTTP ${upstream.status}.`,
        },
        {
          status:
            upstream.status >= 400 && upstream.status < 600
              ? upstream.status
              : 502,
        }
      );
    }

    return NextResponse.json(payload, {
      status: upstream.status,
    });
  } catch (error) {
    const timedOut =
      error instanceof Error &&
      (error.name === "TimeoutError" ||
        error.name === "AbortError");

    const message =
      error instanceof Error
        ? error.message
        : "Unknown connection error.";

    return NextResponse.json(
      {
        error: timedOut
          ? "Extraction exceeded the 3 minute 24 second limit. The engine may still be processing."
          : `Could not connect to the extraction engine: ${message}`,
      },
      { status: timedOut ? 504 : 502 }
    );
  }
}
