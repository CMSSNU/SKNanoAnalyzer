#ifndef MeasureJetTaggingEff_h
#define MeasureJetTaggingEff_h

#include "AnalyzerCore.h"

class MeasureJetTaggingEff : public AnalyzerCore
{

public:
    void initializeAnalyzer();

    void executeEvent();

    MeasureJetTaggingEff();
    ~MeasureJetTaggingEff();

protected:
    vector<float> vec_etabins;
    vector<float> vec_ptbins;

    float PtMax;

    int NEtaBin;
    int NPtBin;

    float *etabins;
    float *ptbins;

    RVec<JetTagging::JetFlavTagger> Taggers;
    RVec<JetTagging::JetFlavTaggerWP> WPs;
};

#endif