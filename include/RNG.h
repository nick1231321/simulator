#pragma once
#include<random>
#include"modernize.h"
#include<algorithm>
#include <iomanip>
#include <chrono>
#include <numeric>

namespace RNG {
    inline bool useWeighted=false;
inline double zipfExponent = 1.5; // you can adjust this exponent
    inline u32* generateRandomSortedInts(u32 N, u32 maxValueExclusive) {
        u32* ans = (u32*)malloc(N * sizeof(u32));
        assertWithMessage(ans!=nullptr, "error, allocation failure");

        for (u32 i = 0; i < N; ++i) {
            ans[i] = std::rand() % maxValueExclusive;
        }
        std::sort(ans, ans + N);

        return ans;
    }

    inline u32* generateRandomIntsWithSpecificSum(u32 N, u32 sum) {
        u32* ans = (u32*)malloc(N * sizeof(u32));
        assertWithMessage(ans!=nullptr, "error, allocation failure");

        //Imagine L = sum balls, we want to divide them into buckets
        //We will need N-1 separators, they take L+1 positions because it can be in front of the first and behind the last

        // example with 5 balls and 5 separators, which corresponds to 6 numbers
        // | * | ** | ** | |
        // This is 0 1 2 2 0 0

        //So, we generate N-1 separators, their value goes from 0 to L inclusive
        u32* separators = generateRandomSortedInts(N-1, sum+1); //sum+1 because it takes maxExclusive
        defer(free(separators));

        ans[0] = separators[0];
        for (u32 i = 1; i <= N-2; ++i ) {
            ans[i] = separators[i] - separators[i-1];
        }
        ans[N-1] = sum-separators[N-2];

        return ans;
    }
    
    inline std::tuple<u32, u32, u32> generate3RandomInts(u32 minInclusive, u32 maxExclusive) { //Extremely lazy but good-enough implementation
        u32 N = maxExclusive - minInclusive;
        assertWithMessage(N >= 3, "Error, need to generate 3 distinct numbers");
        u32 a1 = rand() % N;
        u32 a2 = rand() % N;
        while (a2 == a1) {
            a2 = rand() % N;
        }
        u32 a3 = rand() % N;
        while (a3 == a1 || a3 == a2) {
            a3 = rand() % N;
        }
        return {a1 + minInclusive, a2 + minInclusive, a3 + minInclusive};
    }
inline std::mt19937& getEngine() {
    static std::mt19937 engine(
        static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
        )
    );
    return engine;
}

inline bool coinFlip(double probability) {
    assertWithMessage(probability >= 0.0 && probability <= 1.0,
                      "Probability must be between 0 and 1.");
    std::uniform_real_distribution<double> dist(0.0, 1.0); // no static
    return dist(getEngine()) < probability;
}


    // One uniform distribution per tag
    inline std::array<std::uniform_int_distribution<>, static_cast<size_t>(LocalNodeTag::_COUNT)>
        uniformDistributions;

    // One weighted distribution per tag
    inline std::array<std::discrete_distribution<>, static_cast<size_t>(LocalNodeTag::_COUNT)>
        weightedDistributions;

    // Population sizes
    inline std::array<size_t, static_cast<size_t>(LocalNodeTag::_COUNT)> nodePops;

    // Initialize distributions
    inline void initializeDistributions() {
    // Fill nodePops from allLocalNodes::allNodePops
    nodePops[static_cast<size_t>(LocalNodeTag::EntryRelay)]       = allLocalNodes::allNodePops.entryRelaysPop;
    nodePops[static_cast<size_t>(LocalNodeTag::MiddleRelay)]      = allLocalNodes::allNodePops.middleRelaysPop;
    nodePops[static_cast<size_t>(LocalNodeTag::ExitRelay)]        = allLocalNodes::allNodePops.exitRelaysPop;
    nodePops[static_cast<size_t>(LocalNodeTag::Client)]           = allLocalNodes::allNodePops.clientsPop;
    nodePops[static_cast<size_t>(LocalNodeTag::Server)]           = allLocalNodes::allNodePops.serversPop;
    nodePops[static_cast<size_t>(LocalNodeTag::AnonymousService)] = allLocalNodes::allNodePops.anonymousServicesPop;

    for (size_t i = 0; i < nodePops.size(); ++i) {
        const size_t pop = nodePops[i];
        if (pop > 0) {
            // Always prepare uniform distribution
            uniformDistributions[i] = std::uniform_int_distribution<>(0, static_cast<int>(pop) - 1);
        }

        // Zipf ONLY for Server and AnonymousService (if requested and pop > 0)
        const bool isServerOrAnon =
            (i == static_cast<size_t>(LocalNodeTag::Server)) ||
            (i == static_cast<size_t>(LocalNodeTag::AnonymousService));

        if (useWeighted && isServerOrAnon && pop > 0) {
            std::vector<double> weights(pop);
            double sum = 0.0;
            for (size_t k = 0; k < pop; ++k) {
                // rank k -> k+1 (1-indexed)
                const double w = 1.0 / std::pow(static_cast<double>(k + 1), zipfExponent);
                weights[k] = w;
                sum += w;
            }
            for (auto &w : weights) w /= sum; // normalize
            weightedDistributions[i] = std::discrete_distribution<>(weights.begin(), weights.end());
        } else {
            // Fallback: make weighted == uniform for other tags (or when not using Zipf)
            if (pop > 0) {
                std::vector<double> weights(pop, 1.0 / static_cast<double>(pop));
                weightedDistributions[i] = std::discrete_distribution<>(weights.begin(), weights.end());
            }
        }
    }
}

inline int getRandomIndexForTag(LocalNodeTag tag) {
    if (tag == LocalNodeTag::Client) {
        std::cout << "Unsupported for Client nodes.\n";
        std::exit(-1);
    }

    const size_t idx = static_cast<size_t>(tag);
    if (nodePops[idx] == 0) {
        std::cout << "No nodes for this tag.\n";
        std::exit(-1);
    }

    // Use Zipf (weightedDistributions) only for Server and AnonymousService when requested
    const bool isServerOrAnon =
        (tag == LocalNodeTag::Server) || (tag == LocalNodeTag::AnonymousService);

    if (useWeighted && isServerOrAnon) {
        return weightedDistributions[idx](getEngine()); // Zipf
    } else {
        return uniformDistributions[idx](getEngine());  // uniform for everything else
    }
}


void testUniformRandom(LocalNodeTag tag, int samples = 1000000) {
    // Initialize distributions

    // Get population size
    size_t popSize = RNG::nodePops[static_cast<size_t>(tag)];
    std::vector<int> counts(popSize, 0);

    // Run samples
    for (int i = 0; i < samples; ++i) {
        int idx = RNG::getRandomIndexForTag(tag);
        if (idx >= 0) counts[idx]++;
    }

    // Print results
    std::cout << "Uniform Random Test for tag: " << static_cast<int>(tag)
              << " with " << samples << " samples\n";

    for (size_t i = 0; i < popSize; ++i) {
        double percentage = (double)counts[i] / samples * 100.0;
        std::cout << "Index " << i << ": " << counts[i]
                  << " (" << std::fixed << std::setprecision(2) << percentage << "%)\n";
    }
    }

}

class UserSessionSimulator {
private:
    // Random engine and distributions as static so they persist
    static std::mt19937& getGenerator() {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    static int truncatedPoisson(double lambda) {
        static std::poisson_distribution<int> dist(lambda);
        int pages = 0;
        auto& gen = getGenerator();
        while (pages < 1) {
            pages = dist(gen);
        }
        return pages;
    }

    static double sampleLogNormal(double median, double sigma) {
        double mu = std::log(median); // median = exp(mu)
        std::lognormal_distribution<double> dist(mu, sigma);
        return dist(getGenerator());
    }

    static double sampleExponential(double mean) {
        std::exponential_distribution<double> dist(1.0 / mean); // λ = 1/mean
        return dist(getGenerator());
    }

public:
    struct Session {
        int pages;
        std::vector<double> dwell_times;
        double total_duration;
    };

    static Session simulateSession(double lambda_pages = 2.5,
                                   double dwell_median = 52.0,
                                   double dwell_sigma = 0.8,
                                   double interpage_mean = 3.0) 
    {
        int pages = truncatedPoisson(lambda_pages);

        std::vector<double> dwell_times;
        dwell_times.reserve(pages);
        for (int i = 0; i < pages; ++i) {
            dwell_times.push_back(sampleLogNormal(dwell_median, dwell_sigma));
        }

        double inter_delay_sum = 0.0;
        for (int i = 0; i < pages - 1; ++i) {
            inter_delay_sum += sampleExponential(interpage_mean);
        }

        double total_dwell = std::accumulate(dwell_times.begin(), dwell_times.end(), 0.0);
        double total_duration = total_dwell + inter_delay_sum;

        return { pages, dwell_times, total_duration };
    }
};
