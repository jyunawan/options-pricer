#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "optpricer/normal.hpp"
#ifndef BLACK_SCHOLES_HPP
#define BLACK_SCHOLES_HPP

/**
 * \file black_scholes.hpp
 * Closed-form Black-Scholes pricing for European options with a continuous
 * dividend yield.
 */

/**
 * European option pricing under the Black-Scholes model.
 */
namespace optpricer
{
    /**
     * Which side of the contract to price.
     */
    enum class OptionType
    {
        Call, ///< right to buy the underlying at K
        Put   ///< right to sell the underlying at K
    };

    /**
     * Market and contract parameters for a European option under Black-Scholes.
     * All rates are annualised and continuously compounded.
     */
    struct BlackScholesInputs
    {
        double S_0;   ///< underlying asset's current price
        double K;     ///< strike price
        double r;     ///< risk-free interest rate (continuously compounded)
        double sigma; ///< volatility of underlying asset
        double T;     ///< time to maturity (YEARS)
        double q;     ///< dividend yield (continuously compounded)
    };

    /**
     * Throws std::invalid_argument if S_0, K, or sigma is negative.
     */
    inline void validate(const BlackScholesInputs &v)
    {
        if (v.S_0 < 0.0 || v.K < 0.0 || v.sigma < 0.0)
        {
            throw std::invalid_argument("BlackScholes: S_0, K and sigma must be non-negative");
        }
    }

    /**
     * Black-Scholes d1 term. Assumes sigma > 0 and T > 0.
     */
    inline double d1(const BlackScholesInputs &v)
    {
        return (std::log(v.S_0 / v.K) + (v.r - v.q + v.sigma * v.sigma / 2) * v.T) / (v.sigma * std::sqrt(v.T));
    }

    /**
     * Black-Scholes d2 term (= d1 - sigma*sqrt(T)). Assumes sigma > 0 and T > 0.
     */
    inline double d2(const BlackScholesInputs &v)
    {
        return d1(v) - v.sigma * std::sqrt(v.T);
    }

    /**
     * Price of a European call with continuous dividend yield.
     * Handles expiry (T <= 0) and zero-volatility as intrinsic/forward payoffs.
     */
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
            return std::max(v.S_0 * std::exp(-v.q * v.T) - v.K * std::exp(-v.r * v.T), 0.0);
        }

        const double D1 = d1(v);
        const double D2 = d2(v);

        return v.S_0 * std::exp(-v.q * v.T) * N(D1) - v.K * std::exp(-v.r * v.T) * N(D2);
    }

    /**
     * Price of a European put with continuous dividend yield.
     * Handles expiry (T <= 0) and zero-volatility as intrinsic/forward payoffs.
     */
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
            return std::max(v.K * std::exp(-v.r * v.T) - v.S_0 * std::exp(-v.q * v.T), 0.0);
        }

        const double D1 = d1(v);
        const double D2 = d2(v);

        return v.K * std::exp(-v.r * v.T) * N(-D2) - v.S_0 * std::exp(-v.q * v.T) * N(-D1);
    }

    /**
     * Switches between BlackScholesCall / BlackScholesPut by option type.
     */
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
