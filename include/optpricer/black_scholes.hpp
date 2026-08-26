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
        const double D1 = d1(v);
        const double D2 = d2(v);

        return v.S_0 * N(D1) - v.K * std::exp(-v.r * v.T) * N(D2);
    }

    inline double BlackScholesPut(const BlackScholesInputs &v)
    {
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
