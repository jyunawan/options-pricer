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
     * Uses erfc so the far tails stay accurate.
     */
    inline double N(double x)
    {
        return 0.5 * std::erfc(-x * std::numbers::sqrt2 / 2.0);
    }
}

#endif
