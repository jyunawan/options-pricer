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
     * Stricter validation for the greeks: on top of validate(), requires
     * sigma > 0 and T > 0, since every greek divides by sigma or sqrt(T)
     * and has no finite value at the degenerate boundary.
     * Throws std::invalid_argument otherwise.
     */
    inline void validate_greeks(const BlackScholesInputs &v)
    {
        validate(v);
        if (v.sigma <= 0.0 || v.T <= 0.0)
        {
            throw std::invalid_argument("BlackScholes greeks: sigma and T must be positive");
        }
    }

    /**
     * Sign convention that folds the call and put formulas into one:
     * +1 for a call, -1 for a put. Substituting w = -1 into any of the
     * pricing or greek expressions below yields the put form.
     */
    inline double cp_sign(const OptionType t)
    {
        return t == OptionType::Call ? 1.0 : -1.0;
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
     * Closed-form Black-Scholes price of a European option with a continuous
     * dividend yield q, dispatched on option type via w = cp_sign(t).
     *
     * price = w * (S_0 e^{-qT} N(w d1) - K e^{-rT} N(w d2))
     *   call = S_0 e^{-qT} N(d1) - K e^{-rT} N(d2)
     *   put  = K e^{-rT} N(-d2) - S_0 e^{-qT} N(-d1)
     *
     * Degenerate inputs are returned as limiting payoffs rather than NaN:
     *   - T <= 0 (expired): intrinsic value, max(w * (S_0 - K), 0)
     *   - sigma == 0: the underlying grows deterministically to its forward, so
     *     the price is the discounted forward intrinsic value.
     * Requires S_0, K, sigma >= 0
     */
    inline double BlackScholesPrice(const BlackScholesInputs &v, const OptionType t)
    {
        validate(v);
        const double w = cp_sign(t);

        if (v.T <= 0.0)
        {
            return std::max(w * (v.S_0 - v.K), 0.0);
        }

        // zero volatility: the underlying grows deterministically to its forward
        if (v.sigma == 0.0)
        {
            return std::max(w * (v.S_0 * std::exp(-v.q * v.T) - v.K * std::exp(-v.r * v.T)), 0.0);
        }

        return w * (v.S_0 * std::exp(-v.q * v.T) * N(w * d1(v)) - v.K * std::exp(-v.r * v.T) * N(w * d2(v)));
    }

    /**
     * Delta: d(price)/d(S_0), the sensitivity of the price to a small move in
     * the underlying, dispatched on option type via w = cp_sign(t).
     *
     * delta = w * exp(-q*T) * N(w * d1)
     *   call =  exp(-q*T) * N(d1),   in [0, 1]
     *   put  = -exp(-q*T) * N(-d1),  in [-1, 0]
     *
     * Requires sigma > 0 and T > 0
     */
    inline double BlackScholesDelta(const BlackScholesInputs &v, const OptionType t)
    {
        validate_greeks(v);
        const double w = cp_sign(t);
        return w * std::exp(-v.q * v.T) * N(w * d1(v));
    }

    /**
     * Gamma: d2(price)/d(S_0)^2, i.e. the rate of change of delta with the
     * underlying. Identical for calls and puts, always >= 0.
     *
     * call = put = exp(-q*T) * N'(d1) / (S_0 * sigma * sqrt(T))
     *
     * Requires sigma > 0 and T > 0
     */
    inline double BlackScholesGamma(const BlackScholesInputs &v)
    {
        validate_greeks(v);
        return (N_prime(d1(v)) * std::exp(-v.q * v.T)) / (v.S_0 * v.sigma * std::sqrt(v.T));
    }

    /**
     * Theta: d(price)/d(t) as calendar time advances (= -d(price)/d(T)),
     * expressed per year, dispatched on option type via w = cp_sign(t). Usually
     * negative: the option loses value as expiry approaches.
     *
     * theta = -S_0 e^{-qT} N'(d1) sigma / (2 sqrt(T))
     *         + w * (q S_0 e^{-qT} N(w d1) - r K e^{-rT} N(w d2))
     *
     * The first term is the time decay of the option's convexity and is shared;
     * only the carry terms flip sign between call and put.
     *
     * Requires sigma > 0 and T > 0
     */
    inline double BlackScholesTheta(const BlackScholesInputs &v, const OptionType t)
    {
        validate_greeks(v);
        const double w = cp_sign(t);
        const double D1 = d1(v);
        const double D2 = d2(v);

        return -(v.S_0 * std::exp(-v.q * v.T) * N_prime(D1) * v.sigma) / (2 * std::sqrt(v.T)) + w * (v.q * v.S_0 * std::exp(-v.q * v.T) * N(w * D1) - v.r * v.K * std::exp(-v.r * v.T) * N(w * D2));
    }

    /**
     * Vega: d(price)/d(sigma), the sensitivity to volatility, per unit change
     * in sigma (i.e. 1.0 = 100 vol points). Identical for calls and puts and
     * always >= 0.
     *
     * call = put = S_0 * exp(-q*T) * N'(d1) * sqrt(T)
     *
     * Requires sigma > 0 and T > 0
     */
    inline double BlackScholesVega(const BlackScholesInputs &v)
    {
        validate_greeks(v);
        return (v.S_0 * std::sqrt(v.T) * N_prime(d1(v)) * std::exp(-v.q * v.T));
    }

    /**
     * Rho: d(price)/d(r), per unit change in the risk-free rate, dispatched on
     * option type via w = cp_sign(t).
     *
     * rho = w * K * T * exp(-r*T) * N(w * d2)
     *   call =  K * T * exp(-r*T) * N(d2),   >= 0
     *   put  = -K * T * exp(-r*T) * N(-d2),  <= 0
     *
     * Requires sigma > 0 and T > 0
     */
    inline double BlackScholesRho(const BlackScholesInputs &v, const OptionType t)
    {
        validate_greeks(v);
        const double w = cp_sign(t);
        return w * v.K * v.T * std::exp(-v.r * v.T) * N(w * d2(v));
    }

}
#endif
