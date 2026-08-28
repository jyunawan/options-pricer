# options-pricer

A header-only C++20 library for pricing options. 
A study of the numerical methods in derivatives pricing, based on Hull's *Options, Futures, and Other Derivatives*.

## Status

Milestones:
- [x] **0 — Project skeleton.** CMake, header-only `INTERFACE` target, Catch2 via `FetchContent`.
- [x] **1 — Normal CDF and Black–Scholes closed form.**
- [ ] 2 — Analytic Greeks (delta, gamma, vega, theta, rho)
- [ ] 3 — Payoff abstraction (payoffs separated from engines)
- [ ] 4 — Binomial tree (CRR; European and American)
- [ ] 5 — Monte Carlo (antithetic and control variates)
- [ ] 6 — Implied volatility (bracketed Newton–Raphson)
- [ ] 7 — Finite differences (Crank–Nicolson with Rannacher smoothing)

## Build and test

Requires a C++20 compiler and CMake ≥ 3.20. Catch2 is fetched automatically at
configure time; nothing else is needed.

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Use

```cpp
#include "optpricer/black_scholes.hpp"

const optpricer::BlackScholesInputs v{
    /*S_0*/ 100.0, /*K*/ 100.0, /*r*/ 0.05,
    /*sigma*/ 0.20, /*T*/ 1.0
};

const double c = optpricer::BlackScholesPrice(v, optpricer::OptionType::Call);
```

## Verification

Prices at the reference inputs `S₀ = 100, K = 100, r = 0.05, σ = 0.20, T = 1`:

| Quantity | Value |
|---|---|
| European call `c` | 10.450583572186 |
| European put `p` | 5.573526022257 |

Both are asserted to 1e-9 in `tests/test_black_scholes.cpp`, along with the
intrinsic value returned at expiry (`T = 0`).

Put–call parity, `c + Ke^(−rT) = p + S₀`, is checked across a grid of
`K, r, σ, T` in `tests/test_parity.cpp`, also to 1e-9. Parity is model-free, so
it is a check on the implementation rather than on the model.

## Notation

| Symbol | Meaning |
|---|---|
| `S_0` | underlying price today |
| `K` | strike price |
| `r` | continuously compounded riskless rate, per year |
| `sigma` | volatility, per year |
| `T` | time to maturity, **in years** |
| `N(x)` | standard normal CDF |

All rates and times are annualised, and rates are continuously compounded.

## Design notes

**Header-only, deliberately.** The library is a CMake `INTERFACE` target that hands its include directory to whatever links it, so there is no `src/` and nothing to compile separately.

## Known limitations

- European options only, and no dividend yield. The model assumes a non-dividend-paying underlying.
- `apps/price_cli.cpp` is a stub. There is no command-line interface yet.
- `S_0 = K = 0` gives NaN (IEEE arithmetic makes `S_0 = 0` or `K = 0` alone work correctly, but not both at once).

## Disclaimer

I used an AI assistant while building this, mainly to explain concepts I was
learning and to sanity-check my work against the textbook. The design decisions and code are mine
