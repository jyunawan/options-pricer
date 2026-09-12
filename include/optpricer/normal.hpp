#include <cmath>
#include <numbers>
#ifndef NORMAL_HPP
#define NORMAL_HPP

/**
 * \file normal.hpp
 * Standard normal distribution helpers used by the pricing formulas.
 */

namespace optpricer
{
    /**
     * Standard normal CDF: P(Z <= x) for Z ~ N(0, 1).
     */
    inline double N(double x)
    {
        return 0.5 * std::erfc(-x * std::numbers::sqrt2 / 2.0);
    }

    /**
     * Standard normal PDF: the density phi(x) = exp(-x^2 / 2) / sqrt(2*pi)
     * for Z ~ N(0, 1). This is N'(x), the derivative of the CDF.
     */
    inline double N_prime(double x)
    {
        return std::exp(-0.5 * x * x) / std::sqrt(2.0 * std::numbers::pi);
    }
}

#endif
