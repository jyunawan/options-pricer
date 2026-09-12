#pragma once
#include <algorithm>
#include <utility>
#include <vector>

/**
 * \file payoff.hpp
 * Terminal payoff functions for option contracts, separated from any pricing
 * engine. An engine produces a terminal underlying price S and hands it to a
 * Payoff; the Payoff turns it into a cash amount.
 */

namespace optpricer
{
    /**
     * Abstract payoff: a function object mapping the terminal underlying price
     * S to the amount the holder receives. Engines take a const Payoff& and
     * call it, so adding a contract means adding a subclass and nothing else.
     */
    class Payoff
    {
    public:
        virtual ~Payoff() = default;

        /**
         * Payoff received when the underlying finishes at price S.
         */
        virtual double operator()(double S) const = 0;
    };

    /**
     * European call: max(S - K, 0).
     */
    class CallPayoff final : public Payoff
    {
    public:
        explicit CallPayoff(double K) : K_(K) {}
        double operator()(double S) const override { return std::max(S - K_, 0.0); }

    private:
        double K_; ///< strike price
    };

    /**
     * European put: max(K - S, 0).
     */
    class PutPayoff final : public Payoff
    {
    public:
        explicit PutPayoff(double K) : K_(K) {}
        double operator()(double S) const override { return std::max(K_ - S, 0.0); }

    private:
        double K_; ///< strike price
    };

    /**
     * Cash-or-nothing (binary) call: pays a fixed amount Q if S > K, otherwise
     * nothing. Closed-form price is Q e^{-rT} N(d2).
     */
    class CashOrNothingCall final : public Payoff
    {
    public:
        CashOrNothingCall(double K, double Q = 1.0) : K_(K), Q_(Q) {}
        double operator()(double S) const override { return S > K_ ? Q_ : 0.0; }

    private:
        double K_; ///< strike price
        double Q_; ///< cash amount paid when in the money
    };

    /**
     * Asset-or-nothing (binary) call: pays the underlying price S if S > K,
     * otherwise nothing. Closed-form price is S_0 e^{-qT} N(d1).
     */
    class AssetOrNothingCall final : public Payoff
    {
    public:
        explicit AssetOrNothingCall(double K) : K_(K) {}
        double operator()(double S) const override { return S > K_ ? S : 0.0; }

    private:
        double K_; ///< strike price
    };

    /**
     * Weighted sum of vanilla legs. Covers straddles, strangles, strips, straps,
     * etc. without a class per strategy.
     */
    class PortfolioPayoff final : public Payoff
    {
    public:
        void add(double weight, CallPayoff leg) { calls_.emplace_back(weight, leg); }
        void add(double weight, PutPayoff leg) { puts_.emplace_back(weight, leg); }

        double operator()(double S) const override
        {
            double v = 0.0;
            for (auto &[w, c] : calls_)
                v += w * c(S);
            for (auto &[w, p] : puts_)
                v += w * p(S);
            return v;
        }

    private:
        std::vector<std::pair<double, CallPayoff>> calls_;
        std::vector<std::pair<double, PutPayoff>> puts_;
    };

}
