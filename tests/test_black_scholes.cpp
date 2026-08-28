#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "optpricer/black_scholes.hpp"

using namespace optpricer;
using Catch::Matchers::WithinAbs;

TEST_CASE("Black-Scholes Standard Benchmark")
{
    BlackScholesInputs v{100.0, 100.0, 0.05, 0.20, 1.0, 0.0};
    REQUIRE_THAT(BlackScholesPrice(v, OptionType::Call), WithinAbs(10.450583572186, 1e-9));
    REQUIRE_THAT(BlackScholesPrice(v, OptionType::Put), WithinAbs(5.573526022257, 1e-9));
}

TEST_CASE("Dividend yield: reference values and effect on price")
{
    // S0=100, K=100, r=0.05, sigma=0.20, T=1, q=0.03
    BlackScholesInputs with_div{100.0, 100.0, 0.05, 0.20, 1.0, 0.03};
    REQUIRE_THAT(BlackScholesPrice(with_div, OptionType::Call), WithinAbs(8.652528553943, 1e-9));
    REQUIRE_THAT(BlackScholesPrice(with_div, OptionType::Put), WithinAbs(6.730917649163, 1e-9));

    // A yield q is equivalent to discounting today's spot by e^{-qT} and setting q = 0.
    BlackScholesInputs shifted{100.0 * std::exp(-0.03), 100.0, 0.05, 0.20, 1.0, 0.0};
    REQUIRE_THAT(BlackScholesPrice(with_div, OptionType::Call),
                 WithinAbs(BlackScholesPrice(shifted, OptionType::Call), 1e-12));
    REQUIRE_THAT(BlackScholesPrice(with_div, OptionType::Put),
                 WithinAbs(BlackScholesPrice(shifted, OptionType::Put), 1e-12));

    // Dividends transfer value from the stock to its holder: calls fall, puts rise.
    BlackScholesInputs no_div{100.0, 100.0, 0.05, 0.20, 1.0, 0.0};
    REQUIRE(BlackScholesPrice(with_div, OptionType::Call) < BlackScholesPrice(no_div, OptionType::Call));
    REQUIRE(BlackScholesPrice(with_div, OptionType::Put) > BlackScholesPrice(no_div, OptionType::Put));
}

TEST_CASE("Zero volatility returns discounted intrinsic value")
{
    BlackScholesInputs atm{100.0, 100.0, 0.05, 0.0, 1.0, 0.0};
    const double disc = 100.0 * std::exp(-0.05);
    REQUIRE_THAT(BlackScholesPrice(atm, OptionType::Call), WithinAbs(100.0 - disc, 1e-12));
    REQUIRE_THAT(BlackScholesPrice(atm, OptionType::Put), WithinAbs(0.0, 1e-12));
}

TEST_CASE("Negative inputs are rejected")
{
    REQUIRE_THROWS_AS(BlackScholesPrice({-100.0, 100.0, 0.05, 0.20, 1.0, 0.0}, OptionType::Call), std::invalid_argument);
    REQUIRE_THROWS_AS(BlackScholesPrice({100.0, -100.0, 0.05, 0.20, 1.0, 0.0}, OptionType::Call), std::invalid_argument);
    REQUIRE_THROWS_AS(BlackScholesPrice({100.0, 100.0, 0.05, -0.20, 1.0, 0.0}, OptionType::Put), std::invalid_argument);
}

TEST_CASE("Black-Scholes at expiry returns intrinsic value")
{
    BlackScholesInputs itm{110.0, 100.0, 0.05, 0.20, 0.0, 0.0};
    BlackScholesInputs otm{90.0, 100.0, 0.05, 0.20, 0.0, 0.0};
    REQUIRE_THAT(BlackScholesPrice(itm, OptionType::Call), WithinAbs(10.0, 1e-12));
    REQUIRE_THAT(BlackScholesPrice(otm, OptionType::Call), WithinAbs(0.0, 1e-12));
    REQUIRE_THAT(BlackScholesPrice(otm, OptionType::Put), WithinAbs(10.0, 1e-12));
    REQUIRE_THAT(BlackScholesPrice(itm, OptionType::Put), WithinAbs(0.0, 1e-12));
}
