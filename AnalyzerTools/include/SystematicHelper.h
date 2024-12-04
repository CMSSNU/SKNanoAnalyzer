#ifndef SystematicHelper_h
#define SystematicHelper_h
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "ROOT/RVec.hxx"
#include "Correction.h"

using namespace ROOT::VecOps;
using namespace std;
struct SYST
{
    std::string syst;
    std::string variation_target = "";
    std::string source = "total";
    bool evtLoopAgain = false;
    bool oneSided = false;
    bool hasDedicatedSample = false;
    std::string dedicatedSampleKey_up = "";
    std::string dedicatedSampleKey_down = "";
};

struct CORRELATION
{
    std::string type;
    std::string rep_name;
    RVec<SYST *> sources;
    unordered_set<std::string> child_syst_names;
};

struct Iter_obj
{
    std::string name;
    unordered_map<std::string, std::pair<std::string, Correction::variation>> variation_target_sources_variations;
};

class SystematicHelper
{
public:
    SystematicHelper(std::string yaml_path);
    ~SystematicHelper();
    SYST *findSystematic(std::string syst_name);
    /*class SystIterator
    {
    public:
        // Nested Iterator for range-based for compatibility
        class Iterator
        {
        public:
            Iterator(RVec<Iter_obj> &evtLoop, RVec<Iter_obj> &weight, size_t evtIdx, size_t weightIdx)
                : evtLoop_(evtLoop), weight_(weight), evtIdx_(evtIdx), weightIdx_(weightIdx) {}

            bool operator!=(const Iterator &other) const
            {
                return evtIdx_ != other.evtIdx_;
            }

            // Advance evtLoop_ and weight_ conditionally
            Iterator &operator++()
            {
                if (evtIdx_ < evtLoop_.size() && evtLoop_[evtIdx_].syst == "central" && weightIdx_ < weight_.size())
                {
                    ++weightIdx_;
                }
                ++evtIdx_;
                return *this;
            }

            // Dereference returns a pair of current items
            std::pair<Iter_obj *, Iter_obj *> operator*() const
            {
                Iter_obj *evtLoopCurrent = (evtIdx_ < evtLoop_.size()) ? &evtLoop_[evtIdx_] : nullptr;
                Iter_obj *weightCurrent = (weightIdx_ < weight_.size()) ? &weight_[weightIdx_] : nullptr;
                return {evtLoopCurrent, weightCurrent};
            }

        private:
            RVec<Iter_obj> &evtLoop_;
            RVec<Iter_obj> &weight_;
            size_t evtIdx_;
            size_t weightIdx_;
        };

        SystIterator(RVec<Iter_obj> &evtLoop, RVec<Iter_obj> &weight)
            : evtLoop_(evtLoop), weight_(weight) {}

        // Begin and End methods for range-based for loop
        Iterator begin() { return Iterator(evtLoop_, weight_, 0, 0); }
        Iterator end() { return Iterator(evtLoop_, weight_, evtLoop_.size(), weight_.size()); }

    private:
        RVec<Iter_obj> &evtLoop_;
        RVec<Iter_obj> &weight_;
    };

    SystIterator getIterator()
    {
        return SystIterator(systematics_evtLoopAgain, systematics_weight);
    }
*/

private:
    void checkBadSystematics();
    void make_Iter_obj_EvtLoopAgain();
    void make_Iter_obj_weight();
    bool IsDedicatedSample(TString sample);
    RVec<SYST> systematics;
    unordered_map<std::string, CORRELATION> correlations;
    RVec<Iter_obj> systematics_evtLoopAgain;
    RVec<Iter_obj> systematics_weight;
    RVec<Iter_obj> systematics_dedicatedSample;
};

#endif