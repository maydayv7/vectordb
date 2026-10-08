#include "vectordb/flat_search.hpp"
#include "vectordb/vector_store_io.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    try {
        const auto base = vectordb::VectorStoreIO::read_vecs<float>(
            "data/siftsmall/siftsmall_base.fvecs");
        const auto queries = vectordb::VectorStoreIO::read_vecs<float>(
            "data/siftsmall/siftsmall_query.fvecs");
        if (base.size() != 10'000 || queries.size() != 100 ||
            base.front().dimension() != 128 ||
            queries.front().dimension() != 128) {
            throw std::runtime_error("Expected SIFT-small: 10000 base vectors, "
                                     "100 queries, 128 dimensions");
        }

        benchmark::RegisterBenchmark(
            "FlatSearch",
            [&base, &queries](benchmark::State &state) {
                const auto k = static_cast<std::size_t>(state.range(0));
                const auto batch_size =
                    static_cast<benchmark::IterationCount>(queries.size());
                while (state.KeepRunningBatch(batch_size)) {
                    for (const auto &query : queries) {
                        auto ids = vectordb::flat_search(query.vector, base, k);
                        benchmark::DoNotOptimize(ids);
                        benchmark::ClobberMemory();
                    }
                }
                state.SetItemsProcessed(state.iterations());
            })
            ->ArgName("k")
            ->Arg(10)
            ->Arg(100)
            ->UseRealTime()
            ->Unit(benchmark::kMicrosecond);

        benchmark::RunSpecifiedBenchmarks();
        benchmark::Shutdown();
    } catch (const std::exception &error) {
        std::cerr << "Benchmark failed: " << error.what()
                  << "\nRun from the project root after fetching SIFT-small.\n";
        return 1;
    }
}
