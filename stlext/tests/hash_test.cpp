//
// Created by yashr on 6/8/24.
//

#include <benchmark/benchmark.h>
#include <random>

#include "hash.h"

static void BananHashTest(benchmark::State &state) {
    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    auto hasher = BananSTLExt::BananHash<uint64_t>();

    for (auto _: state) {
        (void)hasher(dist(e));
    }
}

static void STDHashTest(benchmark::State &state) {
    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    auto hasher = std::hash<uint64_t>();

    for (auto _: state) {
        (void)hasher(dist(e));
    }
}

static void BananHashStringTest(benchmark::State &state) {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
    std::uniform_int_distribution<uint64_t> dist2(0, 32);

    auto hasher = BananSTLExt::BananHash<std::basic_string<char>>();

    for (auto _: state) {
        std::shuffle(str.begin(), str.end(), e);
        (void)hasher(str.substr(0, dist2(e)));
    }
}

static void STDHashStringTest(benchmark::State &state) {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
    std::uniform_int_distribution<uint64_t> dist2(0, 32);

    auto hasher = std::hash<std::basic_string<char>>();

    for (auto _: state) {
        std::shuffle(str.begin(), str.end(), e);
        (void)hasher(str.substr(0, dist2(e)));

    }
}

BENCHMARK(BananHashTest);
BENCHMARK(STDHashTest);
BENCHMARK(BananHashStringTest);
BENCHMARK(STDHashStringTest);

BENCHMARK_MAIN();