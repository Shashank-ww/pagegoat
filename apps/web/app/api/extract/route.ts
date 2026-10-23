import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const maxDuration = 90;

export async function POST(request: Request) {
  const engineUrl = process.env.PAGEGOAT_ENGINE_URL;

  if (!engineUrl) {
    return NextResponse.json(
      {
        error:
          "The PageGoat extraction service is not configured yet.",
      },
      { status: 503 }
    );
  }

  const startedAt = Date.now();

  try {
    const body = await request.json();
    const rawUrl = body?.url;

    if (
      typeof rawUrl !== "string" ||
      !rawUrl.trim() ||
      rawUrl.length > 2048
    ) {
      return NextResponse.json(
        {
          error: "Please enter a valid website URL.",
        },
        { status: 400 }
      );
    }

    const url = rawUrl.trim();

    const endpoint = `${engineUrl.replace(/\/$/, "")}/extract`;

    const response = await fetch(endpoint, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ url }),
      signal: AbortSignal.timeout(85_000),
      cache: "no-store",
    });

    let result: Record<string, unknown>;

    try {
      result = await response.json();
    } catch {
      return NextResponse.json(
        {
          error: "The extraction service returned an invalid response.",
        },
        { status: 502 }
      );
    }

    if (!response.ok) {
      return NextResponse.json(
        {
          error:
            typeof result.error === "string"
              ? result.error
              : "Website extraction failed.",
        },
        {
          status:
            response.status >= 400 && response.status < 600
              ? response.status
              : 502,
        }
      );
    }

    return NextResponse.json(
      {
        ...result,
        engine: {
          ...(typeof result.engine === "object" &&
          result.engine !== null
            ? result.engine
            : {}),
          elapsed_ms: Date.now() - startedAt,
        },
      },
      { status: 200 }
    );
  } catch (error) {
    const timedOut =
      error instanceof DOMException &&
      error.name === "TimeoutError";

    return NextResponse.json(
      {
        error: timedOut
          ? "The extraction service took too long to respond."
          : "Could not reach the PageGoat extraction service.",
      },
      { status: timedOut ? 504 : 502 }
    );
  }
}