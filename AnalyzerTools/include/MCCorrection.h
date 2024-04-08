#ifndef MCCorrection_h
#define MCCorrection_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TString.h"

#include "correction.h"
using correction::CorrectionSet;

class MCCorrection {
public:
    MCCorrection();
    MCCorrection(const TString &era);
    ~MCCorrection();

    inline TString GetEra() const { return DataEra; }
    inline void SetEra(TString era) { DataEra = era; }
    
    float GetMuonIDSF(const TString &Muon_ID_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;

private:
    TString DataEra;
    unique_ptr<CorrectionSet> cset_muon;
};

#endif
