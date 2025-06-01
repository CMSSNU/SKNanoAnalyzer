#include "ParseEleIDVariables.h"

ParseEleIDVariables::ParseEleIDVariables() {}
ParseEleIDVariables::~ParseEleIDVariables() {}

void ParseEleIDVariables::initializeAnalyzer() {
    Events = new TTree("Events", "Events");
    Events->Branch("genWeight", &genWeight);
    Events->Branch("nElectrons", &nElectrons);
    Events->Branch("pt", pt, "pt[nElectrons]/F");
    Events->Branch("scEta", scEta, "scEta[nElectrons]/F");
    Events->Branch("sieie", sieie, "sieie[nElectrons]/F");
    Events->Branch("lepType", lepType, "lepType[nElectrons]/I");
    Events->Branch("deltaEtaInSC", deltaEtaInSC, "deltaEtaInSC[nElectrons]/F");
    Events->Branch("deltaPhiInSeed", deltaPhiInSeed, "deltaPhiInSC[nElectrons]/F");
    Events->Branch("hoe", hoe, "hoe[nElectrons]/F");
    Events->Branch("ecalPFClusterIso", ecalPFClusterIso, "ecalPFClusterIso[nElectrons]/F");
    Events->Branch("hcalPFClusterIso", hcalPFClusterIso, "hcalPFClusterIso[nElectrons]/F");
    Events->Branch("rho", rho, "rho[nElectrons]/F");
    Events->Branch("dr03TkSumPt", dr03TkSumPt, "dr03TkSumPt[nElectrons]/F");
    Events->Branch("isMVANoIsoWP90", isMVANoIsoWP90, "isMVANoIsoWP90[nElectrons]/O");
    Events->Branch("isMVANoIsoWPLoose", isMVANoIsoWPLoose, "isMVANoIsoWPLoose[nElectrons]/O");
    Events->Branch("isPOGMedium", isPOGMedium, "isPOGMedium[nElectrons]/O");
    Events->Branch("isPOGTight", isPOGTight, "isPOGTight[nElectrons]/O");
    Events->Branch("convVeto",  convVeto, "convVeto[nElectrons]/O");
    Events->Branch("lostHits", lostHits, "lostHits[nElectrons]/I");
    Events->Branch("dZ", dZ, "dZ[nElectrons]/F");
    Events->Branch("sip3d", sip3d, "sip3d[nElectrons]/F");
    Events->Branch("miniPFRelIso", miniPFRelIso, "miniPFRelIso[nElectrons]/F");
    Events->Branch("mvaNoIso", mvaNoIso, "mvaNoIso[nElectrons]/F");
    Events->Branch("nearestJetFlavour", nearestJetFlavour, "nearestJetFlavour[nElectrons]/I");
    Events->Branch("isTrigMatched", isTrigMatched, "isTrigMatched[nElectrons]/O");

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

void ParseEleIDVariables::executeEvent() {
    Event ev = GetEvent();
    const Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI);
    RVec<Jet> jets = GetAllJets();

    if (!PassMETFilter(METv, jets)) return;

    RVec<Electron> electrons = GetElectrons("", 15., 2.5);
    RVec<Muon> muons = GetMuons("POGTight", 25., 2.4);
    RVec<Gen> truth = GetAllGens();
    RVec<TrigObj> trigObjs = GetAllTrigObjs();

    // Require event to pass EMu trigger
    // and hard muon for the tag
    if (! ev.PassTrigger(EMuTriggers)) return;
    if (! (muons.size() == 1)) return;
    const auto &mu = muons.at(0);
    // Require muon to match the trigger object
    bool isMuonTrigMatched = false;
    for (const auto &trigObj : trigObjs) {
        if (trigObj.isMuon() && trigObj.DeltaR(mu) < 0.1) {
            if (trigObj.hasBit(0)) {
                isMuonTrigMatched = true;
                break;
            }
        }
    }
    if (!isMuonTrigMatched) return;
    if (!(electrons.size() > 0)) return;
    
    // Update branches
    genWeight = MCweight()*ev.GetTriggerLumi("Full")*GetL1PrefireWeight()*myCorr->GetPUWeight(ev.nTrueInt());
    nElectrons = electrons.size();
    for (int i = 0; i < nElectrons; i++) {
        const auto &el = electrons.at(i);
        pt[i] = el.Pt();
        scEta[i] = el.scEta();
        //try {
        lepType[i] = GetLeptonType(el, truth);
        //} catch (const std::exception& e) {
        //    lepType[i] = 0;
        //}
        sieie[i] = el.sieie();
        deltaEtaInSC[i] = el.deltaEtaInSC();
        deltaPhiInSeed[i] = el.deltaPhiInSeed();
        hoe[i] = el.hoe();
        ecalPFClusterIso[i] = el.ecalPFClusterIso();
        hcalPFClusterIso[i] = el.hcalPFClusterIso();
        rho[i] = el.rho();
        dr03TkSumPt[i] = el.dr03TkSumPt();
        isMVANoIsoWP90[i] = el.PassID("POGMVANoIsoWP90");
        isMVANoIsoWPLoose[i] = el.PassID("POGMVANoIsoWPLoose");
        isPOGMedium[i] = el.PassID("POGMedium");
        isPOGTight[i] = el.PassID("POGTight");
        convVeto[i] = el.ConvVeto();
        lostHits[i] = el.LostHits();
        sip3d[i] = el.SIP3D();
        dZ[i] = el.dZ();
        miniPFRelIso[i] = el.MiniPFRelIso();
        mvaNoIso[i] = el.MvaNoIso();

        // Find the nearest jet
        if (jets.size() > 0) {
            Jet nearest_jet = jets.at(0);
            for (const auto &j: jets) {
                if (j.DeltaR(el) > nearest_jet.DeltaR(el))
                    continue;
                nearest_jet = j;
            }
            nearestJetFlavour[i] = nearest_jet.hadronFlavour();
        } else {
            // No jets found, set a default value
            nearestJetFlavour[i] = -999; // or some other appropriate default value
        }
        
        // Check trigger object matching for CaloIdL_TrackIdL_IsoVL filter
        isTrigMatched[i] = false;
        for (const auto &trigObj : trigObjs) {
            // Check if trigger object is an electron and within deltaR < 0.3
            if (trigObj.isElectron() && trigObj.DeltaR(el) < 0.1) {
                // Check if it passes the CaloIdL_TrackIdL_IsoVL filter (bit 0)
                if (trigObj.hasBit(0)) {
                    isTrigMatched[i] = true;
                    break; // Found a match, no need to check other trigger objects
                }
            }
        }
    }
    Events->Fill();
}

void ParseEleIDVariables::WriteHist() {
    outfile->cd();
    Events->Write();
    outfile->Close();
}
