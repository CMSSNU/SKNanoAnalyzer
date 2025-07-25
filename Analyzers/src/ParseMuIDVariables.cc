#include "ParseMuIDVariables.h"

ParseMuIDVariables::ParseMuIDVariables() {}
ParseMuIDVariables::~ParseMuIDVariables() {}

void ParseMuIDVariables::initializeAnalyzer() {
    Events = new TTree("Events", "Events");
    Events->Branch("genWeight", &genWeight);
    Events->Branch("nMuons", &nMuons);
    Events->Branch("pt", pt, "pt[nMuons]/F");
    Events->Branch("eta", eta, "eta[nMuons]/F");
    Events->Branch("lepType", lepType, "lepType[nMuons]/I");
    Events->Branch("nearestJetFlavour", nearestJetFlavour, "nearestJetFlavour[nMuons]/I");
    Events->Branch("isPOGMediumId", isPOGMediumId, "isPOGMediumId[nMuons]/F");
    Events->Branch("dZ", dZ, "dZ[nMuons]/F");
    Events->Branch("sip3d", sip3d, "sip3d[nMuons]/F");
    Events->Branch("tkRelIso", tkRelIso, "tkRelIso[nMuons]/F");
    Events->Branch("miniPFRelIso", miniPFRelIso, "miniPFRelIso[nMuons]/F");
    Events->Branch("isTrigMatched", isTrigMatched, "isTrigMatched[nMuons]/O");

    if (DataEra == "2016preVFP") {
        EMuTriggers = {
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL",
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL"
        };
    } else if (DataEra == "2016postVFP") {
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
    } else if (DataEra == "2017") {
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
    } else if (DataEra == "2018") {
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
    } else if (Run == 3) {
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
    } else {
        cerr << "[ParseEleIDVariables::initializeAnalyzer] " << DataEra << " is not implemented" << endl;
        exit(EXIT_FAILURE);
    }

    myCorr = new MyCorrection(DataEra, MCSample, IsDATA);
}

void ParseMuIDVariables::executeEvent() {
    Event ev = GetEvent();
    RVec<Jet> jets = GetAllJets();
    if (!PassNoiseFilter(jets, ev)) return;

    RVec<Electron> electrons = GetElectrons("POGTight", 25., 2.5);
    RVec<Muon> muons = GetMuons("", 10., 2.4);
    RVec<Gen> truth = GetAllGens();
    RVec<TrigObj> trigObjs = GetAllTrigObjs();

    // Require event to pass EMu trigger
    // and hard electron for the tag
    if (! ev.PassTrigger(EMuTriggers)) return;
    if (! (electrons.size() == 1)) return;
    const auto &el = electrons.at(0);
    // Require electron to match the trigger object
    bool isElectronTrigMatched = false;
    for (const auto &trigObj : trigObjs) {
        if (trigObj.isElectron() && trigObj.DeltaR(el) < 0.1) {
            if (trigObj.hasBit(0)) {
                isElectronTrigMatched = true;
                break;
            }
        }
    }
    if (!isElectronTrigMatched) return;
    if (!(muons.size() > 0)) return;
    
    // Update branches
    genWeight = MCweight()*ev.GetTriggerLumi("Full")*GetL1PrefireWeight()*myCorr->GetPUWeight(ev.nTrueInt());
    nMuons = muons.size();
    for (int i = 0; i < nMuons; i++) {
        const auto &mu = muons.at(i);
        pt[i] = mu.Pt();
        eta[i] = mu.Eta();
        isPOGMediumId[i] = mu.PassID("POGMedium");
        dZ[i] = mu.dZ();
        sip3d[i] = mu.SIP3D();
        tkRelIso[i] = mu.TkRelIso();
        miniPFRelIso[i] = mu.MiniPFRelIso();
        lepType[i] = GetLeptonType(mu, truth);

        // Use jetIdx for efficient jet matching
        nearestJetFlavour[i] = -1; // Default: no jet match
        
        short jetIdx = mu.JetIdx();
        if (jetIdx >= 0) {
            // Find the jet with matching original index
            const Jet* matchedJet = nullptr;
            for (const auto &jet : jets) {
                if (jet.OriginalIndex() == jetIdx) {
                    matchedJet = &jet;
                    break;
                }
            }
            
            if (matchedJet) {
                nearestJetFlavour[i] = matchedJet->genJetIdx() < 0 ? -1 : matchedJet->hadronFlavour();
            }
        }

        isTrigMatched[i] = false;
        for (const auto &trigObj : trigObjs) {
            if (trigObj.isMuon() && trigObj.DeltaR(mu) < 0.1) {
                if (trigObj.hasBit(0)) {
                    isTrigMatched[i] = true;
                    break;
                }
            }
        }
    }
    Events->Fill();
}

void ParseMuIDVariables::WriteHist() {
    TFile* outfile = GetOutfile();
    Events->Write();
    outfile->Close();
}
