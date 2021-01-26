#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <memory.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <random>
#include <limits.h>
#include "crypto.h"
#include "exceptions.h"

namespace crypto {

    int first_prime_greater_than(int k)
    {
        if (k < 1)
            throw Exception("first_prime_greater_than -- k < 1");
        while (true)
        {
            k += 1;
            if (is_prime(k))
                return k;
        }
    }

    bool is_prime(const int n)
    {
        if (n > 1000000)
            throw Exception("argument too large");
        if (n < 2)
            return false;
        else if (n % 2 == 0 || n % 3 == 0)
            return false;
        int i = 5;
        while (i * i <= n)
        {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
            i += 6;
        }
        return true;
    }

    void sha512(
        const std::vector<uint8_t>& data,
        std::vector<uint8_t>& out
        )
    {
        out.resize(SHA512_DIGEST_LENGTH);
        SHA512_CTX ctx;
        if (SHA512_Init(&ctx) != 1)
            throw Exception("SHA512_Init");

        try
        {
            if (SHA512_Update(&ctx, data.data(), data.size()) != 1)
                throw Exception("SHA512_Update");
            if (SHA512_Final(out.data(), &ctx) != 1)
                throw Exception("SHA512_Final");
        }
        catch(const std::exception& e)
        {
            OPENSSL_cleanse(&ctx, sizeof(ctx));
            throw;
        }
        OPENSSL_cleanse(&ctx, sizeof(ctx));
    }

    void scrypt(
        const std::vector<uint8_t>& pass,
        const std::vector<uint8_t>& salt,
        std::vector<uint8_t>& out
        )
    {
        uint64_t const scrypt_N = 1024;
        uint64_t const scrypt_r = 8;
        uint64_t const scrypt_p = 16;
        out.resize(64);

        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_SCRYPT, NULL);
        if (pctx == 0)
            throw Exception("EVP_PKEY_CTX_new_id failed");
        try
        {
            if (EVP_PKEY_derive_init(pctx) <= 0)
                throw Exception("EVP_PKEY_derive_init");
            if (EVP_PKEY_CTX_set1_pbe_pass(pctx, (const char*)pass.data(), pass.size()) <= 0)
                throw("EVP_PKEY_CTX_set1_pbe_pass");
            if (EVP_PKEY_CTX_set1_scrypt_salt(pctx, (const unsigned char*)salt.data(), salt.size()) <= 0)
                throw("EVP_PKEY_CTX_set1_scrypt_salt");
            if (EVP_PKEY_CTX_set_scrypt_N(pctx, scrypt_N) <= 0)
                throw("EVP_PKEY_CTX_set_scrypt_N");
            if (EVP_PKEY_CTX_set_scrypt_r(pctx, scrypt_r) <= 0)
                throw("EVP_PKEY_CTX_set_scrypt_r");
            if (EVP_PKEY_CTX_set_scrypt_p(pctx, scrypt_p) <= 0)
                throw("EVP_PKEY_CTX_set_scrypt_p");
            size_t keylen = out.size();
            if (EVP_PKEY_derive(pctx, out.data(), &keylen) <= 0)
                throw("EVP_PKEY_derive");
        }
        catch(...)
        {
            EVP_PKEY_CTX_free(pctx);
            throw;
        }
        EVP_PKEY_CTX_free(pctx);;
    }

    void hmac(
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& data,
        std::vector<uint8_t>& result
        )
    {
        result.resize(64);
        unsigned int md_len = static_cast<unsigned int>(result.size());
        unsigned char *res =
            HMAC(
                EVP_sha3_512(),                 // const EVP_MD *evp_md
                key.data(),                     // const void *key
                static_cast<int>(key.size()),   // int key_len
                data.data(),                    // const unsigned char* d,
                static_cast<int>(data.size()),  // int n
                result.data(),                  // unsigned char* md
                &md_len                         // unsigned int* md_len
            );
        if (res != result.data() || static_cast<size_t>(md_len) != result.size())
            throw Exception("HMAC");
    }

    void random_bytes(rng_t* rng, std::vector<uint8_t>& bytes)
    {
        if (rng->useBytes())
        {
            for (size_t i =  0; i < bytes.size(); i++)
                bytes[i] = rng->pop();
        }
        else
        {
            if (RAND_bytes(static_cast<uint8_t*>(bytes.data()), bytes.size()) != 1)
                throw Exception("RAND_bytes");
        }
    }

    int rand(rng_t* rng)
    {
        std::vector<uint8_t> bytes(sizeof(int));
        random_bytes(rng, bytes);
        int ans = *(int*)bytes.data();
        return ans & INT_MAX;
    }

    std::vector<int> rand_select(rng_t* rng, int n, int m)
    {
        if (!(0 < m && m <= n))
            throw Exception("rand_select -- !(0 < m && m <= n)");
        std::vector<int> xs;
        for (int i = 0; i < n; i++)
            xs.push_back(i);
        std::vector<int> ans;
        for (int i = 0; i < m; i++)
        {
            int k = rand(rng) % (n - i);
            ans.push_back(xs[k + i]);
            xs[k + i] = xs[i];
        }
        xs.clear();
        return ans;
    }
}
