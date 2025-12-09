#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include "wallet_checker.h"
#include "wallet_generator.h"
#include "wallet_database.h"
#include "coingecko_api.h"

// Optional: download database from a URL if not found locally
void ensure_evm_database(const std::string& db_path, const std::string& db_url) {
    if (!wallets_data_base::evm_database_exists()) {
        std::cerr << "[Info] EVM database not found locally. Downloading from: " << db_url << std::endl;
        std::string command = "curl -L -o " + db_path + " " + db_url;
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "[Error] Failed to download EVM database." << std::endl;
            std::exit(1);
        }
    }
}

void start_search_evm_wallets(int mode) {
    wallet_checker checker;
    if (!checker.initialize(mode)) {
        std::cerr << "[Error] Failed to initialize EVM checker." << std::endl;
        return;
    }

    if (mode == 0) {
        if (coingecko_api::get_btc_udst_price() == 0.0f) {
            std::cerr << "[Warning] Failed to connect to Coingecko API." << std::endl;
            return;
        }
    } else {
        ensure_evm_database(EVM_WAL_DATABASE_NAME, "https://github.com/tony-dev1/walletgen/releases/download/database/evm_database.dat");
        std::cout << "Loading EVM wallet database..." << std::endl;
        wallets_data_base::load_evm_database();
    }

    unsigned long long checked = 0;
    int speed = 0, last_check = 0, mnemonics_per_sec = 0;

    std::cout << std::fixed << std::setprecision(6);

    while (true) {
        std::string mnemonic = mnemonic_generate(128);
        wallet wal = generate_evm_wallet(mnemonic);
        wallet_data data = checker.get_wallet_data(wal, WAL_TYPE_EVM);

        if (data.eth_amount > 0 || data.bnb_amount > 0 || data.matic_amount > 0) {
            float usd = data.eth_amount * coingecko_api::get_eth_usdt_price()
                      + data.bnb_amount * coingecko_api::get_bnb_usdt_price()
                      + data.matic_amount * coingecko_api::get_matic_usdt_price();

            std::cout << "🎯 FOUND EVM WALLET!" << std::endl;
            std::cout << "Mnemonic: " << wal.mnemonic << std::endl;
            std::cout << "Address: " << wal.address << std::endl;
            std::cout << "ETH: " << data.eth_amount
                      << ", BNB: " << data.bnb_amount
                      << ", MATIC: " << data.matic_amount << std::endl;
            std::cout << "Estimated USD Value: $" << usd << std::endl;
            std::cout << "--------------------------------------------------" << std::endl;
        }

        if (time(0) - last_check > 1) {
            speed = mnemonics_per_sec;
            mnemonics_per_sec = 0;
            last_check = time(0);
        }

        ++checked;
        ++mnemonics_per_sec;
    }
}

void start_search_btc_wallets() {
    wallet_checker checker;
    if (!checker.initialize(0)) {
        std::cerr << "[Error] Failed to initialize BTC checker." << std::endl;
        return;
    }

    if (coingecko_api::get_btc_udst_price() == 0.0f) {
        std::cerr << "[Warning] Failed to connect to Coingecko API." << std::endl;
        return;
    }

    unsigned long long checked = 0;
    int speed = 0, last_check = 0, mnemonics_per_sec = 0;

    while (true) {
        std::string mnemonic = mnemonic_generate(128);
        wallet wal = generate_btc_wallet(mnemonic);
        wallet_data data = checker.get_wallet_data(wal, WAL_TYPE_BTC);

        if (data.btc_amount > 0) {
            float usd = data.btc_amount * coingecko_api::get_btc_udst_price();

            std::cout << "🎯 FOUND BTC WALLET!" << std::endl;
            std::cout << "Mnemonic: " << wal.mnemonic << std::endl;
            std::cout << "Address: " << wal.address << std::endl;
            std::cout << "BTC: " << data.btc_amount << std::endl;
            std::cout << "Estimated USD Value: $" << usd << std::endl;
            std::cout << "--------------------------------------------------" << std::endl;
        }

        if (time(0) - last_check > 1) {
            speed = mnemonics_per_sec;
            mnemonics_per_sec = 0;
            last_check = time(0);
        }

        ++checked;
        ++mnemonics_per_sec;
    }
}

int main() {
    std::cout << "=== WalletGen Multithreaded Wallet Scanner ===" << std::endl;

    std::thread evm_thread(start_search_evm_wallets, 0); // 0 = online mode
    std::thread btc_thread(start_search_btc_wallets);

    evm_thread.join();
    btc_thread.join();

    return 0;
}
