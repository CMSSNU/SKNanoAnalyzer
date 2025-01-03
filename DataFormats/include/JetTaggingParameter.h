#ifndef JetTaggingParameter_h
#define JetTaggingParameter_h

#include <iostream>
#include <TString.h>

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
    enum class JetTaggingSFMethod
    {
        comb,
        mujets,
        shape,
        wp
    };
    //string version of JetTaggingSFMethod
    inline TString GetJetTaggingSFMethodStr(JetTaggingSFMethod method)
    {
        switch (method)
        {
        case JetTaggingSFMethod::comb:
            return "comb";
        case JetTaggingSFMethod::mujets:
            return "mujets";
        case JetTaggingSFMethod::shape:
            return "shape";
        case JetTaggingSFMethod::wp:
            return "wp";
        default:
            cerr << "[JetTagging::GetJetTaggingSFMethodStr] No such method" << endl;
            exit(ENODATA);
        }
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