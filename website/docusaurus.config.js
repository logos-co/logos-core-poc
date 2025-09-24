// @ts-check
const {themes: prismThemes} = require('prism-react-renderer');
const lightCodeTheme = prismThemes.github;
const darkCodeTheme = prismThemes.dracula;

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'Logos SDK',
  tagline: 'Compose secure, modular messaging apps with the Logos Core SDK.',
  url: 'https://logos.example.com',
  baseUrl: '/logos-core-poc/',
  deploymentBranch: 'gh-pages',
  organizationName: 'logos-co',
  projectName: 'logos-core-poc',
  favicon: 'img/favicon.ico',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  trailingSlash: false,
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },
  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          routeBasePath: 'docs',
          editUrl: 'https://github.com/logos-co/logos-core-poc/edit/main/website/',
        },
        blog: false,
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
  themeConfig: {
    image: 'img/social-card.png',
    algolia: {
      appId: 'YOUR_APP_ID',
      apiKey: 'YOUR_SEARCH_KEY',
      indexName: 'logos',
      contextualSearch: true,
    },
    navbar: {
      title: 'Logos',
      logo: {
        alt: 'Logos Core',
        src: 'img/logos.png',
        srcDark: 'img/logos.png',
      },
      items: [
        { to: '/docs/getting-started', label: 'Docs', position: 'left' },
        { to: '/docs/sdks/javascript-sdk', label: 'SDKs', position: 'left' },
        { to: '/docs/modules/overview', label: 'Modules', position: 'left' },
        {
          href: 'https://github.com/logos-co',
          label: 'GitHub',
          position: 'right',
        },
        {
          href: 'https://discord.gg/logosnetwork',
          label: 'Community',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Documentation',
          items: [
            { label: 'Getting Started', to: '/docs/getting-started' },
            { label: 'SDKs', to: '/docs/sdks/javascript-sdk' },
            { label: 'Modules', to: '/docs/modules/overview' },
            { label: 'Logos Library', to: '/docs/liblogos/interacting-with-the-library' },
          ],
        },
        {
          title: 'Community',
          items: [
            { label: 'Discord', href: 'https://discord.gg/logosnetwork' },
            { label: 'Discourse', href: 'https://forum.logos.co/' },
            { label: 'Twitter', href: 'https://twitter.com/@logos_network' },
            { label: 'YouTube', href: 'https://www.youtube.com/@LogosNetwork' },
          ],
        },
        {
          title: 'More',
          items: [
            { label: 'GitHub', href: 'https://github.com/logos-co' },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Logos.`,
    },
    colorMode: {
      defaultMode: 'dark',
      respectPrefersColorScheme: false,
      disableSwitch: false,
    },
    prism: {
      theme: lightCodeTheme,
      darkTheme: darkCodeTheme,
      additionalLanguages: ['rust', 'python', 'cpp', 'nim'],
    },
  },
};

module.exports = config;
