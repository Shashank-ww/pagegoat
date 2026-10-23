
"use client";

import { FormEvent, useState } from "react";

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

export default function Home() {
  const [url, setUrl] = useState("");
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");
  const [data, setData] = useState<CompanyData | null>(null);

  async function handleSearch(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();

    setLoading(true);
    setError("");
    setData(null);

    try {
      const response = await fetch("/api/extract", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ url: url.trim() }),
      });

      const result = await response.json();

      if (!response.ok) {
        throw new Error(result.error || "Extraction failed.");
      }

      setData(result);
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

  function Field({
    label,
    value,
  }: {
    label: string;
    value?: string | string[];
  }) {
    const values = Array.isArray(value) ? value : [value];
    const present = values.filter(
      (item): item is string => Boolean(item?.trim())
    );

    return (
      <div className="border-t border-[#303830] py-4">
        <p className="mb-2 text-xs uppercase tracking-wider text-[#98a397]">
          {label}
        </p>
        {present.length ? (
          present.map((item, index) => (
            <p key={`${item}-${index}`} className="mb-1 wrap-break-word text-sm">
              {item}
            </p>
          ))
        ) : (
          <p className="text-sm text-[#778176]">Not found</p>
        )}
      </div>
    );
  }

  const company = data?.company;

  return (
    <main className="min-h-screen bg-[#101310] text-[#edf2e9]">
      <header className="flex items-center justify-between border-b border-[#303830] px-6 py-5 md:px-12">
        <a href="/" className="text-xl font-bold tracking-tight">
          page<span className="text-[#b5f36b]">goat</span>
        </a>
        <span className="text-xs text-[#98a397]">
          COMPANY INTELLIGENCE
        </span>
      </header>

      <section className="mx-auto max-w-4xl px-5 pb-20 pt-20 md:pt-28">
        <div className="mb-10 text-center">
          <p className="mb-4 text-xs uppercase tracking-[0.25em] text-[#b5f36b]">
            Website intelligence
          </p>
          <h1 className="text-4xl font-bold leading-tight tracking-tight md:text-6xl">
            Know the company.
            <br />
            <span className="text-[#b5f36b]">
              Not just the website.
            </span>
          </h1>
          <p className="mx-auto mt-5 max-w-xl text-sm leading-6 text-[#98a397]">
            Extract company information, contact details, social profiles
            and website metadata from a single URL.
          </p>
        </div>

        <form
          onSubmit={handleSearch}
          className="flex flex-col gap-2 border border-[#475347] bg-[#191e19] p-2 sm:flex-row"
        >
          <input
            type="text"
            value={url}
            onChange={(event) => setUrl(event.target.value)}
            placeholder="Enter a website, e.g. apple.com"
            required
            className="min-w-0 flex-1 bg-transparent px-4 py-3 text-sm outline-none placeholder:text-[#778176]"
          />
          <button
            type="submit"
            disabled={loading}
            className="bg-[#b5f36b] px-7 py-3 text-sm font-bold text-[#17200f] hover:bg-[#c8ff8c] disabled:cursor-wait disabled:opacity-60"
          >
            {loading ? "Extracting..." : "Search →"}
          </button>
        </form>

        <div aria-live="polite" className="min-h-12 py-4 text-sm text-[#98a397]">
          {loading && "Fetching website and extracting available data..."}
          {error && <span className="text-red-400">{error}</span>}
          {!loading && !error && !data && "Enter a website to get started."}
          {data && "Extraction complete."}
        </div>

        {company && (
          <div className="space-y-5">
            <section className="border border-[#303830] bg-[#191e19] p-6">
              <p className="mb-4 text-xs font-semibold tracking-widest text-[#b5f36b]">
                COMPANY OVERVIEW
              </p>
              <h2 className="wrap-break-word text-2xl font-bold">
                {company.name || company.title || data.website || "Company"}
              </h2>
              <p className="mt-3 text-sm leading-6 text-[#98a397]">
                {company.description || "No description found."}
              </p>
              <div className="mt-5">
                <Field label="Website" value={data.website} />
                <Field label="Page title" value={company.title} />
                <Field label="Language" value={company.language} />
                <Field label="Canonical URL" value={company.canonical_url} />
              </div>
            </section>

            <div className="grid gap-5 md:grid-cols-2">
              <section className="border border-[#303830] bg-[#191e19] p-6">
                <h3 className="mb-4 font-semibold text-[#b5f36b]">
                  Contact details
                </h3>
                <Field label="Emails" value={company.contact?.emails} />
                <Field label="Phone numbers" value={company.contact?.phones} />
              </section>

              <section className="border border-[#303830] bg-[#191e19] p-6">
                <h3 className="mb-4 font-semibold text-[#b5f36b]">
                  Social profiles
                </h3>
                {Object.entries(company.social || {}).map(([key, value]) => (
                  <Field key={key} label={key} value={value} />
                ))}
                {!Object.values(company.social || {}).some(Boolean) && (
                  <Field label="Profiles" />
                )}
              </section>

              <section className="border border-[#303830] bg-[#191e19] p-6">
                <h3 className="mb-4 font-semibold text-[#b5f36b]">
                  Branding
                </h3>
                {Object.entries(company.branding || {}).map(([key, value]) => (
                  <Field key={key} label={key} value={value} />
                ))}
              </section>

              <section className="border border-[#303830] bg-[#191e19] p-6">
                <h3 className="mb-4 font-semibold text-[#b5f36b]">
                  SEO & organization
                </h3>
                <Field label="Keywords" value={company.keywords} />
                {Object.entries(company.seo || {}).map(([key, value]) => (
                  <Field key={key} label={key} value={value} />
                ))}
                {Object.entries(company.organization || {}).map(([key, value]) => (
                  <Field key={key} label={key} value={value} />
                ))}
              </section>
            </div>
          </div>
        )}
      </section>

      <footer className="border-t border-[#303830] px-5 py-6 text-center text-xs text-[#778176]">
        PageGoat · Company intelligence
      </footer>
    </main>
  );
}
