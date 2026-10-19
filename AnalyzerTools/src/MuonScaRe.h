#ifndef MUONSCARE_H
#define MUONSCARE_H

// Building blocks of the MUO POG muon momentum scale and resolution
// correction ("MuonScaRe", muon_scalesmearing.json.gz).  Transcribed from the
// POG reference implementation shipped in jsonpog-integration
// (examples/MuonScaRe.cc and MuonScaRe.py); the formulas are the POG's, only
// the guards against unphysical inputs follow the Python version, which is
// the more defensive of the two.
//
// Header-only and kept under src/ on purpose: nothing here belongs in the
// ROOT dictionary, and the unit test includes it directly.

#include <cmath>
#include <limits>

#include "TMath.h"

namespace SKNano {

// Double-sided Crystal Ball with an analytic CDF and inverse CDF.  The
// resolution correction draws its random number from this shape: a flat
// u in [0,1) goes through invcdf() and comes out distributed like the
// per-muon pull measured in simulation.
struct MuonCrystalBall {
  double m = 0.;  // mean
  double s = 1.;  // sigma
  double a = 10.; // alpha, |a| is where the Gaussian core hands over to the tails
  double n = 10.; // tail power

  MuonCrystalBall() { init(); }
  MuonCrystalBall(double mean, double sigma, double alpha, double power)
      : m(mean), s(sigma), a(alpha), n(power) {
    init();
  }

  // pi is truncated exactly as in the POG reference so the two agree
  // bit-for-bit rather than only to rounding.
  static constexpr double pi = 3.14159;
  double sqrtPiOver2 = std::sqrt(pi / 2.0);
  double sqrt2 = std::sqrt(2.0);
  double B = 0., C = 0., D = 0., N = 0., NA = 0., Ns = 0., NC = 0., F = 0.,
         G = 0., k = 0.;
  double cdfMa = 0., cdfPa = 0.;

  void init() {
    const double fa = std::fabs(a);
    const double ex = std::exp(-fa * fa / 2);
    const double A = std::pow(n / fa, n) * ex;
    const double C1 = n / fa / (n - 1) * ex;
    const double D1 = 2 * sqrtPiOver2 * std::erf(fa / sqrt2);
    B = n / fa - fa;
    C = (D1 + 2 * C1) / C1;
    D = (D1 + 2 * C1) / 2;
    N = 1.0 / s / (D1 + 2 * C1);
    k = 1.0 / (n - 1);
    NA = N * A;
    Ns = N * s;
    NC = Ns * C1;
    F = 1 - fa * fa / n;
    G = s * n / fa;
    cdfMa = cdf(m - a * s);
    cdfPa = cdf(m + a * s);
  }

  double pdf(double x) const {
    const double d = (x - m) / s;
    if (d < -a)
      return NA * std::pow(B - d, -n);
    if (d > a)
      return NA * std::pow(B + d, -n);
    return N * std::exp(-d * d / 2);
  }

  double cdf(double x) const {
    const double d = (x - m) / s;
    if (d < -a) {
      const double base = F - s * d / G;
      // Past the point where the power law would turn over the CDF is flat.
      return base > 0 ? NC / std::pow(base, n - 1) : NC;
    }
    if (d > a) {
      const double base = F + s * d / G;
      return base > 0 ? NC * (C - std::pow(base, 1 - n)) : NC * C;
    }
    return Ns * (D - sqrtPiOver2 * std::erf(-d / sqrt2));
  }

  double invcdf(double u) const {
    if (u < cdfMa) {
      const double ratio = NC / u;
      return ratio > 0 ? m + G * (F - std::pow(ratio, k)) : m + G * F;
    }
    if (u > cdfPa) {
      const double base = C - u / NC;
      return base > 0 ? m - G * (F - std::pow(base, -k)) : m - G * F;
    }
    return m - sqrt2 * s * TMath::ErfInverse((D - u / Ns) / sqrtPiOver2);
  }
};

// The POG corrections are derived for 26 < pt < 200 GeV and are applied
// nowhere else; outside that window a muon keeps its NanoAOD pt.
constexpr float MUON_SCARE_MIN_PT = 26.f;
constexpr float MUON_SCARE_MAX_PT = 200.f;

// The POG guards: a corrected pt that is not finite, negative, or off by more
// than a factor 2 (0.1) from the input is an artefact of a sparsely populated
// bin and is discarded in favour of the input.
inline bool MuonScaReResultIsSane(const double corrected, const double input) {
  if (!std::isfinite(corrected) || corrected < 0.)
    return false;
  if (!(input > 0.))
    return false;
  const double ratio = corrected / input;
  return ratio <= 2. && ratio >= 0.1;
}

} // namespace SKNano

#endif
