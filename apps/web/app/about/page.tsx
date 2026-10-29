
import Link from "next/link";
import { ArrowLeft, Building2, Cpu, Search, Workflow } from "lucide-react";

export default function AboutPage() {
  return (
    <main className="site-shell info-shell">
      <header className="site-header">
        <Link className="wordmark" href="/" aria-label="Welookit homepage">
          <span className="brand-icon">
            <Building2 size={19} strokeWidth={2.2} />
          </span>
          <span>welookit</span>
        </Link>

        <Link className="info-back" href="/">
          <ArrowLeft size={15} /> Homepage
        </Link>
      </header>

      <article className="info-content">
        <p className="eyebrow">WELOOKIT / ABOUT</p>

        <h1>The web has the answers. <br/> Finding it, is the work.</h1>

        <p className="info-intro">
          Welookit started as a hobby project with a simple idea:
          looking up a company should not mean opening 30 multiple tabs and piecing together information that is already out there.
        </p>

        <section className="info-section">
          <h2>A small idea, taken seriously</h2>
          <p>
            It began with a most common problem. You come across a company
            and want to know more about it. Its website, contact details,
            social profiles, the technology it uses, and some
            other useful details that help you understand what it does or is.
          </p>
          <p>
            The information is available, often hidden, and finding it takes time.
            Moving between websites, opening page after page, and putting the
            information together yourself.
          </p>
          <p>
            I started wondering whether that repetitive work could be
            handled by a small, purpose-built programme instead. Something
            straightforward to use on the outside, with the engineering
            doing the hard work underneath. Yes, you can do it with AI. Good luck with that!
          </p>
          <p>
            That experiment became the core thought behind {" "}
            <a href="/" aria-label="Welookit homepage">
            <span>welookit.com</span>
            </a>
          </p>
          <p>
            I was using this in my own research for my internal projects, 
            when my colleagues started asking for it's link.
          </p>
        </section>

        <section className="info-section">
          <h2>Start with the source</h2>
          <p>
            The internet is no dearth of information. What is often
            missing is a simple way to get to the relevant source without
            doing all the manual work yourself.
          </p>
          <p>
            Welookit takes a company name or website and works towards
            a useful, structured view of the information its accessible
            web pages reveal. Including page details, public
            contact information, social links, website metadata and
            detected technologies. Basically, all publicly available information, hidden in plain sight.
          </p>
          <p>
            It is not a directory or company registry, and it does not claim to know
            everything about a business. It is a way to start exploring
            what is actually available on the web for its audience.
          </p>
        </section>

        <section className="info-section">
          <h2>Good tools do not need to be complicated</h2>
          <p>
            We made the experience deliberately simple. Enter a company,
            inspect the results, and decide what is useful to you.
            There is no need to learn a new AI workflow just to find
            basic information.
          </p>
          <p>
            Behind that simplicity is a <b>native engine written in C. </b>
            It is a selective choice to build the underlying mechanism for the task
            rather than making an AI model responsible for every step.
          </p>
          <p>
            While AI has a place in the future of information discovery.
            It can help explain, connect and interpret information.
            But there is also value in retrieving the original source
            and extracting its contents directly. We want to make
            that foundation useful in its own right. Unlike fabricating results with AI and charge hefty tokens.
          </p>
        </section>

        <section className="info-section">
          <h2>Built to keep getting better</h2>
          <p>
            Welookit is an independent project, developed one part
            at a time. The ambition is to make company discovery
            faster and more accessible, while continuing to improve
            the engine that powers it.
          </p>
          <p>
            We are interested in what happens when a simple user
            experience meets software built specifically for the
            job: fewer unnecessary steps, useful results, and a
            clearer view of the information hidden behind the websites.
          </p>
        </section>

        <section className="info-section">
          <h2>What we are building</h2>

          <ul>
            <li>
              <strong>Company discovery</strong> - a simpler way
              to look up businesses and their websites.
            </li>
            <li>
              <strong>Source-based extraction</strong> - information
              retrieved from accessible pages, not fabricated fields.
            </li>
            <li>
              <strong>A purpose-built engine</strong> - native C
              processing programme with dedicated extraction engine.
            </li>
            <li>
              <strong>Room to build</strong> - an underlying
              technology that can evolve beyond the first
              Welookit experience (for devs).
            </li>
          </ul>
        </section>

        <section className="info-section">
          <h2>Look beyond the search</h2>
          <p>
            The website is the starting point. The engine is where
            we intend to keep doing the work.
          </p>
          <p>
            <Link href="/engine">Take a closer look at the engine</Link>
            {" "}and how it works.
          </p>
        </section>

        <p className="info-last-updated">
          An independent project, built with curiosity and purpose.
        </p>
      </article>
    </main>
  );
}
