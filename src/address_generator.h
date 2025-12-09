#pragma once
#include <string>
#include "wallet_generator.h"

// Convert raw bytes to a hex string
std::string bytes_to_hex_string(const uint8_t* data, size_t len);

// Generate an Ethereum wallet from a mnemonic phrase
wallet generate_evm_wallet(std::string mnemonic);

// Generate a Bitcoin wallet (Bech32, Legacy, P2SH) from a mnemonic phrase
wallet generate_bitcoin_wallet(const std::string mnemonic);
