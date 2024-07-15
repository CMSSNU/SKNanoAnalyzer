#ifndef JetTaggingParameter_h
#define JetTaggingParameter_h

#include <iostream>

using namespace std;

namespace JetTagging
{

    // Jet Tagging parameters
    enum class JetFlavTagger
    {
        DeepJet,
        ParticleNet,
        ParT
    };
    enum class JetFlavTaggerWP
    {
        Loose,
        Medium,
        Tight,
        VeryTight,
        SuperTight
    };
    inline TString GetTaggerCorrectionLibStr(JetFlavTagger tagger)
    {
        switch (tagger)
        {
        case JetFlavTagger::DeepJet:
            return "deepJet";
        case JetFlavTagger::ParticleNet:
            return "particleNet";
        case JetFlavTagger::ParT:
            return "robustParticleTransformer";
        default:
            cerr << "[JetTagging::GetTaggerCorrectionLibStr] No such tagger" << endl;
            exit(ENODATA);
        }
    };
    inline TString GetTaggerCorrectionWPStr(JetFlavTaggerWP wp)
    {
        switch (wp)
        {
        case JetFlavTaggerWP::Loose:
            return "L";
        case JetFlavTaggerWP::Medium:
            return "M";
        case JetFlavTaggerWP::Tight:
            return "T";
        case JetFlavTaggerWP::VeryTight:
            return "XT";
        case JetFlavTaggerWP::SuperTight:
            return "XXT";
        default:
        cerr << "[JetTagging::GetTaggerCorrectionLibStr] No such wp" << endl;
        exit(ENODATA);
        }
    };
}

#endif