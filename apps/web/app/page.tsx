"use client";

import {
  Activity,
  ArrowUpRight,
  BookOpen,
  Building2,
  Check,
  ChevronRight,
  Clock3,
  Code2,
  Database,
  ExternalLink,
  Globe2,
  History,
  Info,
  Link2,
  Mail,
  Menu,
  Network,
  Phone,
  Radar,
  RefreshCw,
  Search,
  Server,
  ShieldCheck,
  Sparkles,
  TerminalSquare,
  X,
} from "lucide-react";
import { FormEvent, useEffect, useMemo, useState } from "react";

type CompanyData = {
  website?: string;
  company?: {
    name?: string;
    title?: string;
    description?: string;
    language?: string;
    canonical_url?: string;
    keywords?: string;
    contact?: {
      emails?: string[];
      phones?: string[];
    };
    social?: Record<string, string>;
    branding?: Record<string, string>;
    seo?: Record<string, string>;
    organization?: Record<string, string>;
  };
};

type SearchRecord = {
  url: string;
  timestamp: number;
};

const HISTORY_KEY = "pagegoat-search-history";

const ENGINE_STEPS = [
  {
    id: "connect",
    label: "Establishing connection",
    detail: "Opening target",
    icon: Globe2,
  },
  {
    id: "document",
    label: "Reading document",
    detail: "Parsing page structure",
    icon: BookOpen,
  },
  {
    id: "signals",
    label: "Collecting signals",
    detail: "Metadata, links & identifiers",
    icon: Radar,
  },
  {
    id: "contacts",
    label: "Resolving contacts",
    detail: "Email & telephone patterns",
    icon: Network,
  },
  {
    id: "identity",
    label: "Building company record",
    detail: "Normalizing available data",
    icon: Building2,
  },
];

function formatTime(timestamp: number) {
  const date = new Date(timestamp);

  return new Intl.DateTimeFormat("en", {
    hour: "2-digit",
    minute: "2-digit",
  }).format(date);
}

function cleanUrl(value: string) {
  return value
    .replace(/^https?:\/\//, "")
    .replace(/^www\./, "")
    .replace(/\/$/, "");
}

function externalUrl(value: string) {
  if (/^https?:\/\//i.test(value)) {
    return value;
  }

  return `https://${value}`;
}

export default function Home() {
  const [url, setUrl] = useState("");
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");
  const [data, setData] = useState<CompanyData | null>(null);
  const [history, setHistory] = useState<SearchRecord[]>([]);
  const [activeStep, setActiveStep] = useState(0);
  const [completedSteps, setCompletedSteps] = useState<number[]>([]);
  const [mobileHistory, setMobileHistory] = useState(false);

  useEffect(() => {
    try {
      const saved = localStorage.getItem(HISTORY_KEY);

      if (saved) {
        setHistory(JSON.parse(saved));
      }
    } catch {
      // Ignore invalid local history.
    }
  }, []);

  useEffect(() => {
    if (!loading) return;

    setActiveStep(0);
    setCompletedSteps([]);

    const timers = ENGINE_STEPS.map((_, index) => {
      return window.setTimeout(() => {
        setActiveStep(index);

        if (index > 0) {
          setCompletedSteps((previous) => [
            ...new Set([...previous, index - 1]),
          ]);
        }
      }, index * 850);
    });

    return () => timers.forEach(window.clearTimeout);
  }, [loading]);

  function saveHistory(value: string) {
    const normalized = cleanUrl(value);

    const record: SearchRecord = {
      url: normalized,
      timestamp: Date.now(),
    };

    const next = [
      record,
      ...history.filter((item) => item.url !== normalized),
    ].slice(0, 8);

    setHistory(next);

    try {
      localStorage.setItem(HISTORY_KEY, JSON.stringify(next));
    } catch {
      // Local storage is optional.
    }
  }

  // IMPORTANT:
  // Keep this search flow untouched.
  // No client-side URL validation is performed here.
  async function handleSearch(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();

    const target = url.trim();

    if (!target) return;

    setLoading(true);
    setError("");
    setData(null);

    try {
      const response = await fetch("/api/extract", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          url: target,
        }),
      });

      const result = await response.json();

      if (!response.ok) {
        throw new Error(result.error || "Extraction failed.");
      }

      setCompletedSteps(ENGINE_STEPS.map((_, index) => index));
      setActiveStep(ENGINE_STEPS.length - 1);

      setData(result);
      saveHistory(target);
    } catch (err) {
      setError(
        err instanceof Error
          ? err.message
          : "Unable to extract this website."
      );
    } finally {
      setLoading(false);
    }
  }

  function runHistorySearch(value: string) {
    setUrl(value);
    setMobileHistory(false);

    window.setTimeout(() => {
      document.getElementById("search-form")?.scrollIntoView({
        behavior: "smooth",
        block: "center",
      });
    }, 0);
  }

  function clearHistory() {
    setHistory([]);

    try {
      localStorage.removeItem(HISTORY_KEY);
    } catch {
      // Ignore storage failures.
    }
  }

  return (
    <main className="min-h-screen bg-[#f4f5f1] text-[#171a17]">
      {/* Utility bar */}
      <div className="border-b border-[#d9ddd5] bg-[#eef0eb]">
        <div className="mx-auto flex h-8 max-w-375 items-center justify-between px-4 text-[10px] font-medium uppercase tracking-[0.14em] text-[#697168] md:px-6">
          <div className="flex items-center gap-3">
            <span>PAGEGOAT ENGINE</span>

            <span className="hidden h-3 w-px bg-[#c9cec6] sm:block" />

            <span className="hidden sm:block">
              Company Intelligence System
            </span>
          </div>

          <div className="flex items-center gap-2">
            <span className="h-1.5 w-1.5 rounded-full bg-[#79a84d]" />
            SYSTEM OPERATIONAL
          </div>
        </div>
      </div>

      {/* Main header */}
      <header className="border-b border-[#d9ddd5] bg-[#f8f9f6]">
        <div className="mx-auto flex h-16 max-w-375 items-center justify-between px-4 md:px-6">
          <a
            href="/"
            className="flex items-center gap-2 text-lg font-bold tracking-[-0.04em]"
          >
            <span className="flex h-7 w-7 items-center justify-center border border-[#171a17] bg-[#b7ef68]">
              <TerminalSquare size={15} strokeWidth={2.2} />
            </span>

            <span>
              page<span className="text-[#638e32]">goat</span>
            </span>
          </a>

          <nav className="hidden items-center gap-6 text-[11px] font-semibold uppercase tracking-[0.12em] text-[#626b62] md:flex">
            <a href="/" className="text-[#171a17]">
              Engine
            </a>

            <a href="/about" className="hover:text-[#171a17]">
              About
            </a>

            <a href="/contact" className="hover:text-[#171a17]">
              Contact
            </a>

            <a href="/terms" className="hover:text-[#171a17]">
              Terms
            </a>
          </nav>

          {/* Mobile sidebar trigger */}
          <button
            type="button"
            onClick={() => setMobileHistory(true)}
            className="flex items-center gap-2 border border-[#cfd4cc] bg-white px-3 py-2 text-xs font-semibold hover:bg-[#f4f5f1] md:hidden"
            aria-label="Open search history"
            aria-expanded={mobileHistory}
          >
            <Menu size={14} />
            History
          </button>
        </div>
      </header>

      <div className="mx-auto grid max-w-375 md:grid-cols-[230px_minmax(0,1fr)]">
        {/* Desktop history sidebar */}
        <aside className="hidden border-r border-[#d9ddd5] bg-[#eef0eb] md:block">
          <div className="sticky top-0 p-5">
            <div className="mb-5 flex items-center justify-between">
              <div>
                <p className="text-[10px] font-bold uppercase tracking-[0.18em] text-[#687168]">
                  Search history
                </p>

                <p className="mt-1 text-xs text-[#8a9189]">
                  Stored locally
                </p>
              </div>

              {history.length > 0 && (
                <button
                  type="button"
                  onClick={clearHistory}
                  className="text-[#8a9189] hover:text-[#171a17]"
                  title="Clear history"
                  aria-label="Clear search history"
                >
                  <X size={14} />
                </button>
              )}
            </div>

            {history.length === 0 ? (
              <div className="border border-dashed border-[#cdd2ca] bg-[#f7f8f5] p-4">
                <Clock3
                  size={16}
                  className="mb-3 text-[#8a9189]"
                />

                <p className="text-xs font-semibold text-[#5c655c]">
                  No searches yet.
                </p>

                <p className="mt-1 text-[11px] leading-5 text-[#8a9189]">
                  Websites you inspect will appear here.
                </p>
              </div>
            ) : (
              <div className="space-y-1">
                {history.map((item) => (
                  <button
                    key={`${item.url}-${item.timestamp}`}
                    type="button"
                    onClick={() => runHistorySearch(item.url)}
                    className="group flex w-full items-start gap-2 border border-transparent px-2 py-2.5 text-left hover:border-[#d0d5cc] hover:bg-[#f8f9f6]"
                  >
                    <Globe2
                      size={13}
                      className="mt-0.5 shrink-0 text-[#899189]"
                    />

                    <span className="min-w-0 flex-1">
                      <span className="block truncate text-xs font-medium text-[#3d453d]">
                        {item.url}
                      </span>

                      <span className="mt-0.5 block text-[10px] text-[#929991]">
                        {formatTime(item.timestamp)}
                      </span>
                    </span>

                    <ChevronRight
                      size={12}
                      className="mt-0.5 opacity-0 transition-opacity group-hover:opacity-100"
                    />
                  </button>
                ))}
              </div>
            )}

            <div className="mt-8 border-t border-[#d3d8d0] pt-5">
              <p className="mb-3 text-[9px] font-bold uppercase tracking-[0.16em] text-[#899189]">
                Engine status
              </p>

              <div className="space-y-2 text-[10px] text-[#707970]">
                <div className="flex items-center justify-between">
                  <span>Extractor</span>
                  <span className="text-[#638e32]">READY</span>
                </div>

                <div className="flex items-center justify-between">
                  <span>Network</span>
                  <span className="text-[#638e32]">ONLINE</span>
                </div>

                <div className="flex items-center justify-between">
                  <span>Parser</span>
                  <span className="text-[#638e32]">READY</span>
                </div>
              </div>
            </div>
          </div>
        </aside>

        {/* Main engine */}
        <section className="min-w-0">
          <div className="mx-auto max-w-275 px-5 pb-20 pt-10 md:px-10 md:pt-14">
            {/* Engine intro */}
            <div className="grid gap-10 lg:grid-cols-[1fr_300px]">
              <div>
                <div className="mb-5 flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.2em] text-[#6d766d]">
                  <span className="h-1.5 w-1.5 bg-[#79a84d]" />
                  Web intelligence / 01
                </div>

                <h1 className="max-w-3xl text-4xl font-semibold leading-[1.02] tracking-[-0.055em] md:text-6xl">
                  Discover real info
                  <br />
                  <span className="text-[#71876a]">
                    behind the website.
                  </span>
                </h1>

                <p className="mt-5 max-w-2xl text-sm leading-6 text-[#697169]">
                  Enter a website. PageGoat reads the publicly available
                  document and builds a structured company intelligence
                  record from what it finds.
                </p>
              </div>

              <div className="border-l border-[#d9ddd5] pl-6">
                <div className="mb-4 flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.18em] text-[#687168]">
                  <Activity size={13} />
                  Engine notes
                </div>

                <p className="text-xs leading-5 text-[#737b73]">
                  Designed as a direct extraction engine. No conversational
                  interface is required to interrogate the source.
                </p>

                <div className="mt-5 space-y-2 text-[10px] text-[#858d85]">
                  <div className="flex gap-2">
                    <Check size={12} className="text-[#638e32]" />
                    Public web data
                  </div>

                  <div className="flex gap-2">
                    <Check size={12} className="text-[#638e32]" />
                    Structured extraction
                  </div>

                  <div className="flex gap-2">
                    <Check size={12} className="text-[#638e32]" />
                    Deterministic processing
                  </div>
                </div>
              </div>
            </div>

            {/* Search engine */}
            <div id="search-form" className="mt-12">
              <form onSubmit={handleSearch}>
                <div className="border border-[#aeb5ac] bg-white p-1 shadow-[0_1px_2px_rgba(0,0,0,0.04)] focus-within:border-[#6f8e4d]">
                  <div className="flex flex-col sm:flex-row">
                    <div className="flex min-w-0 flex-1 items-center">
                      <Search
                        size={18}
                        className="ml-4 shrink-0 text-[#7d857c]"
                      />

                      <input
                        type="text"
                        value={url}
                        onChange={(event) => setUrl(event.target.value)}
                        placeholder="Enter a website - apple.com"
                        required
                        autoComplete="url"
                        className="min-w-0 flex-1 bg-transparent px-3 py-4 text-sm outline-none placeholder:text-[#9aa199]"
                      />
                    </div>

                    <button
                      type="submit"
                      disabled={loading}
                      className="flex items-center justify-center gap-2 bg-[#171a17] px-7 py-4 text-xs font-bold uppercase tracking-[0.12em] text-white hover:bg-[#292e29] disabled:cursor-wait disabled:opacity-70"
                    >
                      {loading ? (
                        <>
                          <RefreshCw
                            size={14}
                            className="animate-spin"
                          />
                          Working
                        </>
                      ) : (
                        <>
                          Search
                          <ArrowUpRight size={14} />
                        </>
                      )}
                    </button>
                  </div>
                </div>
              </form>

              <div className="mt-3 flex items-center justify-between text-[10px] text-[#8a9189]">
                <span>
                  Try a domain, URL or company website.
                </span>

                <span className="hidden sm:block">
                  Publicly accessible sources only
                </span>
              </div>
            </div>

            {/* Engine activity */}
            {loading && (
              <section className="mt-8 border border-[#cdd3ca] bg-[#ecefe9]">
                <div className="flex items-center justify-between border-b border-[#d3d8d0] px-4 py-3">
                  <div className="flex items-center gap-2">
                    <Server size={14} />

                    <span className="text-[10px] font-bold uppercase tracking-[0.16em]">
                      Engine activity
                    </span>
                  </div>

                  <span className="font-mono text-[10px] text-[#72806e]">
                    PROCESSING
                  </span>
                </div>

                <div className="p-4">
                  <div className="space-y-1">
                    {ENGINE_STEPS.map((step, index) => {
                      const Icon = step.icon;
                      const completed = completedSteps.includes(index);
                      const active = activeStep === index;

                      return (
                        <div
                          key={step.id}
                          className={`flex items-center gap-3 border px-3 py-3 ${
                            active
                              ? "border-[#b7c6a9] bg-[#f6f8f3]"
                              : "border-transparent"
                          }`}
                        >
                          <div className="flex h-7 w-7 shrink-0 items-center justify-center border border-[#cbd1c7] bg-white">
                            {completed ? (
                              <Check
                                size={13}
                                className="text-[#638e32]"
                              />
                            ) : active ? (
                              <Icon
                                size={13}
                                className="text-[#638e32]"
                              />
                            ) : (
                              <span className="font-mono text-[9px] text-[#9ca39a]">
                                0{index + 1}
                              </span>
                            )}
                          </div>

                          <div className="min-w-0 flex-1">
                            <p className="text-xs font-semibold">
                              {step.label}
                            </p>

                            <p className="text-[10px] text-[#858d85]">
                              {step.detail}
                            </p>
                          </div>

                          <span
                            className={`text-[9px] font-bold uppercase tracking-[0.12em] ${
                              completed || active
                                ? "text-[#638e32]"
                                : "text-[#a0a69f]"
                            }`}
                          >
                            {completed
                              ? "Done"
                              : active
                                ? "Working"
                                : "Queued"}
                          </span>
                        </div>
                      );
                    })}
                  </div>

                  <div className="mt-4 flex items-center gap-2 border-t border-[#d5dad2] pt-3 font-mono text-[9px] text-[#7f887e]">
                    <span className="text-[#638e32]">&gt;</span>
                    Inspecting {cleanUrl(url) || "target"}...
                  </div>
                </div>
              </section>
            )}

            {/* Status */}
            <div
              aria-live="polite"
              className="min-h-12 py-5 text-xs text-[#778078]"
            >
              {error && (
                <div className="flex items-center gap-2 text-red-600">
                  <Info size={14} />
                  {error}
                </div>
              )}

              {!loading && !error && !data && (
                <div className="flex items-center gap-2">
                  <span className="h-1.5 w-1.5 bg-[#8da487]" />
                  Engine ready. Awaiting target.
                </div>
              )}

              {data && !loading && (
                <div className="flex items-center gap-2 text-[#638e32]">
                  <Check size={14} />
                  Extraction complete. Record assembled.
                </div>
              )}
            </div>

            {/* Results */}
            {data?.company && <Results data={data} />}

            {/* Engine disclosure */}
            <section className="mt-20 border-t border-[#d9ddd5] pt-6">
              <div className="grid gap-6 md:grid-cols-[1fr_auto]">
                <div>
                  <div className="mb-3 flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.18em] text-[#687168]">
                    <ShieldCheck size={13} />
                    Engine disclosure
                  </div>

                  <p className="max-w-3xl text-[11px] leading-5 text-[#7b837b]">
                    PageGoat does not use AI to generate or hallucinate company
                    information. The extraction engine is a proprietary
                    software system developed in-house to retrieve, parse,
                    normalize and present publicly accessible website signals.
                    Results depend on what the target website makes available.
                  </p>
                </div>

                <div className="flex items-start gap-2 text-[10px] text-[#8a9189]">
                  <Code2 size={13} className="mt-0.5" />

                  <span>
                    Proprietary
                    <br />
                    extraction engine
                  </span>
                </div>
              </div>

              <div className="mt-5 flex flex-col justify-between gap-3 border-t border-[#e0e3de] pt-4 text-[9px] uppercase tracking-[0.12em] text-[#969d95] sm:flex-row">
                <span>
                  Data sourced from publicly accessible web documents.
                </span>

                <span>No AI-generated company facts.</span>
              </div>
            </section>
          </div>
        </section>
      </div>

      {/* Mobile history sidebar / drawer */}
      {mobileHistory && (
        <div className="fixed inset-0 z-50 md:hidden">
          {/* Backdrop closes sidebar */}
          <button
            type="button"
            aria-label="Close history"
            onClick={() => setMobileHistory(false)}
            className="absolute inset-0 bg-[#171a17]/20"
          />

          <aside
            className="absolute right-0 top-0 flex h-full w-[320px] max-w-[90vw] flex-col border-l border-[#d9ddd5] bg-[#f4f5f1] shadow-xl"
            aria-label="Search history sidebar"
          >
            <div className="flex items-center justify-between border-b border-[#d9ddd5] bg-[#eef0eb] p-5">
              <div>
                <p className="text-[10px] font-bold uppercase tracking-[0.18em]">
                  Search history
                </p>

                <p className="mt-1 text-xs text-[#899189]">
                  Stored locally
                </p>
              </div>

              {/* Explicit sidebar close button */}
              <button
                type="button"
                onClick={() => setMobileHistory(false)}
                className="border border-[#d1d6ce] bg-white p-2 hover:bg-[#f4f5f1]"
                aria-label="Close search history"
                title="Close sidebar"
              >
                <X size={15} />
              </button>
            </div>

            <div className="flex-1 overflow-y-auto p-5">
              {history.length === 0 ? (
                <div className="border border-dashed border-[#cdd2ca] bg-[#f7f8f5] p-4">
                  <Clock3
                    size={16}
                    className="mb-3 text-[#8a9189]"
                  />

                  <p className="text-xs font-semibold text-[#5c655c]">
                    No searches yet.
                  </p>

                  <p className="mt-1 text-[11px] leading-5 text-[#8a9189]">
                    Websites you inspect will appear here.
                  </p>
                </div>
              ) : (
                <div className="space-y-1">
                  {history.map((item) => (
                    <button
                      key={`${item.url}-${item.timestamp}`}
                      type="button"
                      onClick={() => runHistorySearch(item.url)}
                      className="group flex w-full items-center gap-3 border border-transparent p-3 text-left hover:border-[#d1d6ce] hover:bg-white"
                    >
                      <Globe2
                        size={14}
                        className="shrink-0 text-[#7e887c]"
                      />

                      <span className="min-w-0 flex-1">
                        <span className="block truncate text-xs font-medium">
                          {item.url}
                        </span>

                        <span className="text-[10px] text-[#909890]">
                          {formatTime(item.timestamp)}
                        </span>
                      </span>

                      <ChevronRight
                        size={13}
                        className="shrink-0 text-[#a0a69f] opacity-0 group-hover:opacity-100"
                      />
                    </button>
                  ))}
                </div>
              )}

              {history.length > 0 && (
                <button
                  type="button"
                  onClick={clearHistory}
                  className="mt-5 flex items-center gap-2 text-[10px] font-bold uppercase tracking-[0.12em] text-[#7a8279] hover:text-[#171a17]"
                >
                  <X size={12} />
                  Clear history
                </button>
              )}
            </div>

            <div className="border-t border-[#d9ddd5] p-5">
              <div className="mb-3 flex items-center gap-2 text-[9px] font-bold uppercase tracking-[0.16em] text-[#899189]">
                <Activity size={12} />
                Engine status
              </div>

              <div className="space-y-2 text-[10px] text-[#707970]">
                <div className="flex items-center justify-between">
                  <span>Extractor</span>
                  <span className="text-[#638e32]">READY</span>
                </div>

                <div className="flex items-center justify-between">
                  <span>Network</span>
                  <span className="text-[#638e32]">ONLINE</span>
                </div>

                <div className="flex items-center justify-between">
                  <span>Parser</span>
                  <span className="text-[#638e32]">READY</span>
                </div>
              </div>
            </div>
          </aside>
        </div>
      )}

      <footer className="border-t border-[#d9ddd5] bg-[#eef0eb]">
        <div className="mx-auto flex max-w-375 flex-col gap-3 px-5 py-5 text-[9px] uppercase tracking-[0.12em] text-[#858d85] md:flex-row md:items-center md:justify-between md:px-6">
          <span>PageGoat · Company Intelligence Engine</span>

          <div className="flex gap-5">
            <a href="/about" className="hover:text-[#171a17]">
              About
            </a>

            <a href="/contact" className="hover:text-[#171a17]">
              Contact
            </a>

            <a href="/terms" className="hover:text-[#171a17]">
              Terms
            </a>
          </div>
        </div>
      </footer>
    </main>
  );
}

function Results({ data }: { data: CompanyData }) {
  const company = data.company;

  const socialEntries = Object.entries(company?.social || {});
  const brandingEntries = Object.entries(company?.branding || {});
  const seoEntries = Object.entries(company?.seo || {});
  const organizationEntries = Object.entries(
    company?.organization || {}
  );

  const resultCount = useMemo(() => {
    let count = 0;

    if (data.website) count++;
    if (company?.name) count++;
    if (company?.description) count++;
    if (company?.title) count++;
    if (company?.language) count++;
    if (company?.canonical_url) count++;

    count += company?.contact?.emails?.length || 0;
    count += company?.contact?.phones?.length || 0;

    count += socialEntries.length;
    count += brandingEntries.length;
    count += seoEntries.length;
    count += organizationEntries.length;

    return count;
  }, [
    data.website,
    company,
    socialEntries.length,
    brandingEntries.length,
    seoEntries.length,
    organizationEntries.length,
  ]);

  return (
    <section className="space-y-5">
      {/* Report heading */}
      <div className="border-y border-[#cfd5cc] bg-[#eef0eb]">
        <div className="flex flex-col justify-between gap-4 px-5 py-5 sm:flex-row sm:items-center">
          <div>
            <div className="mb-2 flex items-center gap-2 text-[9px] font-bold uppercase tracking-[0.18em] text-[#6b746b]">
              <Database size={12} />
              Intelligence record
            </div>

            <h2 className="max-w-3xl wrap-break-word text-2xl font-semibold tracking-[-0.035em]">
              {company?.name ||
                company?.title ||
                data.website ||
                "Company record"}
            </h2>

            {data.website && (
              <a
                href={externalUrl(data.website)}
                target="_blank"
                rel="noreferrer"
                className="mt-1 flex items-center gap-1 text-[11px] text-[#7d857d] hover:text-[#638e32]"
              >
                {data.website}
                <ExternalLink size={10} />
              </a>
            )}
          </div>

          <div className="grid grid-cols-2 gap-px border border-[#d0d5cd] bg-[#d0d5cd]">
            <div className="bg-[#f8f9f6] px-4 py-3">
              <p className="text-[9px] uppercase tracking-[0.12em] text-[#8a9189]">
                Signals
              </p>

              <p className="mt-1 font-mono text-sm font-bold">
                {resultCount}
              </p>
            </div>

            <div className="bg-[#f8f9f6] px-4 py-3">
              <p className="text-[9px] uppercase tracking-[0.12em] text-[#8a9189]">
                Status
              </p>

              <p className="mt-1 text-[10px] font-bold text-[#638e32]">
                COMPLETE
              </p>
            </div>
          </div>
        </div>
      </div>

      {/* Overview */}
      <section className="border border-[#d3d8d0] bg-white">
        <SectionHeader
          icon={<Building2 size={14} />}
          number="01"
          title="Company overview"
        />

        <div className="grid gap-0 lg:grid-cols-[1.4fr_1fr]">
          <div className="border-b border-[#e1e4df] p-5 lg:border-b-0 lg:border-r">
            <p className="text-sm leading-6 text-[#505950]">
              {company?.description || "No description found."}
            </p>
          </div>

          <div className="grid grid-cols-2">
            <DataCell
              label="Website"
              value={data.website}
              link={data.website}
            />

            <DataCell
              label="Language"
              value={company?.language}
            />

            <DataCell
              label="Page title"
              value={company?.title}
            />

            <DataCell
              label="Canonical"
              value={company?.canonical_url}
              link={company?.canonical_url}
            />
          </div>
        </div>
      </section>

      {/* Contact + social */}
      <div className="grid gap-5 lg:grid-cols-2">
        <section className="border border-[#d3d8d0] bg-white">
          <SectionHeader
            icon={<Phone size={14} />}
            number="02"
            title="Contact details"
          />

          <div className="p-5">
            <Field label="Email addresses">
              {company?.contact?.emails?.length ? (
                company.contact.emails.map((email) => (
                  <a
                    key={email}
                    href={`mailto:${email}`}
                    className="group flex items-center gap-2 py-1.5 text-sm hover:text-[#638e32]"
                  >
                    <Mail size={13} />

                    <span className="wrap-break-word">
                      {email}
                    </span>

                    <ArrowUpRight
                      size={11}
                      className="opacity-0 group-hover:opacity-100"
                    />
                  </a>
                ))
              ) : null}
            </Field>

            <Field label="Phone numbers">
              {company?.contact?.phones?.length
                ? company.contact.phones.map((phone) => (
                    <a
                      key={phone}
                      href={`tel:${phone}`}
                      className="flex items-center gap-2 py-1.5 text-sm hover:text-[#638e32]"
                    >
                      <Phone size={13} />
                      {phone}
                    </a>
                  ))
                : null}
            </Field>
          </div>
        </section>

        <section className="border border-[#d3d8d0] bg-white">
          <SectionHeader
            icon={<Link2 size={14} />}
            number="03"
            title="Social profiles"
          />

          <div className="p-5">
            {socialEntries.length ? (
              socialEntries.map(([key, value]) => (
                <div
                  key={key}
                  className="flex gap-4 border-b border-[#eceeea] py-3 last:border-0"
                >
                  <span className="w-24 shrink-0 text-[9px] font-bold uppercase tracking-[0.12em] text-[#8a9189]">
                    {key}
                  </span>

                  <a
                    href={externalUrl(value)}
                    target="_blank"
                    rel="noreferrer"
                    className="flex min-w-0 items-center gap-1 text-xs text-[#4c564c] hover:text-[#638e32]"
                  >
                    <span className="wrap-break-word">
                      {value}
                    </span>

                    <ExternalLink
                      size={11}
                      className="shrink-0"
                    />
                  </a>
                </div>
              ))
            ) : (
              <EmptyField />
            )}
          </div>
        </section>
      </div>

      {/* Metadata */}
      <div className="grid gap-5 lg:grid-cols-3">
        <MetadataCard
          number="04"
          title="Branding"
          icon={<Sparkles size={14} />}
          entries={brandingEntries}
        />

        <MetadataCard
          number="05"
          title="SEO"
          icon={<Search size={14} />}
          entries={[
            ...(company?.keywords
              ? [["keywords", company.keywords] as [string, string]]
              : []),
            ...seoEntries,
          ]}
        />

        <MetadataCard
          number="06"
          title="Organization"
          icon={<Network size={14} />}
          entries={organizationEntries}
        />
      </div>

      {/* Result footer */}
      <div className="flex flex-col gap-3 border-t border-[#d9ddd5] pt-5 text-[10px] text-[#8b938a] sm:flex-row sm:items-center sm:justify-between">
        <span>End of intelligence record.</span>

        <span className="flex items-center gap-2">
          <ShieldCheck size={12} />
          Extracted from publicly accessible information
        </span>
      </div>
    </section>
  );
}

function SectionHeader({
  icon,
  number,
  title,
}: {
  icon: React.ReactNode;
  number: string;
  title: string;
}) {
  return (
    <div className="flex items-center gap-3 border-b border-[#dfe3dd] bg-[#f7f8f5] px-5 py-3">
      <span className="font-mono text-[9px] text-[#929991]">
        {number}
      </span>

      <span className="text-[#6f786e]">{icon}</span>

      <h3 className="text-[10px] font-bold uppercase tracking-[0.15em]">
        {title}
      </h3>
    </div>
  );
}

function DataCell({
  label,
  value,
  link,
}: {
  label: string;
  value?: string;
  link?: string;
}) {
  return (
    <div className="border-b border-[#e1e4df] p-4 last:border-b-0">
      <p className="mb-1 text-[9px] font-bold uppercase tracking-[0.12em] text-[#90978f]">
        {label}
      </p>

      <p className="wrap-break-word text-xs leading-5 text-[#444c44]">
        {value ? (
          link ? (
            <a
              href={externalUrl(link)}
              target="_blank"
              rel="noreferrer"
              className="inline-flex items-center gap-1 hover:text-[#638e32]"
            >
              {value}
              <ExternalLink size={10} />
            </a>
          ) : (
            value
          )
        ) : (
          <span className="text-[#a0a69f]">
            Not found
          </span>
        )}
      </p>
    </div>
  );
}

function Field({
  label,
  children,
}: {
  label: string;
  children?: React.ReactNode;
}) {
  return (
    <div className="border-b border-[#eceeea] py-3 last:border-0">
      <p className="mb-1 text-[9px] font-bold uppercase tracking-[0.12em] text-[#90978f]">
        {label}
      </p>

      {children || <EmptyField />}
    </div>
  );
}

function EmptyField() {
  return (
    <p className="text-xs text-[#a0a69f]">
      No information found.
    </p>
  );
}

function MetadataCard({
  number,
  title,
  icon,
  entries,
}: {
  number: string;
  title: string;
  icon: React.ReactNode;
  entries: [string, string][];
}) {
  return (
    <section className="border border-[#d3d8d0] bg-white">
      <SectionHeader
        icon={icon}
        number={number}
        title={title}
      />

      <div className="p-5">
        {entries.length ? (
          entries.map(([key, value]) => (
            <div
              key={`${key}-${value}`}
              className="border-b border-[#eceeea] py-3 last:border-0"
            >
              <p className="mb-1 text-[9px] font-bold uppercase tracking-[0.12em] text-[#90978f]">
                {key}
              </p>

              <p className="wrap-break-word text-xs leading-5 text-[#4d554d]">
                {value}
              </p>
            </div>
          ))
        ) : (
          <EmptyField />
        )}
      </div>
    </section>
  );
}