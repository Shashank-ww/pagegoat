import {
  ArrowUpRight,
  Mail,
  MessageSquare,
  TerminalSquare,
} from "lucide-react";

export default function ContactPage() {
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
        <p className="mb-4 text-[10px] font-bold uppercase tracking-[0.2em] text-[#638e32]">
          Contact
        </p>

        <h1 className="text-4xl font-semibold tracking-tighter md:text-6xl">
          Talk to the people
          <br />
          behind the engine.
        </h1>

        <p className="mt-6 max-w-2xl text-sm leading-7 text-[#697169]">
          Questions about the extraction engine, data quality, partnerships,
          or something that looks wrong in a result? Send a message.
        </p>

        <div className="mt-12 border border-[#d3d8d0] bg-white">
          <div className="border-b border-[#dfe3dd] bg-[#eef0eb] px-5 py-3">
            <div className="flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.15em]">
              <MessageSquare size={13} />
              Direct contact
            </div>
          </div>

          <div className="p-6">
            <div className="flex items-start gap-4">
              <div className="flex h-9 w-9 items-center justify-center border border-[#d2d7cf] bg-[#f4f5f1]">
                <Mail size={16} />
              </div>

              <div>
                <p className="text-xs font-semibold">
                  General enquiries
                </p>

                <a
                  href="mailto:hello@pagegoat.com"
                  className="mt-1 flex items-center gap-1 text-sm text-[#638e32] hover:underline"
                >
                  hello@pagegoat.com
                  <ArrowUpRight size={12} />
                </a>

                <p className="mt-3 text-[11px] leading-5 text-[#899189]">
                  Replace this address with your actual support/contact
                  address before deploying.
                </p>
              </div>
            </div>
          </div>
        </div>
      </section>
    </main>
  );
}