import React from 'react';
import Layout from '@theme/Layout';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';

const features = [
];

export default function Home() {
  const { siteConfig } = useDocusaurusContext();

  return (
    <Layout description={siteConfig.tagline}>
      <main className="landing">
        <section className="landing__hero">
          <div className="landing__hero-copy">
            <p className="landing__eyebrow">LOGOS SDK</p>
            <h1>Build truly decentralized apps.</h1>
            <p className="landing__lead">
              Logos gives you a modular ecosystem to build and run truly decentralized applications.
            </p>
            <div className="landing__cta">
              <Link className="button button--primary button--lg" to="/docs/getting-started">
                Start Building
              </Link>
              <Link className="button button--outline button--lg" to="https://github.com/logos-co">
                View on GitHub
              </Link>
            </div>
          </div>
          <div className="landing__hero-visual">
            <img src="/img/hero-illustration.svg" alt="Logos SDK illustration" />
          </div>
        </section>

        <section className="landing__feature-intro">
          <h2>Why Logos</h2>
          <p>
            From secure identity to storage, the Logos SDK ecosystem gives you the tools to build truly decentralized applications.
          </p>
        </section>

        <section className="landing__features">
          {features.map((feature) => (
            <article key={feature.title} className="landing__feature-card">
              <h3>{feature.title}</h3>
              <p>{feature.description}</p>
            </article>
          ))}
        </section>

        <section className="landing__cta-footer">
          <div className="landing__cta-panel">
            <h2>Bring Logos into your app</h2>
            <p>
              Explore the SDK reference and tutorials on how to integrate Logos into your application.
            </p>
            <div className="landing__cta">
              <Link className="button button--primary button--lg" to="/">
                Explore the SDK
              </Link>
            </div>
          </div>
        </section>
      </main>
    </Layout>
  );
}
