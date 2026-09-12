#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "optpricer/payoff.hpp"

using namespace optpricer;
using Catch::Matchers::WithinAbs;

TEST_CASE("Vanilla payoffs")
{
    const CallPayoff c{100.0};
    const PutPayoff p{100.0};

    REQUIRE(c(110.0) == 10.0);
    REQUIRE(c(100.0) == 0.0);
    REQUIRE(c(90.0) == 0.0);

    REQUIRE(p(90.0) == 10.0);
    REQUIRE(p(100.0) == 0.0);
    REQUIRE(p(110.0) == 0.0);
}

TEST_CASE("Binary payoffs")
{
    const CashOrNothingCall con{100.0, 5.0};
    const AssetOrNothingCall aon{100.0};

    REQUIRE(con(100.01) == 5.0);
    REQUIRE(con(100.0) == 0.0); // strictly in the money only
    REQUIRE(con(99.0) == 0.0);
    REQUIRE(CashOrNothingCall{100.0}(150.0) == 1.0); // default Q

    REQUIRE(aon(120.0) == 120.0);
    REQUIRE(aon(100.0) == 0.0);
    REQUIRE(aon(80.0) == 0.0);
}

// Binary decomposition: a call = asset-or-nothing call - K * cash-or-nothing call.
// Holds pointwise in S, so it is exact at the payoff level with no engine.
TEST_CASE("Call decomposes into binaries")
{
    const double K = GENERATE(80.0, 100.0, 120.0);
    const double S = GENERATE(0.0, 50.0, 99.999, 100.0, 100.001, 150.0);
    const CallPayoff c{K};
    const AssetOrNothingCall aon{K};
    const CashOrNothingCall con{K};
    REQUIRE_THAT(c(S), WithinAbs(aon(S) - K * con(S), 1e-12));
}

TEST_CASE("PortfolioPayoff: straddle, strangle and empty")
{
    const double S = GENERATE(70.0, 95.0, 100.0, 105.0, 130.0);

    PortfolioPayoff empty;
    REQUIRE(empty(S) == 0.0);

    // Straddle: long call + long put at the same K.
    PortfolioPayoff straddle;
    straddle.add(1.0, CallPayoff{100.0});
    straddle.add(1.0, PutPayoff{100.0});
    REQUIRE_THAT(straddle(S), WithinAbs(std::abs(S - 100.0), 1e-12));

    // Strangle: put at 95, call at 105.
    PortfolioPayoff strangle;
    strangle.add(1.0, PutPayoff{95.0});
    strangle.add(1.0, CallPayoff{105.0});
    REQUIRE_THAT(strangle(S), WithinAbs(std::max(95.0 - S, 0.0) + std::max(S - 105.0, 0.0), 1e-12));

    // Weights scale and can be negative (a short leg).
    PortfolioPayoff spread; // bull call spread 100/110, 2x
    spread.add(2.0, CallPayoff{100.0});
    spread.add(-2.0, CallPayoff{110.0});
    REQUIRE_THAT(spread(S), WithinAbs(2.0 * (std::max(S - 100.0, 0.0) - std::max(S - 110.0, 0.0)), 1e-12));
}
