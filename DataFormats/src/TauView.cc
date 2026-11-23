#include "TauView.h"

#include <cmath>

TauESSource TauView::ClassifyESSource(const unsigned char dm,
                                      const unsigned char genmatch,
                                      const float eta) {
    if (!IsCalibratedDecayMode(dm))
        return TauESSource::None;
    const int slot = dm == 0 ? 0 : dm == 1 ? 1 : dm == 10 ? 2 : 3;
    if (genmatch == 5)
        return static_cast<TauESSource>(
            static_cast<int>(TauESSource::GenuineDM0) + slot);
    if (genmatch == 1 || genmatch == 3)
        return static_cast<TauESSource>(
            static_cast<int>(TauESSource::EFakeDM0Barrel) + 2 * slot +
            (std::abs(eta) < 1.5f ? 0 : 1));
    if (genmatch == 2 || genmatch == 4)
        return static_cast<TauESSource>(
            static_cast<int>(TauESSource::MuFakeDM0) + slot);
    return TauESSource::None;
}

bool TauView::PassID(const TauID &id) const {
    if (id.requireNewDM && !idDecayModeNewDMs())
        return false;
    if (id.requireCalibratedDecayMode && !IsCalibratedDecayMode(DecayMode()))
        return false;
    if (std::abs(dZ()) >= id.maxDz)
        return false;
    // The stored column is the tightest point the tau passes, so ">=" is the
    // cumulative test and an axis left at NONE (0) never rejects anything.
    if (jetId() < static_cast<unsigned char>(id.vsJet))
        return false;
    if (electronId() < static_cast<unsigned char>(id.vsE))
        return false;
    if (muonId() < static_cast<unsigned char>(id.vsMu))
        return false;
    return true;
}
