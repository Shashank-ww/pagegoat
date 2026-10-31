import Link from "next/link";
import { ArrowLeft, Target } from "lucide-react";

export default function TermsPage() {
  return (
    <main className="site-shell info-shell">
      <header className="site-header">
        <Link className="wordmark" href="/" aria-label="Welookit homepage">
          <span className="brand-icon">
            <Target size={20} strokeWidth={2} />
          </span>
          <span>welookit</span>
        </Link>

        <Link className="info-back" href="/">
          <ArrowLeft size={15} /> Homepage
        </Link>
      </header>

      <article className="info-content">
        <p className="eyebrow">WELOOKIT / TERMS</p>

        <h1>Terms</h1>

        <p className="info-intro">
          Please read these terms before using the Welookit company discovery
          service.
        </p>

        <section className="info-section">
          <h2>1. Using Welookit</h2>
          <p>
            Welookit provides a company lookup interface that retrieves and
            presents information returned by its extraction service. By using
            the service, you agree to use it lawfully and not to interfere with
            its operation, attempt unauthorised access, or misuse the
            information it returns.
          </p>
        </section>

        <section className="info-section">
          <h2>2. Information accuracy</h2>
          <p>
            Information is provided for general discovery and convenience. We
            do not guarantee that it is accurate, complete, current, available,
            or suitable for a particular purpose. You are responsible for
            independently verifying information before making business,
            financial, legal or other consequential decisions.
          </p>
        </section>

        <section className="info-section">
          <h2>3. Public sources and third-party websites</h2>
          <p>
            Results may contain links to third-party websites or information
            made publicly available by other parties. Those websites and their
            content are outside Welookit’s control and may have their own terms
            and privacy policies. A result or link does not imply endorsement
            or affiliation.
          </p>
        </section>

        <section className="info-section">
          <h2>4. Acceptable use</h2>
          <p>
            You must not use Welookit to violate applicable law, harass or
            target individuals, bypass access controls, overload the service,
            or collect and use information in ways that infringe privacy,
            intellectual-property rights or other legal rights.
          </p>
        </section>

        <section className="info-section">
          <h2>5. Availability and changes</h2>
          <p>
            The service may change, be interrupted or become unavailable.
            Features and returned fields may vary between lookups. We may
            update these terms as the product evolves.
          </p>
        </section>

        <section className="info-section">
          <h2>6. Limitation</h2>
          <p>
            To the extent permitted by applicable law, Welookit is provided on
            an “as is” and “as available” basis, without warranties regarding
            results or uninterrupted operation. Nothing in these terms
            excludes a liability that cannot lawfully be excluded.
          </p>
        </section>

        <section className="info-section">
          <h2>7. Contact and privacy</h2>
          <p>
            For questions about these terms or information displayed by the
            service, use the <a href="/contact">Contact page</a>. A dedicated
            privacy notice and verified contact channel should be published
            before public launch.
          </p>
        </section>

        <section className="info-section">
          <h2>8. Technology and infrastructure</h2>
          <p>
            Welookit is developed and operated using a combination of
            proprietary software, open-source technologies and third-party
            infrastructure. These technologies support the discovery,
            extraction, processing, presentation and delivery of company
            information.
          </p>

          <p>
            The technology stack includes, but is not limited to:
          </p>

          <ul>
            <li>
              <strong>Core extraction engine:</strong> C17, CMake, GCC and
              libcurl.
            </li>
            <li>
              <strong>Web application:</strong> Next.js, React, TypeScript and
              Node.js.
            </li>
            <li>
              <strong>Cloud infrastructure:</strong> Google Cloud Compute
              Engine and Ubuntu Server.
            </li>
            <li>
              <strong>Network and security:</strong> Cloudflare, Cloudflare
              Tunnel and Nginx.
            </li>
            <li>
              <strong>Hosting and deployment:</strong> Vercel and GitHub
              Actions.
            </li>
            <li>
              <strong>Source management:</strong> Git and GitHub.
            </li>
            <li>
              <strong>Data management:</strong> SQLite within supported
              application and engine workflows.
            </li>
            <li>
              <strong>Domain and routing:</strong> DNS configuration, domain
              resolution and reverse-proxy infrastructure.
            </li>
          </ul>

          <p>
            Technologies, infrastructure providers and implementation details
            may change as Welookit develops. The inclusion of a technology or
            third-party provider does not imply ownership, endorsement or
            affiliation with that provider.
          </p>
        </section>

        <p className="info-last-updated">
          Last updated: 31 October 2026
        </p>
      </article>
    </main>
  );
}