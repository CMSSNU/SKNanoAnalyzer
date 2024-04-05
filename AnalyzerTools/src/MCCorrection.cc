#include "MCCorrection.h"

MCCorrection::MCCorrection() {}

MCCorrection::MCCorrection(const TString &era) {
    SetEra(era);
    string json_muon = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/MUO";
    if (era == "2022") {
        json_muon += "/2022_27Jun2023/muon_Z.json.gz";
    }
    else if (era == "2022EE") {
        json_muon += "/2022EE_27Jun2023/muon_Z.json.gz";
    }
    else {
        cerr << "[MCCorrection::MCCorrection] no correction file for era " << era << endl;
    }
    cout << "[MCCorrection::MCCorrection] using muon SF file: " << json_muon << endl;
    cset_muon = CorrectionSet::from_file(json_muon);
}

MCCorrection::~MCCorrection() {}

float MCCorrection::GetMuonIDSF(const TString &Muon_ID_SF_Key, const float eta, const float pt, const TString &sys) const {
    auto cset = cset_muon->at(string(Muon_ID_SF_Key));
    return cset->evaluate({fabs(eta), pt, sys.Data()});
}

