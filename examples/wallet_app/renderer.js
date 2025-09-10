document.addEventListener('DOMContentLoaded', () => {
  const statusEl = document.getElementById('status');
  const resultEl = document.getElementById('result');
  const rpcEl = document.getElementById('rpcUrl');
  const addrEl = document.getElementById('address');
  const btnInit = document.getElementById('btnInit');
  const btnWalletInit = document.getElementById('btnWalletInit');
  const btnChainId = document.getElementById('btnChainId');
  const btnBalance = document.getElementById('btnBalance');

  function setStatus(msg) { statusEl.textContent = msg; }
  function setResult(msg) { resultEl.textContent = msg; }

  btnInit.addEventListener('click', async () => {
    setStatus('Initializing...');
    const res = await window.walletAPI.initialize();
    if (!res.success) {
      setStatus(`Error: ${res.error}`);
      return;
    }
    setStatus('Initialized. Plugins: ' + JSON.stringify(res.pluginStatus.loaded));
  });

  btnWalletInit.addEventListener('click', async () => {
    const cfg = { rpcUrl: rpcEl.value || 'https://ethereum-rpc.publicnode.com' };
    const res = await window.walletAPI.initWallet(cfg);
    if (!res.success) { setResult(`Init error: ${res.error}`); return; }
    setResult(`Wallet init: ${JSON.stringify(res.message)}`);
  });

  btnChainId.addEventListener('click', async () => {
    const rpcUrl = rpcEl.value || 'https://ethereum-rpc.publicnode.com';
    const res = await window.walletAPI.chainId(rpcUrl);
    if (!res.success) { setResult(`ChainId error: ${res.error}`); return; }
    setResult(`Chain ID: ${res.message}`);
  });

  btnBalance.addEventListener('click', async () => {
    const rpcUrl = rpcEl.value || 'https://ethereum-rpc.publicnode.com';
    const address = addrEl.value.trim();
    if (!address) { setResult('Please enter an address'); return; }
    const res = await window.walletAPI.ethBalance(rpcUrl, address);
    if (!res.success) { setResult(`Balance error: ${res.error}`); return; }
    setResult(`ETH balance (wei): ${res.message}`);
  });
});


