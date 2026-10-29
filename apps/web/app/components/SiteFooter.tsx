
export default function SiteFooter() {
  return (
    <footer className="site-footer">
      <a
        className="footer-wordmark"
        href="/"
        aria-label="Welookit homepage"
      >
        <span>© {new Date().getFullYear()} welookit.com</span>
      </a>
      <span>
        Engine built on <a href="https://www.c-language.org/">C</a>. Pure speed.
      </span>

      <nav className="site-footer-nav" aria-label="Footer navigation">
        <a href="/about">About</a>
        <a href="/engine">Engine</a>
        <a href="/terms">Terms</a>
        <a href="/contact">Contact</a>
      </nav>
    </footer>
  );
}
