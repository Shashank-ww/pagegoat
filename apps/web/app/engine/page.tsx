
import Link from "next/link";
import {
  ArrowLeft,
  Building2,
  Cpu,
  Database,
  Globe2,
  Layers,
  Workflow,
} from "lucide-react";

export default function EnginePage() {
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
        <p className="eyebrow">WELOOKIT / ENGINE</p>

        <h1>Built to crawl the web into structured Markdowns.</h1>

        <p className="info-intro">
          Welookit is powered by a purpose-built engine written in C.
          It discovers and crawls the site to retrieve accessible pages, that it processes
          to extract the information those pages actually contain. 
        </p>  
        <p>
        The goal is simple: turn a web page information into an easy answer.
        </p>

        <section className="info-section">
          <h2>Why build an engine at all?</h2>
          <p>
            A company website is more than what you see on the screen.
            Beneath its visible content are page titles, descriptions,
            links, contact details, metadata, references to social
            profiles and signals about the technologies used to
            build it.
          </p>
          <p>
            Much of this information can be collected directly from
            the pages that publish it. But doing that repeatedly,
            across different websites and layouts, takes work. And more importantly, time. 
          </p>
          <p>
            Welookit was built to handle that work. Instead of
            treating every lookup as a fresh research exercise, the
            engine follows a defined process for finding a destination,
            retrieving its pages and extracting the information
            they expose.
          </p>
          <p>
            The result is a <i>structured view of a company</i> based on
            what the engine can actually retrieve, not a generated
            profile that merely sounds convincing. No fabricated information. No fancy AI layers. 
            Clean markdown files, that you can export as .JSON or .CSV needed.
          </p>
        </section>

        <section className="info-section">
          <h2>But why C?</h2>
          <p>
            The choice of C is deliberate. Welookit is not simply
            a collection of prompts we wrap around a website.
            Its foundation is a native programme designed to perform
            a defined sequence of crawl, network, 
            parsing and extraction tasks.
          </p>
          <p>
            C gives us direct control over memory, data structures,
            execution and the way components fit together. 
            Top companies can prefer any secured platform to create or host their website.
            But the underlining technology is going to remain in a low-level programming language. Which is the key for our setup.
            </p>
            <p>
            In terms of raw volume, web technology trackers report detected over 22.6 million live 
            websites actively running on PHP. And PHP is built on the C.
            It also allows us to build a compact programme without making
            a large application runtime the centre of the engine.
          </p>
          <p>
            That control matters to us when building software whose job
            is to repeat a relatively focused set of operations:
            to retrieve a page, process its contents, identify useful
            information and return a predictable result structure.
          </p>
          <p>
            It is also an engineering commitment. Rather than
            depending on a general-purpose model to perform every
            part of the task, we can define the extraction rules,
            inspect implementation and improve individual
            components as we learn more ourselves.
          </p>
          <p>
            C is not a magic performance switch. Actual speed and
            resource usage depend on the implementation, network
            conditions, page complexity and workload. Our aim is
            to use the language's control and efficiency where
            they are useful, and measure performance as the engine
            develops.
          </p>
        </section>

        <section className="info-section">
          <h2>Where AI fits in</h2>
          <p>
            AI and a native extraction engine solve different
            parts of the information problem.
          </p>
          <p>
            An AI model can interpret a question, summarise a
            company, connect ideas across sources and explain
            complicated information in plain language. These
            capabilities are valuable, and we are not building
            Welookit to compete with them.
          </p>
          <p>
            The engine has a different responsibility: retrieve
            accessible web content and extract the information
            it already carries.
          </p>
          <p>
            Consider a company website that publishes a telephone
            number, an email address and a set of social links.
            The engine can look for those details in the crawled
            page and return the values it finds. It does not need
            a language model to invent a likely phone number or
            write a description of the company before it can
            perform that task.
          </p>
          <p>
            This distinction matters. AI-generated answers can
            be useful, but they can also misinterpret a source
            or fill gaps with information that was never present.
            Rule-based extraction has its own limitations: pages
            vary, information may be missing, and not every
            website exposes its data in the same way.
          </p>
          <p>
            Our approach is to make retrieval and extraction
            work as reliably as we can, preserve the distinction
            between found and missing information, and leave
            room for AI to add value where interpretation is
            genuinely needed. Later.
          </p>
          <p>
            We see the two as complementary. A purpose-built
            engine can supply the evidence; an AI model can
            help people understand it.
          </p>
        </section>

        <section className="info-section">
          <h2>From a company name to structured results</h2>
          <p>
            A lookup passes through a series of defined stages.
            Each stage has a specific job, keeping discovery,
            retrieval and extraction separate.
          </p>

          <div className="info-section">
            <h2>
              <Globe2
                size={17}
                strokeWidth={2}
                style={{
                  display: "inline",
                  verticalAlign: "middle",
                  marginRight: "8px",
                }}
              />
              01. Input and discovery
            </h2>
            <p>
              The engine accepts a company name, website URL or
              supported search input. When a destination needs
              to be discovered, the crawl process works
              towards identifying a relevant website.
            </p>
          </div>

          <div className="info-section">
            <h2>
              <Workflow
                size={17}
                strokeWidth={2}
                style={{
                  display: "inline",
                  verticalAlign: "middle",
                  marginRight: "8px",
                }}
              />
              02. URL validation and retrieval
            </h2>
            <p>
              The URLs are then normalised and validated before
              entering the retrieval process. The download
              module requests accessible pages and passes
              their responses to the processing layer.
            </p>
          </div>

          <div className="info-section">
            <h2>
              <Layers
                size={17}
                strokeWidth={2}
                style={{
                  display: "inline",
                  verticalAlign: "middle",
                  marginRight: "8px",
                }}
              />
              03. HTML processing and HyperParse
            </h2>
            <p>
              Retrieved HTML is processed by dedicated extraction
              modules. Depending on the page and available
              information, these modules can identify company
              metadata, contact details, social profiles, SEO
              information, branding signals and website
              technologies.
            </p>
          </div>

          <div className="info-section">
            <h2>
              <Database
                size={17}
                strokeWidth={2}
                style={{
                  display: "inline",
                  verticalAlign: "middle",
                  marginRight: "8px",
                }}
              />
              04. Structured output
            </h2>
            <p>
              The extracted fields are assembled into a unified
              result structure. The engine is designed around
              outputs that can be inspected and processed by
              software, rather than only presented as prose.
              JSON and CSV are natural output formats for this
              kind of data.
            </p>
          </div>
        </section>

        <section className="info-section">
          <h2>What the engine can look for</h2>
          <ul>
            <li>
              <strong>Company and page information</strong> -
              page titles, descriptions and available metadata.
            </li>
            <li>
              <strong>Contact details</strong> - email addresses
              and telephone numbers found in accessible content.
            </li>
            <li>
              <strong>Social profiles</strong> - links to
              company profiles on supported social platforms.
            </li>
            <li>
              <strong>Technology signals</strong> - detectable
              frameworks, advertising cookies, analytics tools, content management
              systems and related website technologies.
            </li>
            <li>
              <strong>Branding and SEO signals</strong> -
              available icons, canonical URLs and other
              relevant page metadata.
            </li>
          </ul>
          <p>
            These are extraction targets, not guarantees. A field
            may be absent, hidden behind secured restrictions,
            loaded dynamically or unavailable in the pages
            retrieved during a lookup.
          </p>
        </section>

        <section className="info-section">
          <h2>A crawler is not the same as a search index</h2>
          <p>
            Traditional search engines maintain large indexes
            built from pages discovered and processed over time.
            A crawler can also retrieve a page when it is needed
            and process its contents directly.
          </p>
          <p>
            Welookit focuses on company discovery and page-level
            extraction. It can work with accessible pages during
            a lookup, but that does not mean it maintains a
            complete, continuously updated index of the internet.
          </p>
          <p>
            This distinction is important because a live retrieval
            can reflect what a website serves at the time of
            access, while still being subject to redirects,
            network failures, access restrictions and changes
            made by the website itself.
          </p>
        </section>

        <section className="info-section">
          <h2>Built to evolve</h2>
          <p>
            The current engine is the foundation, not the
            finished ambition. Keeping discovery, HTTP retrieval,
            HTML processing and extraction modules distinct
            gives us a clearer path to improve individual
            components without turning the entire system into
            one large, tightly coupled process.
          </p>
          <p>
            Over time, that could mean broader extraction
            coverage, better handling of different page types,
            more output options and ways for other software
            to build on the engine.
          </p>
          <p>
            We would rather improve the machinery one measurable
            step at a time than promise capabilities before
            they exist.
          </p>
        </section>

        <section className="info-section">
          <h2>The principle is simple</h2>
          <p>
            Retrieve the page. Extract what is there. Keep the
            result structured. Make it useful.
          </p>
          <p>
            AI will continue to change how people interact
            with information. We are interested in building
            the underlying tools that help make that
            information accessible in the first place.
          </p>
          <p>
            <Link href="/about">Read why we started Welookit</Link>
            {" "}or return to the <Link href="/">company discovery engine</Link>.
          </p>
        </section>

        <p className="info-last-updated">
          Welookit Engine - a work in progress.
        </p>
      </article>
    </main>
  );
}
