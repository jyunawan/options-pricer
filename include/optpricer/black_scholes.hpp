#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "optpricer/normal.hpp"
#ifndef BLACK_SCHOLES_HPP
#define BLACK_SCHOLES_HPP

namespace optpricer
{
    enum class OptionType
    {
        Call,
        Put
    };

    struct BlackScholesInputs
    {
        double S_0;   // underlying asset's current price
        double K;     // strike price
        double r;     // risk-free interest rate (continuously compounded)
        double sigma; // volatility of underlying asset
        double T;     // time to maturity (YEARS)
    };

    inline void validate(const BlackScholesInputs &v)
    {
        if (v.S_0 < 0.0 || v.K < 0.0 || v.sigma < 0.0)
        {
            throw std::invalid_argument("BlackScholes: S_0, K and sigma must be non-negative");
        }
    }

    inline double d1(const BlackScholesInputs &v)
    {
        return (std::log(v.S_0 / v.K) + (v.r + v.sigma * v.sigma / 2) * v.T) / (v.sigma * std::sqrt(v.T));
    }

    inline double d2(const BlackScholesInputs &v)
    {
        return d1(v) - v.sigma * std::sqrt(v.T);
    }

    inline double BlackScholesCall(const BlackScholesInputs &v)
    {
        validate(v);

        if (v.T <= 0.0)
        {
            return std::max(v.S_0 - v.K, 0.0);
        }

        // zero volatility: the underlying grows deterministically to its forward
        if (v.sigma == 0.0)
        {
            return std::max(v.S_0 - v.K * std::exp(-v.r * v.T), 0.0);
        }

        const double D1 = d1(v);
        const double D2 = d2(v);

        return v.S_0 * N(D1) - v.K * std::exp(-v.r * v.T) * N(D2);
    }

    inline double BlackScholesPut(const BlackScholesInputs &v)
    {
        validate(v);

        if (v.T <= 0.0)
        {
            return std::max(v.K - v.S_0, 0.0);
        }

        // zero volatility: the underlying grows deterministically to its forward
        if (v.sigma == 0.0)
        {
            return std::max(v.K * std::exp(-v.r * v.T) - v.S_0, 0.0);
        }

        const double D1 = d1(v);
        const double D2 = d2(v);

        return v.K * std::exp(-v.r * v.T) * N(-D2) - v.S_0 * N(-D1);
    }

    inline double BlackScholesPrice(const BlackScholesInputs &v, const OptionType t)
    {
        switch (t)
        {
        case OptionType::Call:
            return BlackScholesCall(v);
        case OptionType::Put:
            return BlackScholesPut(v);
        }
        throw std::invalid_argument("unreachable: invalid OptionType");
    }

}

#endif
