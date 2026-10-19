#ifndef SELECTEDFATJETVIEW_H
#define SELECTEDFATJETVIEW_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "FatJetView.h"
#include "SelectedJetView.h"

// AK8 counterpart of SelectedJetView: Pt()/M()/SDMass()/P4() read the lane
// the projection names instead of the raw NanoAOD values. Every other
// accessor, including the explicit lanes, is FatJetView's.
class SelectedFatJetView : public FatJetView {
public:
    SelectedFatJetView() = default;
    SelectedFatJetView(const FatJetSoA *storage, std::size_t index,
                       JetKinematicProjection projection)
        : FatJetView(storage, index), projection_(projection) {}

    float Pt() const {
        switch (projection_) {
        case JetKinematicProjection::Corrected: return CorrectedPt();
        case JetKinematicProjection::Nominal: return SmearedPtNominal();
        case JetKinematicProjection::JesUp: return JesPtUp();
        case JetKinematicProjection::JesDown: return JesPtDown();
        case JetKinematicProjection::JerUp: return SmearedPtUp();
        case JetKinematicProjection::JerDown: return SmearedPtDown();
        }
        throw std::logic_error("invalid fat jet kinematic projection");
    }

    float M() const {
        switch (projection_) {
        case JetKinematicProjection::Corrected: return CorrectedMass();
        case JetKinematicProjection::Nominal: return SmearedMassNominal();
        case JetKinematicProjection::JesUp: return JesMassUp();
        case JetKinematicProjection::JesDown: return JesMassDown();
        case JetKinematicProjection::JerUp: return SmearedMassUp();
        case JetKinematicProjection::JerDown: return SmearedMassDown();
        }
        throw std::logic_error("invalid fat jet kinematic projection");
    }

    // m_SD has no JEC-only lane: Corrected keeps the stored, unsmeared value.
    float SDMass() const {
        switch (projection_) {
        case JetKinematicProjection::Corrected: return FatJetView::SDMass();
        case JetKinematicProjection::Nominal: return SDMassNominal();
        case JetKinematicProjection::JesUp: return SDMassJesUp();
        case JetKinematicProjection::JesDown: return SDMassJesDown();
        case JetKinematicProjection::JerUp: return SDMassJerUp();
        case JetKinematicProjection::JerDown: return SDMassJerDown();
        }
        throw std::logic_error("invalid fat jet kinematic projection");
    }

    float Mass() const { return M(); }

    TLorentzVector P4() const {
        TLorentzVector result;
        result.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return result;
    }
    operator TLorentzVector() const { return P4(); }

private:
    JetKinematicProjection projection_ = JetKinematicProjection::Nominal;
};

class SelectedFatJetViewCollection {
public:
    class const_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = SelectedFatJetView;
        using difference_type = std::ptrdiff_t;
        using reference = SelectedFatJetView;

        const_iterator() = default;
        const_iterator(const SelectedFatJetViewCollection *owner,
                       std::size_t position)
            : owner_(owner), position_(position) {}
        reference operator*() const { return (*owner_)[position_]; }
        reference operator[](difference_type offset) const {
            return (*owner_)[position_ + offset];
        }
        const_iterator &operator++() { ++position_; return *this; }
        const_iterator operator++(int) { auto copy = *this; ++*this; return copy; }
        const_iterator &operator--() { --position_; return *this; }
        const_iterator &operator+=(difference_type n) { position_ += n; return *this; }
        const_iterator &operator-=(difference_type n) { position_ -= n; return *this; }
        friend const_iterator operator+(const_iterator it, difference_type n) { it += n; return it; }
        friend const_iterator operator-(const_iterator it, difference_type n) { it -= n; return it; }
        friend difference_type operator-(const const_iterator &a, const const_iterator &b) { return static_cast<difference_type>(a.position_) - static_cast<difference_type>(b.position_); }
        friend bool operator==(const const_iterator &a, const const_iterator &b) { return a.owner_ == b.owner_ && a.position_ == b.position_; }
        friend bool operator<(const const_iterator &a, const const_iterator &b) { return a.position_ < b.position_; }
    private:
        const SelectedFatJetViewCollection *owner_ = nullptr;
        std::size_t position_ = 0;
    };

    SelectedFatJetViewCollection() = default;
    SelectedFatJetViewCollection(std::shared_ptr<FatJetSoA> storage,
                                 std::vector<std::size_t> indices,
                                 JetKinematicProjection projection)
        : storage_(std::move(storage)), indices_(std::move(indices)),
          projection_(projection) {}

    std::size_t size() const {
        if (storage_)
            static_cast<void>(storage_->size());
        return indices_.size();
    }
    bool empty() const { return size() == 0; }
    SelectedFatJetView operator[](std::size_t position) const {
        if (position >= size())
            throw std::out_of_range("SelectedFatJetViewCollection index");
        return SelectedFatJetView(storage_.get(), indices_[position],
                                  projection_);
    }
    SelectedFatJetView at(std::size_t position) const { return (*this)[position]; }
    std::size_t rawIndex(std::size_t position) const { return indices_.at(position); }
    const std::vector<std::size_t> &indices() const { return indices_; }
    const std::shared_ptr<FatJetSoA> &storage() const { return storage_; }
    JetKinematicProjection projection() const { return projection_; }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size()); }

private:
    std::shared_ptr<FatJetSoA> storage_;
    std::vector<std::size_t> indices_;
    JetKinematicProjection projection_ = JetKinematicProjection::Nominal;
};

#endif
