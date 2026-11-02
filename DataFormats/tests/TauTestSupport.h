#ifndef SKNANO_TAU_TEST_SUPPORT_H
#define SKNANO_TAU_TEST_SUPPORT_H

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "ColumnSource.h"
#include "TauView.h"

namespace sknano_test {

template <typename T>
class VectorColumn final : public SKNano::ColumnSource<T> {
public:
    VectorColumn() = default;
    explicit VectorColumn(std::vector<T> values) : values_(std::move(values)) {}

    T get(std::size_t idx) const override { return values_[idx]; }
    std::size_t size() const override { return values_.size(); }
    bool valid() const override { return true; }
    std::uint64_t epoch() const override { return 1; }
    SKNano::ContiguousView<T> snapshot() const override {
        return SKNano::ContiguousView<T>(this, values_.data(), values_.size(),
                                         epoch());
    }

    std::vector<T> &values() { return values_; }

private:
    std::vector<T> values_;
};

class BoolVectorColumn final : public SKNano::ColumnSource<bool> {
public:
    BoolVectorColumn() = default;
    explicit BoolVectorColumn(std::vector<std::uint8_t> values)
        : values_(std::move(values)) {}

    bool get(std::size_t idx) const override { return values_[idx] != 0; }
    std::size_t size() const override { return values_.size(); }
    bool valid() const override { return true; }
    std::uint64_t epoch() const override { return 1; }
    SKNano::ByteContiguousView snapshot() const override {
        return SKNano::ByteContiguousView(this, values_.data(), values_.size(),
                                          epoch());
    }

private:
    std::vector<std::uint8_t> values_;
};

struct TauFixture {
    VectorColumn<float> pt, eta, phi, mass, dxy, dz;
    VectorColumn<float> rawVsJet, rawVsE, rawVsMu;
    VectorColumn<short> charge, genPartIdx;
    VectorColumn<unsigned char> decayMode, genPartFlav, vsE, vsJet, vsMu;
    BoolVectorColumn newDM;
    TauSoA soa;

    TauView at(std::size_t index) const { return TauView(&soa, index); }
};

inline void BindRawMomentum(TauSoA &soa) {
    soa.populateMomentum = [target = &soa] {
        const std::size_t n = target->pt.size();
        target->correctedPt.resize(n);
        target->correctedMass.resize(n);
        target->tesPtUp.resize(n);
        target->tesPtDown.resize(n);
        target->tesMassUp.resize(n);
        target->tesMassDown.resize(n);
        target->esSource.assign(n, TauESSource::None);
        for (std::size_t i = 0; i < n; ++i) {
            target->correctedPt[i] = target->tesPtUp[i] =
                target->tesPtDown[i] = target->pt[i];
            target->correctedMass[i] = target->tesMassUp[i] =
                target->tesMassDown[i] = target->mass[i];
        }
        target->momentumReady = true;
    };
}

inline std::shared_ptr<TauFixture>
MakeTaus(const std::vector<float> &pts, const std::vector<float> &etas,
         const std::vector<unsigned char> &vsJet,
         const std::vector<unsigned char> &vsE,
         const std::vector<unsigned char> &vsMu,
         const std::vector<float> &dzs = {},
         const std::vector<unsigned char> &dms = {}) {
    auto f = std::make_shared<TauFixture>();
    const std::size_t n = pts.size();

    f->pt = VectorColumn<float>(pts);
    f->eta = VectorColumn<float>(etas);
    f->phi = VectorColumn<float>(std::vector<float>(n, 0.0f));
    f->mass = VectorColumn<float>(std::vector<float>(n, 1.777f));
    f->dxy = VectorColumn<float>(std::vector<float>(n, 0.0f));
    f->dz = VectorColumn<float>(dzs.empty() ? std::vector<float>(n, 0.0f)
                                            : dzs);
    f->charge = VectorColumn<short>(std::vector<short>(n, 1));
    f->genPartIdx = VectorColumn<short>(std::vector<short>(n, -1));
    f->decayMode = VectorColumn<unsigned char>(
        dms.empty() ? std::vector<unsigned char>(n, 0) : dms);
    f->genPartFlav =
        VectorColumn<unsigned char>(std::vector<unsigned char>(n, 5));
    f->vsJet = VectorColumn<unsigned char>(vsJet);
    f->vsE = VectorColumn<unsigned char>(vsE);
    f->vsMu = VectorColumn<unsigned char>(vsMu);
    f->newDM = BoolVectorColumn(std::vector<std::uint8_t>(n, 1));

    f->soa.pt.bind(&f->pt);
    f->soa.eta.bind(&f->eta);
    f->soa.phi.bind(&f->phi);
    f->soa.mass.bind(&f->mass);
    f->soa.dxy.bind(&f->dxy);
    f->soa.dz.bind(&f->dz);
    f->soa.charge.bind(&f->charge);
    f->soa.decayMode.bind(&f->decayMode);
    f->soa.genPartFlav.bind(&f->genPartFlav);
    f->soa.genPartIdx.bind(&f->genPartIdx);
    f->soa.idDecayModeNewDMs.bind(&f->newDM);
    f->soa.idDeepTau2018v2p5VSe.bind(&f->vsE);
    f->soa.idDeepTau2018v2p5VSjet.bind(&f->vsJet);
    f->soa.idDeepTau2018v2p5VSmu.bind(&f->vsMu);
    BindRawMomentum(f->soa);
    return f;
}

inline TauViewCollection MakeTauCollection(const std::shared_ptr<TauFixture> &f) {
    return TauViewCollection(std::shared_ptr<TauSoA>(f, &f->soa));
}

} 

#endif 
