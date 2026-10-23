import {
  Code2,
  Database,
  Globe2,
  ShieldCheck,
  TerminalSquare,
} from "lucide-react";

export default function AboutPage() {
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

      <section className="mx-auto max-w-250 px-5 py-16 md:py-24">
        <p className="mb-4 text-[10px] font-bold uppercase tracking-[0.2em] text-[#638e32]">
          About PageGoat
        </p>

        <h1 className="max-w-3xl text-4xl font-semibold tracking-tighter md:text-6xl">
          A machine for reading
          <br />
          the public web.
        </h1>

        <p className="mt-6 max-w-2xl text-sm leading-7 text-[#697169]">
          PageGoat is an independent company intelligence engine designed
          to turn publicly accessible website information into a structured,
          readable record.
        </p>

        <div className="mt-14 grid gap-px border border-[#d3d8d0] bg-[#d3d8d0] md:grid-cols-3">
          <InfoCard
            icon={<Globe2 size={17} />}
            title="Web first"
            text="The engine starts with the source itself rather than a generated answer."
          />

          <InfoCard
            icon={<Database size={17} />}
            title="Structured"
            text="Information is organized into company, contact, social, branding and metadata records."
          />

          <InfoCard
            icon={<Code2 size={17} />}
            title="Built in-house"
            text="The extraction system is proprietary software developed specifically for this purpose."
          />
        </div>

        <div className="mt-12 border-t border-[#d9ddd5] pt-8">
          <div className="flex items-start gap-3">
            <ShieldCheck
              size={16}
              className="mt-0.5 text-[#638e32]"
            />

            <div>
              <h2 className="text-sm font-semibold">
                No AI-generated company facts
              </h2>

              <p className="mt-2 max-w-2xl text-xs leading-6 text-[#7b837b]">
                PageGoat does not use generative AI to invent, infer or
                fabricate company information. Its purpose is extraction:
                retrieving publicly accessible information and presenting
                the signals it can identify.
              </p>
            </div>
          </div>
        </div>
      </section>
    </main>
  );
}

function InfoCard({
  icon,
  title,
  text,
}: {
  icon: React.ReactNode;
  title: string;
  text: string;
}) {
  return (
    <div className="bg-[#f8f9f6] p-6">
      <div className="mb-5 text-[#638e32]">{icon}</div>

      <h2 className="text-sm font-bold">{title}</h2>

      <p className="mt-2 text-xs leading-5 text-[#7b837b]">
        {text}
      </p>
    </div>
  );
}