#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "optpricer/black_scholes.hpp"

using namespace optpricer;
using Catch::Matchers::WithinAbs;

TEST_CASE("Black-Scholes Standard Benchmark")
{
    BlackScholesInputs v{100.0, 100.0, 0.05, 0.20, 1.0};
    REQUIRE_THAT(BlackScholesPrice(v, OptionType::Call), WithinAbs(10.450583572186, 1e-9));
    REQUIRE_THAT(BlackScholesPrice(v, OptionType::Put), WithinAbs(5.573526022257, 1e-9));
}
