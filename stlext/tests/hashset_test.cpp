//
// Created by yashr on 4/29/24.
//

#include <vector>
#include <random>
#include <cassert>
#include <iostream>
#include <unordered_set>
#include <benchmark/benchmark.h>

#include "hashtable.h"
#include "emhash_set8.hpp"

/**
static void US_InsertionIteration(benchmark::State &state) {
    auto set = std::unordered_set<uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        set.emplace(dist(e));
    }

    for (auto val : set) {
        assert(val != 0);
    }
}

static void BS_InsertionIteration(benchmark::State &state) {
    auto set = BananSTLExt::BananHashSet<uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        set.emplace(dist(e));
    }

    for (auto val : set) {
        assert(val != 0);
    }
}

static void ES_InsertionIteration(benchmark::State &state) {
    auto map = emhash8::HashSet<uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (auto _: state) {
        map.emplace(dist(e));
    }

    for (auto val : map) {
        assert(val != 0);
    }
}

BENCHMARK(US_InsertionIteration);
BENCHMARK(BS_InsertionIteration);
BENCHMARK(ES_InsertionIteration);

BENCHMARK_MAIN();
**/

int main() {
    auto set = BananSTLExt::BananHashSet<uint64_t>();

    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

    for (int i = 0; i < 10000; i++) {
        set.emplace(dist(e));
    }

    for (auto val : set) {
        assert(val != 0);
    }
}