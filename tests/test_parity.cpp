#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "optpricer/black_scholes.hpp"

using namespace optpricer;
using Catch::Matchers::WithinAbs;

// Put-call parity c + K e^{-rT} = p + S0 e^{-qT}
TEST_CASE("Put-call parity holds to machine precision")
{
    const double S0 = GENERATE(100.0, 100.0, 100.0);
    const double K = GENERATE(80.0, 100.0, 120.0);
    const double r = GENERATE(0.03, 0.05);
    const double s = GENERATE(0.1, 0.2, 0.3);
    const double T = GENERATE(1.0, 1.5, 2.0);
    const double q = GENERATE(0.0, 0.03);

    BlackScholesInputs v{S0, K, r, s, T, q};
    const double lhs = BlackScholesPrice(v, OptionType::Call) + K * std::exp(-r * T);
    const double rhs = BlackScholesPrice(v, OptionType::Put) + S0 * std::exp(-q * T);
    REQUIRE_THAT(lhs, WithinAbs(rhs, 1e-9));
}
