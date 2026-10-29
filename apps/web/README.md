Welookit / PageGoat streaming and metadata update

Files in this patch:
- app/page.tsx: updated page with real SSE activity events and updated metadata/font mappings.
- app/api/extract-stream/route.ts: new Next.js route that streams /extract-stream from the C engine.
- globals.css.append.css: CSS additions only. Append this file's contents to the END of your existing app/globals.css. Do not replace globals.css.

Apply:
1. Back up your current frontend changes through Git as usual (no project replacement needed).
2. Copy app/page.tsx over your existing app/page.tsx.
3. Add app/api/extract-stream/route.ts at the matching path.
4. Append globals.css.append.css to the end of app/globals.css. Existing CSS comments and styles are not removed.
5. Keep the existing app/api/extract/route.ts unchanged as the JSON endpoint/fallback for any other callers.
6. Ensure PAGEGOAT_ENGINE_URL points to the reachable engine host in deployment; localhost is only a local default.
7. Run npm run build and test a lookup. Verify activity events appear as received, results render, fonts are capped to four, and SEO fields appear only when returned.

The route assumes the C engine's /extract-stream endpoint emits SSE event names: activity, result, complete, and error. It passes the event stream through without buffering. This does not change the C engine.
