#include <cmath>
#include <numbers>
#ifndef NORMAL_HPP
#define NORMAL_HPP

namespace optpricer
{
    // cumulative probability distribution function for a variable with a standard normal distribution
    inline double N(double x)
    {
        return 0.5 * std::erfc(-x * std::numbers::sqrt2 / 2.0);
    }
}

#endif
