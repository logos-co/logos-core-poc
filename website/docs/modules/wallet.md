---
title: Wallet Module
description: Integrating wallet capabilities through Logos modules.
---
# Wallet Module

## Overview

The Wallet Module wraps a Go‑based wallet SDK (`libgowalletsdk`) to provide Ethereum‑compatible wallet capabilities: initializing a client, reading chain information and querying balances. It exposes a minimal API that will grow as functionality is integrated. This module is just a proof of concept at the moment and the API is expected to change dramatically.

## API

### Methods

- `initWallet(configJson: string)` → `bool`
  - Initializes the wallet client using the given configuration (currently uses a built‑in RPC URL for testing).
  - Returns `true` when the client is created.

- `chainId(rpcUrl: string)` → `string`
  - Returns the chain ID of the network at `rpcUrl`.

- `getEthBalance(rpcUrl: string, address: string)` → `string`
  - Returns the native ETH balance for `address` as a string (wei).

- `getErc20Balances(rpcUrl: string, address: string, tokenAddresses: string[])` → `string`
  - Returns ERC‑20 token balances for the provided token contracts.
  - Currently a stub; returns an empty string.

- `initLogos(logosAPI)`
  - Stores the `LogosAPI` pointer for internal calls and event wiring.

### Events

- No public events are emitted by this module at present.

### Notes

- The client handle is created on `initWallet` and freed on destruction.
- Methods auto‑initialize the client when needed in the implementation.

## Examples

### C++

Using generated `C++` wrappers from another module

```cpp
// Initialize wallet and query chain info
logos.wallet_module.initWallet("{}");
const auto id = logos.wallet_module.chainId("https://ethereum-rpc.publicnode.com");
qInfo() << "chainId:" << id;

// Query ETH balance
const auto bal = logos.wallet_module.getEthBalance(
  "https://ethereum-rpc.publicnode.com",
  "0x0000000000000000000000000000000000000000"
);
qInfo() << "balance (wei):" << bal;
```

### Javascript

Using the `JavaScript` SDK (reflective API)

```javascript
const LogosAPI = require('logos-api');

const logos = new LogosAPI();
logos.start();
logos.processAndLoadPlugins(["wallet_module"]);
logos.startEventProcessing();

(async () => {
  await logos.wallet_module.initWallet("{}");
  const id = await logos.wallet_module.chainId("https://ethereum-rpc.publicnode.com");
  console.log("chainId:", id);

  const bal = await logos.wallet_module.getEthBalance(
    "https://ethereum-rpc.publicnode.com",
    "0x0000000000000000000000000000000000000000"
  );
  console.log("balance (wei):", bal);
})();
```
