import { FileText, ShieldCheck, TerminalSquare } from "lucide-react";

export default function TermsPage() {
  return (
    <main className="min-h-screen bg-[#f4f5f1] text-[#171a17]">
      <header className="border-b border-[#d9ddd5] bg-[#f8f9f6]">
        <div className="mx-auto flex h-16 max-w-300 items-center justify-between px-5">
          <a
            href="/"
            className="flex items-center gap-2 text-lg font-bold tracking-[-0.04em]"
          >
            <span className="flex h-7 w-7 items-center justify-center border border-[#171a17] bg-[#b7ef68]">
              <TerminalSquare size={15} />
            </span>
            <span>
              page<span className="text-[#638e32]">goat</span>
            </span>
          </a>

          <a
            href="/"
            className="text-[10px] font-bold uppercase tracking-[0.15em] text-[#687168]"
          >
            ← Back to engine
          </a>
        </div>
      </header>

      <section className="mx-auto max-w-212 px-5 py-16 md:py-24">
        <div className="flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.2em] text-[#638e32]">
          <FileText size={13} />
          Terms
        </div>

        <h1 className="mt-4 text-4xl font-semibold tracking-tighter md:text-6xl">
          Terms of use.
        </h1>

        <div className="mt-12 space-y-10 text-sm leading-7 text-[#626b62]">
          <section>
            <h2 className="mb-3 text-base font-bold text-[#171a17]">
              01 — Service
            </h2>

            <p>
              PageGoat provides a website extraction and company intelligence
              service. Results are generated from information available to
              the engine at the time of extraction.
            </p>
          </section>

          <section>
            <h2 className="mb-3 text-base font-bold text-[#171a17]">
              02 — Accuracy
            </h2>

            <p>
              Website information can be incomplete, outdated, inaccurate or
              intentionally obscured. PageGoat does not guarantee that every
              result is complete or correct.
            </p>
          </section>

          <section>
            <h2 className="mb-3 text-base font-bold text-[#171a17]">
              03 — Public information
            </h2>

            <p>
              The engine is intended to process publicly accessible website
              information. Users are responsible for using extracted
              information lawfully and respectfully.
            </p>
          </section>

          <section>
            <h2 className="mb-3 text-base font-bold text-[#171a17]">
              04 — No generated facts
            </h2>

            <p>
              PageGoat does not use generative AI to manufacture company
              facts. The system is a proprietary extraction and processing
              engine. Any result should nevertheless be independently
              verified before being relied upon for consequential decisions.
            </p>
          </section>

          <section className="border border-[#d3d8d0] bg-[#eef0eb] p-5">
            <div className="flex items-start gap-3">
              <ShieldCheck
                size={16}
                className="mt-1 text-[#638e32]"
              />

              <div>
                <h2 className="text-sm font-bold text-[#171a17]">
                  Important
                </h2>

                <p className="mt-2 text-xs leading-6 text-[#737b73]">
                  These terms are a starting point for the product and should
                  be reviewed and adapted with appropriate legal advice before
                  being treated as your final terms of service.
                </p>
              </div>
            </div>
          </section>
        </div>
      </section>
    </main>
  );
}