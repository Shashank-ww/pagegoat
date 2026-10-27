import Link from "next/link";
import { ArrowLeft, Building2 } from "lucide-react";

export default function ContactPage() {
  const contactEmail = process.env.NEXT_PUBLIC_CONTACT_EMAIL;
  return (
    <main className="site-shell info-shell">
      <header className="site-header">
        <Link className="wordmark" href="/" aria-label="Welookit homepage">
          <span className="brand-icon"><Building2 size={19} strokeWidth={2.2} /></span><span>welookit</span>
        </Link>
        <Link className="info-back" href="/"><ArrowLeft size={15} /> Homepage</Link>
      </header>
      <article className="info-content">
        <p className="eyebrow">WELOOKIT / CONTACT</p>
        <h1>Contact</h1>
        <p className="info-intro">Questions, product feedback or a company-data correction? Here is how to prepare your request.</p>
        
        <section className="info-section"><h2>What can you contact us about?</h2><ul><li>Product feedback or a feature suggestion</li><li>Incorrect, outdated or misattributed company information</li><li>Privacy or data-removal requests relating to information displayed by Welookit</li><li>Business, partnership or general product enquiries</li></ul></section>
        <section className="info-section"><h2>Contact channel</h2><p>Welookit&apos;s public contact email is now live and actively monitored so visitors can reach the right person with feedback or support.</p><p className="info-callout"><strong>Reach out:</strong> shash.mbww@gmail.com</p></section>
        <section className="info-section"><h2>Help us resolve an issue</h2><p>When reporting a company-data issue, include the company name, the relevant page or field, and the correction or removal requested. Avoid sending passwords, financial details or other sensitive personal information.</p></section>
      </article>
    </main>
  );
}
