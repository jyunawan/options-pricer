#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "optpricer/black_scholes.hpp"

using namespace optpricer;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// Reference inputs from hidden/guide.md §3.2: ./price_cli call 100 100 0.05 0 0.2 1
static const BlackScholesInputs kRef{100.0, 100.0, 0.05, 0.20, 1.0, 0.0};

TEST_CASE("Greeks reference values")
{
    REQUIRE_THAT(BlackScholesDelta(kRef, OptionType::Call), WithinAbs(0.636830651176, 1e-9));
    REQUIRE_THAT(BlackScholesDelta(kRef, OptionType::Put), WithinAbs(-0.363169348824, 1e-9));
    REQUIRE_THAT(BlackScholesGamma(kRef), WithinAbs(0.018762017346, 1e-9));
    REQUIRE_THAT(BlackScholesVega(kRef), WithinAbs(37.524034691, 1e-6));
    REQUIRE_THAT(BlackScholesTheta(kRef, OptionType::Call), WithinAbs(-6.414027546, 1e-6));
    REQUIRE_THAT(BlackScholesRho(kRef, OptionType::Call), WithinAbs(53.232481545, 1e-6));
}

// ---- Analytic Greeks vs central finite differences on the pricer itself.
// central 1st derivative:  (f(x+h) - f(x-h)) / 2h
// central 2nd derivative:  (f(x+h) - 2f(x) + f(x-h)) / h^2
TEST_CASE("Greeks match bumped finite differences")
{
    const double S0 = 100.0;
    const double K = GENERATE(80.0, 100.0, 125.0);
    const double r = GENERATE(0.02, 0.05);
    const double sigma = GENERATE(0.15, 0.30);
    const double T = GENERATE(0.5, 2.0);
    const double q = GENERATE(0.0, 0.03);
    const auto type = GENERATE(OptionType::Call, OptionType::Put);

    BlackScholesInputs v{S0, K, r, sigma, T, q};
    auto price = [&](BlackScholesInputs x)
    { return BlackScholesPrice(x, type); };

    // delta: bump S_0
    {
        const double h = 1e-4 * S0;
        auto up = v, dn = v;
        up.S_0 += h;
        dn.S_0 -= h;
        const double fd = (price(up) - price(dn)) / (2 * h);
        REQUIRE_THAT(BlackScholesDelta(v, type), WithinRel(fd, 1e-5));
    }
    // gamma: second bump in S_0
    {
        const double h = 1e-3 * S0;
        auto up = v, dn = v;
        up.S_0 += h;
        dn.S_0 -= h;
        const double fd = (price(up) - 2 * price(v) + price(dn)) / (h * h);
        REQUIRE_THAT(BlackScholesGamma(v), WithinRel(fd, 1e-4));
    }
    // vega: bump sigma (analytic is per 1.00 of vol)
    {
        const double h = 1e-5;
        auto up = v, dn = v;
        up.sigma += h;
        dn.sigma -= h;
        const double fd = (price(up) - price(dn)) / (2 * h);
        REQUIRE_THAT(BlackScholesVega(v), WithinRel(fd, 1e-6));
    }
    // rho: bump r (analytic is per 1.00 of rate)
    {
        const double h = 1e-6;
        auto up = v, dn = v;
        up.r += h;
        dn.r -= h;
        const double fd = (price(up) - price(dn)) / (2 * h);
        REQUIRE_THAT(BlackScholesRho(v, type), WithinRel(fd, 1e-5));
    }
    // theta: = -d(price)/dT, per year
    {
        const double h = 1e-6;
        auto up = v, dn = v;
        up.T += h;
        dn.T -= h;
        const double fd = -(price(up) - price(dn)) / (2 * h);
        REQUIRE_THAT(BlackScholesTheta(v, type), WithinRel(fd, 1e-5));
    }
}

// Delta, theta and rho have distinct call/put formulas. Each pair differs by
// the derivative of put-call parity  c - p = S0 e^{-qT} - K e^{-rT}, so these
// relations pin the two formulas against each other exactly.
TEST_CASE("Call vs put: the Greeks with different formulas")
{
    const double S0 = 100.0;
    const double K = GENERATE(80.0, 100.0, 120.0);
    const double r = GENERATE(0.01, 0.05);
    const double sigma = GENERATE(0.15, 0.35);
    const double T = GENERATE(0.5, 2.0);
    const double q = GENERATE(0.0, 0.03);
    BlackScholesInputs v{S0, K, r, sigma, T, q};

    // d/dS0:  delta_call - delta_put = e^{-qT}
    REQUIRE_THAT(BlackScholesDelta(v, OptionType::Call) - BlackScholesDelta(v, OptionType::Put),
                 WithinAbs(std::exp(-q * T), 1e-12));

    // d/dr:  rho_call - rho_put = K T e^{-rT}
    REQUIRE_THAT(BlackScholesRho(v, OptionType::Call) - BlackScholesRho(v, OptionType::Put),
                 WithinAbs(v.K * v.T * std::exp(-r * T), 1e-9));

    // -d/dT:  theta_call - theta_put = -r K e^{-rT} + q S0 e^{-qT}
    REQUIRE_THAT(BlackScholesTheta(v, OptionType::Call) - BlackScholesTheta(v, OptionType::Put),
                 WithinAbs(-r * K * std::exp(-r * T) + q * S0 * std::exp(-q * T), 1e-9));
}

TEST_CASE("Put Greeks: reference values")
{
    // Same reference inputs as the call benchmark; put values derived from the
    // call values via the parity relations above.
    REQUIRE_THAT(BlackScholesTheta(kRef, OptionType::Put), WithinAbs(-1.657880424, 1e-6));
    REQUIRE_THAT(BlackScholesRho(kRef, OptionType::Put), WithinAbs(-41.890460904, 1e-6));
}

TEST_CASE("Greek sign and bound properties (guide §checklist)")
{
    const double S0 = 100.0;
    const double K = GENERATE(70.0, 100.0, 130.0);
    const double sigma = GENERATE(0.1, 0.5);
    const double T = GENERATE(0.25, 3.0);
    const double q = GENERATE(0.0, 0.04);
    BlackScholesInputs v{S0, K, 0.05, sigma, T, q};

    const double dc = BlackScholesDelta(v, OptionType::Call);
    const double dp = BlackScholesDelta(v, OptionType::Put);
    REQUIRE((dc >= 0.0 && dc <= 1.0));
    REQUIRE((dp >= -1.0 && dp <= 0.0));
    // call delta - put delta = e^{-qT}
    REQUIRE_THAT(dc - dp, WithinAbs(std::exp(-q * T), 1e-12));

    REQUIRE(BlackScholesGamma(v) >= 0.0);
    REQUIRE(BlackScholesVega(v) >= 0.0);
    REQUIRE(BlackScholesRho(v, OptionType::Call) >= 0.0);
    REQUIRE(BlackScholesRho(v, OptionType::Put) <= 0.0);
}

TEST_CASE("Greeks reject degenerate sigma and T")
{
    REQUIRE_THROWS_AS(BlackScholesDelta({100, 100, 0.05, 0.0, 1.0, 0.0}, OptionType::Call), std::invalid_argument);
    REQUIRE_THROWS_AS(BlackScholesGamma({100, 100, 0.05, 0.2, 0.0, 0.0}), std::invalid_argument);
    REQUIRE_THROWS_AS(BlackScholesVega({100, 100, 0.05, -0.2, 1.0, 0.0}), std::invalid_argument);
    REQUIRE_THROWS_AS(BlackScholesTheta({100, 100, 0.05, 0.2, -1.0, 0.0}, OptionType::Put), std::invalid_argument);
}
