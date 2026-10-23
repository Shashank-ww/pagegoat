
import { NextResponse } from "next/server";

export const runtime = "nodejs";
export const maxDuration = 90;

export async function POST(request: Request) {
  const engineUrl = process.env.PAGEGOAT_ENGINE_URL;

  if (!engineUrl) {
    return NextResponse.json(
      { error: "The PageGoat extraction service is not configured yet." },
      { status: 503 }
    );
  }

  try {
    const body = await request.json();
    const url = body?.url;

    if (typeof url !== "string" || !url.trim() || url.length > 2048) {
      return NextResponse.json(
        { error: "Please enter a valid website URL." },
        { status: 400 }
      );
    }

    const response = await fetch(`${engineUrl.replace(/\/$/, "")}/extract`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ url: url.trim() }),
      signal: AbortSignal.timeout(85_000),
      cache: "no-store",
    });

    const result = await response.json();

    if (!response.ok) {
      return NextResponse.json(
        { error: result.error || "Website extraction failed." },
        { status: response.status >= 400 && response.status < 600
          ? response.status
          : 502 }
      );
    }

    return NextResponse.json(result, { status: 200 });
  } catch {
    return NextResponse.json(
      { error: "Could not reach the PageGoat extraction service." },
      { status: 502 }
    );
  }
}
