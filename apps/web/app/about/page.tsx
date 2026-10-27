import Link from "next/link";
import { ArrowLeft, Building2 } from "lucide-react";

export default function AboutPage() {
  return (
    <main className="site-shell info-shell">
      <header className="site-header">
        <Link className="wordmark" href="/" aria-label="Welookit homepage">
          <span className="brand-icon"><Building2 size={19} strokeWidth={2.2} /></span><span>welookit</span>
        </Link>
        <Link className="info-back" href="/"><ArrowLeft size={15} /> Homepage</Link>
      </header>
      <article className="info-content">
        <p className="eyebrow">WELOOKIT / ABOUT</p>
        <h1>About</h1>
        <p className="info-intro">Welookit helps you look up a company and extract essential public information in one place.</p>
        
        <section className="info-section"><h2>What you may find</h2><ul><li>Company website and basic page information</li><li>Public contact details, when found</li><li>Links to social profiles and other official pages</li><li>Website metadata, branding signals and detected technologies</li></ul></section>
        <section className="info-section"><h2>How to use the information</h2><p>Welookit is a discovery crawler, not a definitive company registry or verification service. Information can be incomplete, outdated, incorrectly attributed or unavailable. Check important details against the company’s official website or another authoritative source before relying on them.</p></section>
        <section className="info-section"><h2>Our approach</h2><p>One search. One clear company profile. We aim to make publicly available company information easier to find without making the experience complicated.</p></section>
      </article>
    </main>
  );
}
