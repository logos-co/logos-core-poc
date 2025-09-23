/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  docs: [
    {
      type: 'category',
      label: 'Intro',
      collapsible: false,
      items: [
        'getting-started',
      ],
    },
    {
      type: 'category',
      label: 'SDKs',
      collapsible: false,
      items: [
        'sdks/javascript-sdk',
        'sdks/nim-sdk',
        'sdks/cpp-sdk',
      ],
    },
    {
      type: 'category',
      label: 'Logos App',
      collapsible: false,
      items: [
        'logos-app/building-apps',
      ],
    },
    {
      type: 'category',
      label: 'liblogos',
      collapsible: false,
      items: [
        'liblogos/interacting-with-the-library',
        'liblogos/how-to-create-a-module',
        'liblogos/architecture',
      ],
    },
    {
      type: 'category',
      label: 'Modules',
      collapsible: false,
      items: [
        'modules/overview',
        'modules/chat',
        'modules/package-manager',
        'modules/waku',
        'modules/wallet',
      ],
    },
    {
      type: 'category',
      label: 'Examples',
      collapsible: false,
      items: [
        'examples/electron-app',
        'examples/cpp-app',
      ],
    },
  ],
};

module.exports = sidebars;
