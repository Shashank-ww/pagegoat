"use client";

import { useEffect, useMemo, useRef, useState } from "react";
import type { ReactNode } from "react";
import {
  Activity,
  ArrowUpRight,
  Building2,
  Clock3,
  Home,
  Loader,
  Menu,
  RotateCw,
  Search,
  X,
} from "lucide-react";

type RecordValue = Record<string, unknown>;
type Tab = "Overview" | "Contacts" | "Website & social" | "Technology";
type EngineState = "idle" | "checking" | "online" | "error";

const TABS: Tab[] = ["Overview", "Contacts", "Website & social", "Technology"];
const EXAMPLES = ["Infosys", "Siemens", "Livpure"];
const RECENTS_KEY = "welookit-recent-searches";

function isRecord(value: unknown): value is RecordValue {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}

function text(value: unknown): string {
  return typeof value === "string" || typeof value === "number" ? String(value) : "";
}

function humanize(value: string): string {
  return value
    .replace(/([a-z0-9])([A-Z])/g, "$1 $2")
    .replace(/[_-]+/g, " ")
    .replace(/\b\w/g, (letter) => letter.toUpperCase());
}

function hasValue(value: unknown): boolean {
  if (value === null || value === undefined || value === "") return false;
  if (Array.isArray(value)) return value.length > 0;
  if (isRecord(value)) return Object.values(value).some(hasValue);
  return true;
}

function displayValue(value: unknown): string {
  if (!hasValue(value)) return "";
  if (typeof value === "boolean") return value ? "Yes" : "No";
  if (typeof value === "string" || typeof value === "number") return String(value);
  if (Array.isArray(value)) return value.map(displayValue).filter(Boolean).join(", ");
  if (isRecord(value)) {
    return Object.entries(value)
      .filter(([, child]) => hasValue(child))
      .map(([key, child]) => `${humanize(key)}: ${displayValue(child)}`)
      .join(" · ");
  }
  return "";
}

function normalizeUrl(value: string): string {
  const input = value.trim();
  if (!input) return "";
  if (/^(https?:\/\/|mailto:|tel:)/i.test(input)) return input;
  return `https://${input}`;
}

function hostname(value: string): string {
  try {
    return new URL(normalizeUrl(value)).hostname.replace(/^www\./i, "");
  } catch {
    return value;
  }
}

function linkUrl(value: unknown): string {
  const valueText = text(value).trim();
  if (/^(https?:\/\/|mailto:|tel:)/i.test(valueText)) return valueText;
  if (/^[\w.-]+\.[a-z]{2,}(\/.*)?$/i.test(valueText)) return `https://${valueText}`;
  return "";
}

function FieldRows({ data }: { data: RecordValue }) {
  const entries = Object.entries(data).filter(([, value]) => hasValue(value));
  if (!entries.length) return <p className="empty-note">No information available for this section.</p>;

  return (
    <dl className="field-list">
      {entries.map(([key, value]) => {
        const url = typeof value === "string" ? linkUrl(value) : "";
        return (
          <div className="field-row" key={key}>
            <dt>{humanize(key)}</dt>
            <dd>
              {url ? (
                <a href={url} target="_blank" rel="noreferrer">
                  {text(value)} <ArrowUpRight size={13} aria-hidden="true" />
                </a>
              ) : (
                displayValue(value)
              )}
            </dd>
          </div>
        );
      })}
    </dl>
  );
}

function Section({ title, children }: { title: string; children: ReactNode }) {
  return (
    <section className="data-section">
      <h2>{title}</h2>
      {children}
    </section>
  );
}

function LinkRows({ links }: { links: Array<{ label: string; value: unknown }> }) {
  const available = links.filter(({ value }) => hasValue(value));
  if (!available.length) return <p className="empty-note">No public links returned.</p>;

  return (
    <dl className="field-list">
      {available.map(({ label, value }) => {
        const url = linkUrl(value);
        return (
          <div className="field-row" key={label}>
            <dt>{label}</dt>
            <dd>
              {url ? (
                <a href={url} target="_blank" rel="noreferrer">
                  {text(value)} <ArrowUpRight size={13} aria-hidden="true" />
                </a>
              ) : displayValue(value)}
            </dd>
          </div>
        );
      })}
    </dl>
  );
}

function PeopleList({ people }: { people: unknown[] }) {
  const entries = people.filter(isRecord);
  if (!entries.length) return <p className="empty-note">No leadership details were returned.</p>;

  return (
    <div className="people-grid">
      {entries.map((person, index) => {
        const name = text(person.name) || text(person.full_name) || text(person.title) || `Person ${index + 1}`;
        const role = text(person.role) || text(person.position) || text(person.job_title);
        const details = Object.fromEntries(
          Object.entries(person).filter(
            ([key]) => !["name", "full_name", "title", "role", "position", "job_title"].includes(key),
          ),
        );
        return (
          <article className="person-card" key={`${name}-${index}`}>
            <h3>{name}</h3>
            {role && <p className="person-role">{role}</p>}
            <FieldRows data={details} />
          </article>
        );
      })}
    </div>
  );
}

export default function HomePage() {
  const [query, setQuery] = useState("");
  const [companyInput, setCompanyInput] = useState("");
  const [result, setResult] = useState<RecordValue | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");
  const [tab, setTab] = useState<Tab>("Overview");
  const [searched, setSearched] = useState(false);
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const [recentSearches, setRecentSearches] = useState<string[]>([]);
  const [engineStatus, setEngineStatus] = useState<EngineState>("idle");
  const [elapsedMs, setElapsedMs] = useState<number | null>(null);
  const [liveElapsedMs, setLiveElapsedMs] = useState(0);
  const [checkedAt, setCheckedAt] = useState("");
  const startedAt = useRef(0);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (!sidebarOpen) return;
    const previousHtmlOverflow = document.documentElement.style.overflow;
    const previousBodyOverflow = document.body.style.overflow;
    document.documentElement.style.overflow = "hidden";
    document.body.style.overflow = "hidden";
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.key === "Escape") setSidebarOpen(false);
    };
    window.addEventListener("keydown", onKeyDown);
    return () => {
      document.documentElement.style.overflow = previousHtmlOverflow;
      document.body.style.overflow = previousBodyOverflow;
      window.removeEventListener("keydown", onKeyDown);
    };
  }, [sidebarOpen]);

  useEffect(() => {
  if (!loading) return;

  const updateElapsed = () => {
    setLiveElapsedMs(performance.now() - startedAt.current);
  };

  updateElapsed();

  const interval = window.setInterval(updateElapsed, 100);

  return () => window.clearInterval(interval);
}, [loading]);

  useEffect(() => {
    try {
      const saved = localStorage.getItem(RECENTS_KEY);
      if (saved) {
        const parsed: unknown = JSON.parse(saved);
        if (Array.isArray(parsed)) setRecentSearches(parsed.filter((item): item is string => typeof item === "string").slice(0, 8));
      }
    } catch {
      // Search still works if browser storage is unavailable.
    }
  }, []);

  const company = useMemo(() => (isRecord(result?.company) ? result.company : {}), [result]);
  const organization = isRecord(company.organization) ? company.organization : {};
  const contact = isRecord(company.contact) ? company.contact : {};
  const social = isRecord(company.social) ? company.social : {};
  const branding = isRecord(company.branding) ? company.branding : {};
  const seo = isRecord(company.seo) ? company.seo : {};
  const people = Array.isArray(company.people) ? company.people : [];
  const technologies = Array.isArray(company.technology_stack)
    ? company.technology_stack
    : Array.isArray(company.technologies)
      ? company.technologies
      : [];

  const website = text(result?.website) || companyInput;
  const websiteHost = hostname(website);
  const companyName =
    text(company.name) ||
    text(company.title) ||
    text(company.legal_name) ||
    websiteHost ||
    companyInput;

  const socialLinks = Object.entries(social)
    .filter(([, value]) => hasValue(value))
    .map(([label, value]) => ({ label: humanize(label), value }));

  const overviewData: RecordValue = {
    Description: company.description,
    Keywords: company.keywords,
    Language: company.language,
    "Legal name": organization.legal_name,
    Founded: organization.founded,
  };

  const contactData: RecordValue = {
    Emails: contact.emails,
    Phones: contact.phones,
    Addresses: contact.addresses,
  };

  const websiteData: RecordValue = {
    Website: website,
    "Theme color": branding.theme_color,
    Generator: seo.generator ?? company.generator,
    Sitemap: seo.sitemap,
  };

  function goHome() {
    window.history.pushState({}, "", "/");
    setSearched(false);
    setResult(null);
    setError("");
    setLoading(false);
    setElapsedMs(null);
    setQuery("");
    setCompanyInput("");
    setTab("Overview");
    setSidebarOpen(false);
  }

  async function runSearch(rawQuery: string) {
    const value = rawQuery.trim();
    if (!value || loading) return;

    setQuery(value);
    setCompanyInput(value);
    setError("");
    setResult(null);
    setLoading(true);
    setSearched(true);
    setTab("Overview");
    setSidebarOpen(false);
    setElapsedMs(null);
    setEngineStatus("checking");
    startedAt.current = performance.now();

    // Keep the address bar clean: searches do not add a ?url=... parameter.
    window.history.pushState({}, "", "/");

    try {
      const response = await fetch("/api/extract", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ url: value }),
      });
      const payload: unknown = await response.json();

      if (!response.ok) {
        const message = isRecord(payload) ? text(payload.error) || text(payload.message) : "";
        throw new Error(message || `The request failed (${response.status}).`);
      }

      const data = isRecord(payload) ? payload : {};
      setResult(data);
      setEngineStatus("online");
      setCheckedAt(new Date().toLocaleString());

      setRecentSearches((current) => {
        const updated = [value, ...current.filter((item) => item.toLowerCase() !== value.toLowerCase())].slice(0, 8);
        try {
          localStorage.setItem(RECENTS_KEY, JSON.stringify(updated));
        } catch {
          // Recent history is optional.
        }
        return updated;
      });
    } catch (caught) {
      setEngineStatus("error");
      setError(caught instanceof Error ? caught.message : "We couldn't retrieve this company.");
    } finally {
      setElapsedMs(performance.now() - startedAt.current);
      setLoading(false);
    }
  }

  function renderTabContent() {
    if (!result) return null;

    if (tab === "Contacts") {
      return (
        <div className="content-grid">
          <div className="content-column">
            <Section title="Public contact details"><FieldRows data={contactData} /></Section>
            <Section title="Leadership"><PeopleList people={people} /></Section>
          </div>
          <div className="content-column">
            <Section title="Social profiles"><LinkRows links={socialLinks} /></Section>
            <Section title="Organisation details"><FieldRows data={organization} /></Section>
          </div>
        </div>
      );
    }

    if (tab === "Website & social") {
      return (
        <div className="content-grid">
          <div className="content-column">
            <Section title="Website details"><FieldRows data={websiteData} /></Section>
            <Section title="Official links">
              <LinkRows links={[{ label: "Website", value: website }, ...socialLinks]} />
            </Section>
          </div>
          <div className="content-column">
            <Section title="Branding">
              <div className="branding-grid">
                {[
                  ["Logo", branding.logo_url],
                  ["Favicon", branding.favicon_url],
                  ["Apple icon", branding.apple_icon_url || branding.apple_touch_icon_url],
                ].filter(([, value]) => hasValue(value)).map(([label, value]) => {
                  const url = linkUrl(value);
                  return (
                    <div className="branding-item" key={String(label)}>
                      <span>{String(label)}</span>
                      {url ? <a href={url} target="_blank" rel="noreferrer"><img src={url} alt={String(label)} /></a> : <span className="empty-note">Not returned</span>}
                    </div>
                  );
                })}
              </div>
              <FieldRows data={branding} />
            </Section>
          </div>
        </div>
      );
    }

    if (tab === "Technology") {
      return (
        <div className="content-grid">
          <div className="content-column">
            <Section title="Detected technologies">
              {technologies.length ? (
                <ul className="technology-list">{technologies.map((technology, index) => <li key={`${displayValue(technology)}-${index}`}>{displayValue(technology)}</li>)}</ul>
              ) : <p className="empty-note">No technology stack was returned.</p>}
            </Section>
            <Section title="Page metadata">
              <FieldRows data={{
                Title: company.title,
                Description: company.description,
                Keywords: company.keywords,
                Language: company.language,
                Generator: seo.generator ?? company.generator,
              }} />
            </Section>
          </div>
          <div className="content-column">
            <Section title="SEO signals"><FieldRows data={{
              Robots: seo.robots,
              Sitemap: seo.sitemap,
              "Open Graph": seo.open_graph,
              "Twitter card": seo.twitter_card,
            }} /></Section>
            <Section title="Other returned details">
              <FieldRows data={Object.fromEntries(Object.entries(company).filter(([key]) =>
                !["name", "title", "description", "keywords", "language", "contact", "social", "branding", "seo", "organization", "people", "technology_stack", "technologies"].includes(key),
              ))} />
            </Section>
          </div>
        </div>
      );
    }

    return (
      <div className="content-grid">
        <div className="content-column">
          <Section title="Company overview">
            <p className="company-description">{text(company.description) || "No company description was returned."}</p>
            <FieldRows data={overviewData} />
          </Section>
          <Section title="Official links"><LinkRows links={[{ label: "Website", value: website }, ...socialLinks]} /></Section>
          <Section title="Leadership"><PeopleList people={people} /></Section>
        </div>
        <div className="content-column">
          <Section title="Public contact details"><FieldRows data={contactData} /></Section>
          <Section title="Website signals"><FieldRows data={{
            Language: company.language,
            Generator: seo.generator ?? company.generator,
            "Technology stack": technologies,
            "Theme color": branding.theme_color,
          }} /></Section>
          <Section title="Branding"><FieldRows data={branding} /></Section>
        </div>
      </div>
    );
  }

const displayedElapsedMs = loading ? liveElapsedMs : elapsedMs;

const elapsedLabel =
  displayedElapsedMs === null
    ? ""
    : displayedElapsedMs < 1000
      ? `${Math.round(displayedElapsedMs)} ms`
      : `${(displayedElapsedMs / 1000).toFixed(2)} seconds`;

  return (
    <main className={`site-shell ${searched ? "results-shell" : "home-shell"}`}>
      <header className="site-header">
        <a className="wordmark" href="/" onClick={(event) => { event.preventDefault(); goHome(); }} aria-label="Welookit homepage">
          <span className="brand-icon"><Search size={19} strokeWidth={2.2} /></span>
          <span>welookit</span>
        </a>
        {searched && (
          <form className="header-search" onSubmit={(event) => { event.preventDefault(); void runSearch(query); }}>
            <input ref={inputRef} aria-label="Look up a company" value={query} onChange={(event) => setQuery(event.target.value)} placeholder="Company name or website" />
            <button type="submit" disabled={loading || !query.trim()} aria-label="Look it up">
              {loading ? <Loader className="spin" size={17} /> : <Search size={17} />}
            </button>
          </form>
        )}
        <button className="header-menu-button" type="button" onClick={() => setSidebarOpen(true)} aria-label="Open menu" aria-expanded={sidebarOpen}>
          <Menu size={21} />
        </button>
      </header>

      {!searched ? (
        <>
          <section className="hero">
            {/* <div className="hero-mark"><Search size={27} strokeWidth={2} /></div> */}
            <div className="hero-mark">
              <video
                src="/otter-commuting.webm"
                autoPlay
                loop
                muted
                playsInline
                aria-hidden="true"
              />
            </div>
            <h1>Find what&apos;s in-site, Fast.</h1>
            <p className="hero-copy">Inspect any company or website.</p>
            <form className="search-form" onSubmit={(event) => { event.preventDefault(); void runSearch(query); }}>
              <Search className="search-symbol" size={19} aria-hidden="true" />
              <input aria-label="Company name or website" value={query} onChange={(event) => setQuery(event.target.value)} placeholder="Enter company name or website" autoComplete="off" />
              <button type="submit" disabled={loading || !query.trim()}>
                {loading ? <><RotateCw size={16} className="spin" /> Looking</> : "Look it up"}
              </button>
            </form>
            <div className="example-searches">
              <span>Try</span>
              {EXAMPLES.map((example) => (
                <button key={example} type="button" onClick={() => { setQuery(example); void runSearch(example); }}>{example}</button>
              ))}
            </div>
            {error && <p className="home-error" role="alert">{error}</p>}
            <p className="privacy-note">Public information. One simple search.</p>
          </section>
          <section className="what-you-find">
            <h2>What you can find</h2>
            <div className="feature-grid">
              <article className="feature"><span className="feature-icon"><Building2 size={18} /></span><h3>Company essentials</h3><p>Official website, about the company and key public information in one place.</p></article>
              <article className="feature"><span className="feature-icon"><Activity size={18} /></span><h3>Public contacts</h3><p>Emails, phone numbers and locations when available.</p></article>
              <article className="feature"><span className="feature-icon"><ArrowUpRight size={18} /></span><h3>Online presence</h3><p>Social profiles, website details and detected technologies.</p></article>
            </div>
          </section>
        </>
      ) : (
        <div className="results-main">
          {/* <div className="results-toolbar">
            <button className="back-home" type="button" onClick={goHome}><Home size={15} /> Homepage</button>
            <span className="search-duration">
              <Clock3 size={14} />
              {loading
                ? `Looking - ${elapsedLabel || "0 ms"}`
                : elapsedLabel
                  ? `Looked up in ${elapsedLabel}`
                  : "Ready"}
            </span>
          </div> */}

          <section className="company-heading">
            <div className="company-title-row">
              <div className="company-avatar"><Building2 size={24} /></div>
              <div className="company-title-copy">
                <h1>{result ? companyName : companyInput}</h1>
                {website && <a className="company-domain" href={normalizeUrl(website)} target="_blank" rel="noreferrer">{websiteHost} <ArrowUpRight size={13} /></a>}
              </div>
              {result && website && <a className="visit-button" href={normalizeUrl(website)} target="_blank" rel="noreferrer">Visit website <ArrowUpRight size={14} /></a>}
            </div>
            <p className="checked-status">
              <span className={`status-dot ${loading ? "is-loading" : error ? "is-error" : result ? "is-online" : ""}`} />
              {loading ? "Looking for public information" : error ? "Lookup could not be completed" : result ? `Checked at ${checkedAt || "just now"}` : "Preparing lookup"}
            </p>
          </section>

          {loading && <div className="loading-panel" role="status"><span className="loading-line" /><p><RotateCw size={15} className="spin" /><span>
            Inspecting all available data…
            {loading && (
              <span className="search-duration"> {elapsedLabel || "0 ms"}</span>
            )}
          </span>
          </p></div>}

          {error && !loading && <div className="error-panel" role="alert"><h2>We couldn’t retrieve this company</h2><p>{error}</p><button className="outline-button" onClick={() => void runSearch(companyInput)}>Try again</button></div>}

          {result && !loading && (
            <>
              <nav className="tabs" aria-label="Company information categories">
                {TABS.map((item) => <button key={item} type="button" className={`tab-button ${tab === item ? "active" : ""}`} onClick={() => setTab(item)}>{item}</button>)}
              </nav>
              <section className="results-content">{renderTabContent()}</section>
              <div className="result-bottom-bar">
                <span><Clock3 size={14} /> Lookup completed in {elapsedLabel}</span>
                <button className="outline-button" onClick={() => {
                  const blob = new Blob([JSON.stringify(result, null, 2)], { type: "application/json" });
                  const href = URL.createObjectURL(blob);
                  const anchor = document.createElement("a");
                  anchor.href = href;
                  anchor.download = `${websiteHost || "company"}-welookit.json`;
                  anchor.click();
                  URL.revokeObjectURL(href);
                }}>Export data (.json)</button>
                <button className="home-result-button" onClick={goHome}><Home size={15} /> Homepage</button>
              </div>
            </>
          )}
        </div>
      )}

      {/* <footer className={`site-footer ${searched ? "results-footer" : ""}`}>
        <span>welookit - company intelligence discovery</span>
        <nav aria-label="Footer">
          <a href="/about">About</a>
          <a href="/contact">Contact</a>
          <a href="/terms">Terms</a>
        </nav>
      </footer> */}

      <button
        className={`drawer-backdrop ${sidebarOpen ? "is-open" : ""}`}
        aria-label="Close menu"
        aria-hidden={!sidebarOpen}
        tabIndex={sidebarOpen ? 0 : -1}
        onClick={() => setSidebarOpen(false)}
      />
      <aside
        className={`sidebar-drawer ${sidebarOpen ? "is-open" : ""}`}
        aria-label="Welookit menu"
        aria-hidden={!sidebarOpen}
        role="dialog"
        aria-modal={sidebarOpen}
      >
        <div className="drawer-header">
          <div><span className="drawer-kicker">WELOOKIT</span><h2>Quick menu</h2></div>
          <button className="drawer-close" type="button" onClick={() => setSidebarOpen(false)} aria-label="Close menu" tabIndex={sidebarOpen ? 0 : -1}><X size={19} /></button>
        </div>
        <section className="drawer-section">
          <h3>Navigation</h3>
          <div className="drawer-links">
            <button type="button" onClick={goHome} tabIndex={sidebarOpen ? 0 : -1}><Home size={15} /> Homepage</button>
            <a href="/about" tabIndex={sidebarOpen ? 0 : -1}>About</a><a href="/contact" tabIndex={sidebarOpen ? 0 : -1}>Contact</a><a href="/terms" tabIndex={sidebarOpen ? 0 : -1}>Terms</a>
          </div>
        </section>
        <section className="drawer-section">
          <h3>Recent lookups</h3>
          {recentSearches.length ? (
            <div className="drawer-recent-searches">{recentSearches.map((item) => <button key={item} type="button" tabIndex={sidebarOpen ? 0 : -1} onClick={() => { setQuery(item); void runSearch(item); }}>{item}<ArrowUpRight size={13} /></button>)}</div>
          ) : <p>Your recent lookups will appear here.</p>}
        </section>
        <section className="drawer-section">
          <h3>Engine status</h3>
          <div className="drawer-status"><span className={`drawer-status-dot ${engineStatus === "online" ? "is-online" : engineStatus === "error" ? "is-error" : engineStatus === "checking" ? "is-checking" : ""}`} />{engineStatus === "online" ? "Last lookup completed" : engineStatus === "error" ? "Last lookup failed" : engineStatus === "checking" ? "Lookup in progress" : "Not checked this session"}</div>
          <p>Response time: {elapsedLabel || "Not available yet"}</p>
          <p>Network uptime: Not currently measured</p>
          <p className="drawer-note">Status reflects this browser session. Live service uptime requires a dedicated health endpoint.</p>
        </section>
        <section className="drawer-section">
          <h3>Data & privacy</h3>
          <p>Lookups use public information returned by the Welookit extraction service. Recent searches are stored in this browser only.</p>
          <button className="clear-history" type="button" tabIndex={sidebarOpen ? 0 : -1} onClick={() => { setRecentSearches([]); try { localStorage.removeItem(RECENTS_KEY); } catch {} }}>Clear recent lookups</button>
        </section>
      </aside>
    </main>
  );
}
