//
// Created by yashr on 4/29/24.
//

#include <vector>
#include <random>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <benchmark/benchmark.h>

#include "hashtable.h"
#include "emhash_map8.hpp"
#include "cpuinfo.h"


static void UM_InsertionIteration(benchmark::State &state) {
    auto map = std::unordered_map<uint64_t, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        map.emplace(dist(e), dist(e));
    }

    for (auto kv : map) {
        assert(kv.first != kv.second);
    }
}

static void BM_InsertionIteration(benchmark::State &state) {
    auto map = BananSTLExt::BananHashMap<uint64_t, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        uint64_t key = dist(e);
        uint64_t val = dist(e);

        map.emplace(key, val);
    }
}

static void EM_InsertionIteration(benchmark::State &state) {
    auto map = emhash8::HashMap<uint64_t, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        map.emplace(dist(e), dist(e));
    }

    for (auto kv : map) {
        assert(kv.first != kv.second);
    }
}

static void UM_StringInsertionIteration(benchmark::State &state) {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    auto map = std::unordered_map<std::basic_string<char>, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
    std::uniform_int_distribution<uint64_t> dist2(0, 32);

    for (auto _: state) {
        std::shuffle(str.begin(), str.end(), e);
        map.emplace(str.substr(0, dist2(e)), dist(e));
    }

    map.clear();
}

static void BM_StringInsertionIteration(benchmark::State &state) {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    auto map = BananSTLExt::BananHashMap<std::basic_string<char>, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
    std::uniform_int_distribution<uint64_t> dist2(0, 32);

    for (auto _: state) {
        std::shuffle(str.begin(), str.end(), e);
        map.emplace(str.substr(0, dist2(e)), dist(e));
    }

    map.clear();
}

static void EM_StringInsertionIteration(benchmark::State &state) {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    auto map = emhash8::HashMap<std::basic_string<char>, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
    std::uniform_int_distribution<uint64_t> dist2(0, 32);

    for (auto _: state) {
        std::shuffle(str.begin(), str.end(), e);
        map.emplace(str.substr(0, dist2(e)), dist(e));
    }

    map.clear();
}

BENCHMARK(UM_InsertionIteration);
BENCHMARK(BM_InsertionIteration);
BENCHMARK(EM_InsertionIteration);
BENCHMARK(UM_StringInsertionIteration);
BENCHMARK(BM_StringInsertionIteration);
BENCHMARK(EM_StringInsertionIteration);

BENCHMARK_MAIN();

/**
static void BMAP() {
    auto map = BananSTLExt::BananHashMap<uint64_t, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (int i = 0; i < 1000000; i++) {
        uint64_t key = dist(e);
        uint64_t val = dist(e);

        map.emplace(key, val);
    }
}

static void EMAP() {
    auto map = emhash8::HashMap<uint64_t, uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (int i = 0; i < 1000000; i++) {
        map.emplace(dist(e), dist(e));
    }

    for (auto kv : map) {
        assert(kv.first != kv.second);
    }
}
**/