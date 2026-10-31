import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const ENGINE_URL =
  process.env.PAGEGOAT_ENGINE_URL?.trim() || "http://127.0.0.1:8787";
const ENGINE_TIMEOUT_MS = 204_000;

const ENGINE_SECRET = process.env.PAGEGOAT_ENGINE_SECRET?.trim();

function getErrorMessage(payload: unknown): string | null {
  if (!payload || typeof payload !== "object" || Array.isArray(payload)) return null;
  const data = payload as Record<string, unknown>;
  for (const key of ["error", "message", "detail"]) {
    const value = data[key];
    if (typeof value === "string" && value.trim()) return value.trim();
  }
  return null;
}

export async function POST(request: Request) {
  let body: unknown;
  try {
    body = await request.json();
  } catch {
    return NextResponse.json({ error: "Request body must be valid JSON." }, { status: 400 });
  }

  if (!body || typeof body !== "object" || Array.isArray(body)) {
    return NextResponse.json(
      { error: "A JSON object containing a company URL is required." },
      { status: 400 },
    );
  }

  const url = (body as { url?: unknown }).url;
  if (typeof url !== "string" || !url.trim() || url.trim().length > 2048) {
    return NextResponse.json({ error: "Enter a company website or domain." }, { status: 400 });
  }

  if (!ENGINE_SECRET) {
  return NextResponse.json(
    { error: "Engine authentication is not configured." },
    { status: 500 }
  );
}
  try {
    const upstream = await fetch(`${ENGINE_URL.replace(/\/+$/, "")}/extract-stream`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Accept: "text/event-stream",
        "X-PageGoat-Secret": ENGINE_SECRET || "",
      },
      body: JSON.stringify({ url: url.trim() }),
      cache: "no-store",
      signal: AbortSignal.timeout(ENGINE_TIMEOUT_MS),
    });

    if (!upstream.ok) {
      const raw = await upstream.text();
      let payload: unknown = null;
      try {
        payload = JSON.parse(raw);
      } catch {
        // Preserve plain-text upstream errors in the JSON response.
      }
      return NextResponse.json(
        { error: getErrorMessage(payload) || raw.trim() || `The extraction engine returned HTTP ${upstream.status}.` },
        { status: upstream.status >= 400 && upstream.status < 600 ? upstream.status : 502 },
      );
    }

    if (!upstream.body) {
      return NextResponse.json({ error: "The extraction engine returned an empty activity stream." }, { status: 502 });
    }

    return new Response(upstream.body, {
      status: 200,
      headers: {
        "Content-Type": "text/event-stream; charset=utf-8",
        "Cache-Control": "no-cache, no-transform",
        "X-Accel-Buffering": "no",
      },
    });
  } catch (error) {
    const timedOut = error instanceof Error && (error.name === "TimeoutError" || error.name === "AbortError");
    const message = error instanceof Error ? error.message : "Unknown connection error.";
    return NextResponse.json(
      { error: timedOut
          ? "Extraction exceeded the 3 minute 24 second limit. The engine may still be processing."
          : `Could not connect to the extraction engine: ${message}` },
      { status: timedOut ? 504 : 502 },
    );
  }
}
