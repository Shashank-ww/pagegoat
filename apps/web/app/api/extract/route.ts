
import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const ENGINE_URL =
  process.env.PAGEGOAT_ENGINE_URL?.trim() ||
  "http://127.0.0.1:8787";

export async function POST(request: Request) {
  let body: unknown;

  try {
    body = await request.json();
  } catch {
    return NextResponse.json(
      { error: "Request body must be valid JSON." },
      { status: 400 },
    );
  }

  if (!body || typeof body !== "object" || Array.isArray(body)) {
    return NextResponse.json(
      { error: "A JSON object containing a company URL is required." },
      { status: 400 },
    );
  }

  const url = (body as { url?: unknown }).url;

  if (typeof url !== "string" || !url.trim() || url.trim().length > 2048) {
    return NextResponse.json(
      { error: "Enter a company website or domain." },
      { status: 400 },
    );
  }

  try {
    const upstream = await fetch(`${ENGINE_URL.replace(/\/+$/, "")}/extract`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Accept: "application/json",
      },
      body: JSON.stringify({ url: url.trim() }),
      cache: "no-store",
      signal: AbortSignal.timeout(120_000),
    });

    const raw = await upstream.text();

    let payload: unknown;

    try {
      payload = JSON.parse(raw);
    } catch {
      return NextResponse.json(
        {
          error: "The extraction engine returned an invalid JSON response.",
          upstreamStatus: upstream.status,
        },
        { status: 502 },
      );
    }

    if (!upstream.ok) {
      const upstreamPayload =
        payload && typeof payload === "object" && !Array.isArray(payload)
          ? (payload as Record<string, unknown>)
          : {};

      return NextResponse.json(
        {
          error:
            typeof upstreamPayload.error === "string"
              ? upstreamPayload.error
              : `The extraction engine returned HTTP ${upstream.status}.`,
        },
        { status: upstream.status >= 400 && upstream.status < 600 ? upstream.status : 502 },
      );
    }

    // Pass the engine response through unchanged. Preserve nested company fields.
    return NextResponse.json(payload, { status: 200 });
  } catch (error) {
    const message =
      error instanceof Error ? error.message : "Unknown connection error.";

    const timedOut =
      error instanceof Error &&
      (error.name === "TimeoutError" || error.name === "AbortError");

    return NextResponse.json(
      {
        error: timedOut
          ? "The extraction engine took too long to respond."
          : `Could not connect to the extraction engine: ${message}`,
      },
      { status: timedOut ? 504 : 502 },
    );
  }
}
