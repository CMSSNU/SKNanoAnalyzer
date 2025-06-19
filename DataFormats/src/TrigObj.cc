#include "TrigObj.h"

ClassImp(TrigObj)

TrigObj::TrigObj() : Particle() {
    j_id = -999;
    j_filterBits = 0;
    j_run = 2; // Default to Run 2
}

TrigObj::~TrigObj() {}

bool TrigObj::passTriggerFilter(int id, const TString& hltPath) const {
    // Check if the object ID matches
    if (j_id != id) return false;
    
    // Route to specific filter checks based on object type
    switch (id) {
        case 11: // Electron
            return passElectronFilter(hltPath);
        case 13: // Muon
            return passMuonFilter(hltPath);
        case 15: // Tau
            return passTauFilter(hltPath);
        case 1:  // Jet
            return passJetFilter(hltPath);
        case 3:  // HT
            return passHTFilter(hltPath);
        case 4:  // MHT
            return passMHTFilter(hltPath);
        case 22: // Photon
            return passPhotonFilter(hltPath);
        case 6:  // FatJet
            return passFatJetFilter(hltPath);
        case 1515: // BoostedTau (Run 3 only)
            return passBoostedTauFilter(hltPath);
        default:
            return false;
    }
}

bool TrigObj::passElectronFilter(const TString& hltPath) const {
    if (!isElectron()) return false;
    
    // Electron filter bits are the same for Run 2 and Run 3:
    // bit 0 = CaloIdL_TrackIdL_IsoVL
    // bit 1 = 1e (WPTight)
    // bit 2 = 1e (WPLoose)
    // bit 3 = OverlapFilter PFTau
    // bit 4 = 2e
    // bit 5 = 1e-1mu
    // bit 6 = 1e-1tau
    // bit 7 = 3e
    // bit 8 = 2e-1mu
    // bit 9 = 1e-2mu
    // bit 10 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
    // bit 11 = 1e (CaloIdVT_GsfTrkIdT)
    // bit 12 = 1e (PFJet)
    // bit 13 = 1e (Photon175_OR_Photon200)
    
    if (hltPath.Contains("Ele") || hltPath.Contains("DoubleEle") || hltPath.Contains("TripleEle")) {
        if (hltPath.Contains("CaloIdL") && hltPath.Contains("TrackIdL") && hltPath.Contains("IsoVL")) {
            return hasBit(0);
        }
        if (hltPath.Contains("WPTight")) {
            return hasBit(1);
        }
        if (hltPath.Contains("WPLoose")) {
            return hasBit(2);
        }
        if (hltPath.Contains("CaloIdVT") && hltPath.Contains("GsfTrkIdT")) {
            return hasBit(11);
        }
        if (hltPath.Contains("DoubleEle")) {
            return hasBit(4);
        }
        if (hltPath.Contains("TripleEle")) {
            return hasBit(7);
        }
        if (hltPath.Contains("Photon175") || hltPath.Contains("Photon200")) {
            return hasBit(13);
        }
        if (hltPath.Contains("PFJet")) {
            return hasBit(12);
        }
    }
    
    if (hltPath.Contains("EleMu") || hltPath.Contains("MuEle")) {
        return hasBit(5);
    }
    
    if (hltPath.Contains("EleTau") || hltPath.Contains("TauEle")) {
        return hasBit(6);
    }
    
    return false;
}

bool TrigObj::passMuonFilter(const TString& hltPath) const {
    if (!isMuon()) return false;
    
    if (j_run == 2) {
        // Run 2 Muon filter bits:
        // bit 0 = TrkIsoVVL
        // bit 1 = Iso
        // bit 2 = OverlapFilter PFTau
        // bit 3 = 1mu
        // bit 4 = 2mu
        // bit 5 = 1mu-1e
        // bit 6 = 1mu-1tau
        // bit 7 = 3mu
        // bit 8 = 2mu-1e
        // bit 9 = 1mu-2e
        // bit 10 = 1mu (Mu50)
        // bit 11 = 1mu (Mu100)
        
        if (hltPath.Contains("Mu") || hltPath.Contains("DoubleMu") || hltPath.Contains("TripleMu")) {
            if (hltPath.Contains("TrkIsoVVL")) {
                return hasBit(0);
            }
            if (hltPath.Contains("Iso") && !hltPath.Contains("TrkIsoVVL")) {
                return hasBit(1);
            }
            if (hltPath.Contains("DoubleMu")) {
                return hasBit(4);
            }
            if (hltPath.Contains("TripleMu")) {
                return hasBit(7);
            }
            if (hltPath.Contains("Mu50")) {
                return hasBit(10);
            }
            if (hltPath.Contains("Mu100")) {
                return hasBit(11);
            }
            if (hltPath.Contains("SingleMu") || hltPath.Contains("IsoMu")) {
                return hasBit(3);
            }
        }
        
        if (hltPath.Contains("MuEle") || hltPath.Contains("EleMu")) {
            return hasBit(5);
        }
        
        if (hltPath.Contains("MuTau") || hltPath.Contains("TauMu")) {
            return hasBit(6);
        }
    } else if (j_run == 3) {
        // Run 3 Muon filter bits (same as Run 2 plus bit 12):
        // bit 0-11 = same as Run 2
        // bit 12 = 1mu-1photon
        
        if (hltPath.Contains("Mu") || hltPath.Contains("DoubleMu") || hltPath.Contains("TripleMu")) {
            if (hltPath.Contains("TrkIsoVVL")) {
                return hasBit(0);
            }
            if (hltPath.Contains("Iso") && !hltPath.Contains("TrkIsoVVL")) {
                return hasBit(1);
            }
            if (hltPath.Contains("DoubleMu")) {
                return hasBit(4);
            }
            if (hltPath.Contains("TripleMu")) {
                return hasBit(7);
            }
            if (hltPath.Contains("Mu50")) {
                return hasBit(10);
            }
            if (hltPath.Contains("Mu100")) {
                return hasBit(11);
            }
            if (hltPath.Contains("SingleMu") || hltPath.Contains("IsoMu")) {
                return hasBit(3);
            }
        }
        
        if (hltPath.Contains("MuEle") || hltPath.Contains("EleMu")) {
            return hasBit(5);
        }
        
        if (hltPath.Contains("MuTau") || hltPath.Contains("TauMu")) {
            return hasBit(6);
        }
        
        if (hltPath.Contains("MuPhoton") || hltPath.Contains("PhotonMu")) {
            return hasBit(12);
        }
    }
    
    return false;
}

bool TrigObj::passTauFilter(const TString& hltPath) const {
    if (!isTau()) return false;
    
    if (j_run == 2) {
        // Run 2 Tau filter bits:
        // bit 0 = LooseChargedIso
        // bit 1 = MediumChargedIso
        // bit 2 = TightChargedIso
        // bit 3 = TightID OOSC photons
        // bit 4 = HPS
        // bit 5 = single-tau + tau+MET
        // bit 6 = di-tau
        // bit 7 = e-tau
        // bit 8 = mu-tau
        // bit 9 = VBF+di-tau
        
        if (hltPath.Contains("Tau")) {
            if (hltPath.Contains("LooseChargedIso")) {
                return hasBit(0);
            }
            if (hltPath.Contains("MediumChargedIso")) {
                return hasBit(1);
            }
            if (hltPath.Contains("TightChargedIso")) {
                return hasBit(2);
            }
            if (hltPath.Contains("HPS")) {
                return hasBit(4);
            }
            if (hltPath.Contains("DoubleTau") || hltPath.Contains("DiTau")) {
                return hasBit(6);
            }
            if (hltPath.Contains("VBF") && (hltPath.Contains("DoubleTau") || hltPath.Contains("DiTau"))) {
                return hasBit(9);
            }
            if (hltPath.Contains("MET") || hltPath.Contains("SingleTau")) {
                return hasBit(5);
            }
        }
        
        if (hltPath.Contains("EleTau") || hltPath.Contains("TauEle")) {
            return hasBit(7);
        }
        
        if (hltPath.Contains("MuTau") || hltPath.Contains("TauMu")) {
            return hasBit(8);
        }
    } else if (j_run == 3) {
        // Run 3 Tau filter bits (significantly expanded):
        // bit 0 = LooseChargedIso
        // bit 1 = MediumChargedIso
        // bit 2 = TightChargedIso
        // bit 3 = DeepTau
        // bit 4 = TightID OOSC photons
        // bit 5 = HPS
        // bit 6 = charged iso di-tau
        // bit 7 = deeptau di-tau
        // bit 8 = e-tau
        // bit 9 = mu-tau
        // bit 10 = single-tau/tau+MET
        // bit 11 = run 2 VBF+ditau
        // bit 12 = run 3 VBF+ditau
        // bit 13 = run 3 double PF jets + ditau
        // bit 14 = di-tau + PFJet
        // bit 15 = Displaced Tau
        // bit 16 = Monitoring
        // bit 17 = regional paths
        // bit 18 = L1 seeded paths
        // bit 19 = 1 prong tau paths
        
        if (hltPath.Contains("Tau")) {
            if (hltPath.Contains("LooseChargedIso")) {
                return hasBit(0);
            }
            if (hltPath.Contains("MediumChargedIso")) {
                return hasBit(1);
            }
            if (hltPath.Contains("TightChargedIso")) {
                return hasBit(2);
            }
            if (hltPath.Contains("DeepTau")) {
                return hasBit(3);
            }
            if (hltPath.Contains("HPS")) {
                return hasBit(5);
            }
            if (hltPath.Contains("DoubleTau") || hltPath.Contains("DiTau")) {
                if (hltPath.Contains("DeepTau")) {
                    return hasBit(7);
                } else {
                    return hasBit(6);
                }
            }
            if (hltPath.Contains("VBF")) {
                return hasBit(12); // Run 3 VBF
            }
            if (hltPath.Contains("PFJet") && (hltPath.Contains("DoubleTau") || hltPath.Contains("DiTau"))) {
                return hasBit(13) || hasBit(14);
            }
            if (hltPath.Contains("MET") || hltPath.Contains("SingleTau")) {
                return hasBit(10);
            }
            if (hltPath.Contains("Displaced")) {
                return hasBit(15);
            }
            if (hltPath.Contains("Monitoring")) {
                return hasBit(16);
            }
            if (hltPath.Contains("L1")) {
                return hasBit(18);
            }
            if (hltPath.Contains("1Prong")) {
                return hasBit(19);
            }
        }
        
        if (hltPath.Contains("EleTau") || hltPath.Contains("TauEle")) {
            return hasBit(8);
        }
        
        if (hltPath.Contains("MuTau") || hltPath.Contains("TauMu")) {
            return hasBit(9);
        }
    }
    
    return false;
}

bool TrigObj::passJetFilter(const TString& hltPath) const {
    if (!isJet()) return false;
    
    if (j_run == 2) {
        // Run 2 Jet filter bits (0-17):
        // bit 0 = VBF cross-cleaned from loose iso PFTau
        // bit 1 = hltBTagCaloCSVp087Triple
        // bit 2 = hltDoubleCentralJet90
        // bit 3 = hltDoublePFCentralJetLooseID90
        // bit 4 = hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
        // bit 5 = hltQuadCentralJet30
        // bit 6 = hltQuadPFCentralJetLooseID30
        // bit 7 = hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF
        // bit 8 = hltQuadCentralJet45
        // bit 9 = hltQuadPFCentralJetLooseID45
        // bit 10 = hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet
        // bit 11 = hltBTagCaloCSVp05Double or hltBTagCaloDeepCSVp17Double
        // bit 12 = hltPFCentralJetLooseIDQuad30
        // bit 13 = hlt1PFCentralJetLooseID75
        // bit 14 = hlt2PFCentralJetLooseID60
        // bit 15 = hlt3PFCentralJetLooseID45
        // bit 16 = hlt4PFCentralJetLooseID40
        // bit 17 = hltBTagPFCSVp070Triple or hltBTagPFDeepCSVp24Triple or hltBTagPFDeepCSV4p5Triple
        
        if (hltPath.Contains("VBF")) {
            return hasBit(0);
        }
        if (hltPath.Contains("BTag") && hltPath.Contains("Triple")) {
            if (hltPath.Contains("CSV")) {
                return hasBit(1) || hasBit(17);
            }
            if (hltPath.Contains("DeepCSV")) {
                return hasBit(17);
            }
        }
        if (hltPath.Contains("BTag") && hltPath.Contains("Double")) {
            return hasBit(11);
        }
        if (hltPath.Contains("DoubleJet90") || hltPath.Contains("DoubleCentralJet90")) {
            return hasBit(2) || hasBit(3);
        }
        if (hltPath.Contains("QuadJet") || hltPath.Contains("QuadCentralJet")) {
            if (hltPath.Contains("30")) {
                return hasBit(5) || hasBit(6) || hasBit(12);
            }
            if (hltPath.Contains("45")) {
                return hasBit(8) || hasBit(9);
            }
        }
        if (hltPath.Contains("PFJet75")) {
            return hasBit(13);
        }
        if (hltPath.Contains("PFJet60")) {
            return hasBit(14);
        }
        if (hltPath.Contains("PFJet45")) {
            return hasBit(15);
        }
        if (hltPath.Contains("PFJet40")) {
            return hasBit(16);
        }
    } else if (j_run == 3) {
        // Run 3 Jet filter bits (0-30) - significantly expanded
        if (hltPath.Contains("VBF")) {
            return hasBit(18) || hasBit(19); // VBF cross-cleaned filters
        }
        if (hltPath.Contains("BTag")) {
            if (hltPath.Contains("DeepJet")) {
                return hasBit(24) || hasBit(25); // DeepJet filters
            }
            if (hltPath.Contains("ParticleNet")) {
                return hasBit(26); // ParticleNet filters
            }
            if (hltPath.Contains("DeepCSV")) {
                return hasBit(11); // DeepCSV filters
            }
        }
        if (hltPath.Contains("QuadJet") || hltPath.Contains("QuadCentralJet")) {
            if (hltPath.Contains("30")) {
                return hasBit(5) || hasBit(12);
            }
        }
        if (hltPath.Contains("PFJet")) {
            if (hltPath.Contains("75")) {
                return hasBit(13);
            }
            if (hltPath.Contains("60")) {
                return hasBit(14) || hasBit(22) || hasBit(29);
            }
            if (hltPath.Contains("50")) {
                return hasBit(21) || hasBit(27);
            }
            if (hltPath.Contains("45")) {
                return hasBit(15);
            }
            if (hltPath.Contains("40")) {
                return hasBit(16);
            }
            if (hltPath.Contains("30")) {
                return hasBit(28);
            }
            if (hltPath.Contains("20")) {
                return hasBit(27);
            }
        }
        if (hltPath.Contains("PixelOnly")) {
            return hasBit(0) || hasBit(6) || hasBit(22) || hasBit(27);
        }
    }
    
    return false;
}

bool TrigObj::passHTFilter(const TString& hltPath) const {
    if (!isHT()) return false;
    
    if (j_run == 2) {
        // Run 2 HT filter bits (0-4):
        // bit 0 = hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
        // bit 1 = hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF
        // bit 2 = hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet
        // bit 3 = hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320
        // bit 4 = hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330
        
        if (hltPath.Contains("HT")) {
            if (hltPath.Contains("HT300") || hltPath.Contains("HT320")) {
                return hasBit(3) || hasBit(4);
            }
            if (hltPath.Contains("HT330")) {
                return hasBit(4);
            }
            if (hltPath.Contains("HT380")) {
                return hasBit(2);
            }
            if (hltPath.Contains("HT280")) {
                return hasBit(1) || hasBit(2);
            }
            // General HT triggers
            return hasBit(0) || hasBit(1) || hasBit(2);
        }
    } else if (j_run == 3) {
        // Run 3 HT filter bits (0-5) - added bit 5:
        // bit 0-4 = same as Run 2
        // bit 5 = hltPFHT280Jet30
        
        if (hltPath.Contains("HT")) {
            if (hltPath.Contains("HT300") || hltPath.Contains("HT320")) {
                return hasBit(3) || hasBit(4);
            }
            if (hltPath.Contains("HT330")) {
                return hasBit(4);
            }
            if (hltPath.Contains("HT380")) {
                return hasBit(2);
            }
            if (hltPath.Contains("HT280")) {
                return hasBit(1) || hasBit(2) || hasBit(5);
            }
            // General HT triggers
            return hasBit(0) || hasBit(1) || hasBit(2);
        }
    }
    
    return false;
}

bool TrigObj::passMHTFilter(const TString& hltPath) const {
    if (!isMHT()) return false;
    
    // MHT filter bits are the same for Run 2 and Run 3:
    // bit 0 = hltCaloQuadJet30HT300 OR hltCaloQuadJet30HT320
    // bit 1 = hltPFCentralJetsLooseIDQuad30HT300 OR hltPFCentralJetsLooseIDQuad30HT330
    
    if (hltPath.Contains("MHT") || hltPath.Contains("MET")) {
        if (hltPath.Contains("HT300") || hltPath.Contains("HT320")) {
            return hasBit(0) || hasBit(1);
        }
        if (hltPath.Contains("HT330")) {
            return hasBit(1);
        }
        if (hltPath.Contains("Calo")) {
            return hasBit(0);
        }
        if (hltPath.Contains("PF")) {
            return hasBit(1);
        }
    }
    
    return false;
}

bool TrigObj::passPhotonFilter(const TString& hltPath) const {
    if (!isPhoton()) return false;
    
    if (j_run == 3) {
        // Run 3 Photon filter bits (0-17):
        // bit 0 = hltEG33L1EG26HEFilter
        // bit 1 = hltEG50HEFilter
        // bit 2 = hltEG75HEFilter
        // bit 3 = hltEG90HEFilter
        // bit 4 = hltEG120HEFilter
        // bit 5 = hltEG150HEFilter
        // bit 6 = hltEG150HEFilter
        // bit 7 = hltEG200HEFilter
        // bit 8 = hltHtEcal800
        // bit 9 = hltEG110EBTightIDTightIsoTrackIsoFilter
        // bit 10 = hltEG120EBTightIDTightIsoTrackIsoFilter
        // bit 11 = 1mu-1photon
        // bit 12-17 = various photon ID and isolation filters
        
        if (hltPath.Contains("Photon") || hltPath.Contains("EG")) {
            if (hltPath.Contains("200")) {
                return hasBit(7);
            }
            if (hltPath.Contains("150")) {
                return hasBit(5) || hasBit(6);
            }
            if (hltPath.Contains("120")) {
                return hasBit(4) || hasBit(10);
            }
            if (hltPath.Contains("110")) {
                return hasBit(9);
            }
            if (hltPath.Contains("90")) {
                return hasBit(3);
            }
            if (hltPath.Contains("75")) {
                return hasBit(2);
            }
            if (hltPath.Contains("50")) {
                return hasBit(1);
            }
            if (hltPath.Contains("33")) {
                return hasBit(0);
            }
            if (hltPath.Contains("TightID") && hltPath.Contains("TightIso")) {
                return hasBit(9) || hasBit(10);
            }
        }
        
        if (hltPath.Contains("MuPhoton") || hltPath.Contains("PhotonMu")) {
            return hasBit(11);
        }
    }
    // Run 2 photon triggers would need different implementation if needed
    
    return false;
}

bool TrigObj::passFatJetFilter(const TString& hltPath) const {
    if (!isFatJet()) return false;
    
    if (j_run == 3) {
        // Run 3 FatJet filter bits (0-5):
        // bit 0 = (empty)
        // bit 1 = hltAK8SingleCaloJet200
        // bit 2 = (empty)
        // bit 3 = hltAK8SinglePFJets230SoftDropMass40BTagParticleNetBB0p35 OR similar
        // bit 4 = hltAK8DoublePFJetSDModMass30
        // bit 5 = hltAK8DoublePFJetSDModMass50
        
        if (hltPath.Contains("AK8")) {
            if (hltPath.Contains("CaloJet200")) {
                return hasBit(1);
            }
            if (hltPath.Contains("ParticleNet") && hltPath.Contains("BTag")) {
                return hasBit(3);
            }
            if (hltPath.Contains("DoublePFJet")) {
                if (hltPath.Contains("Mass30")) {
                    return hasBit(4);
                }
                if (hltPath.Contains("Mass50")) {
                    return hasBit(5);
                }
            }
        }
    }
    // Run 2 FatJet triggers would need different implementation if needed
    
    return false;
}

bool TrigObj::passBoostedTauFilter(const TString& hltPath) const {
    if (j_id != 1515) return false; // BoostedTau ID only exists in Run 3
    
    if (j_run == 3) {
        // Run 3 BoostedTau filter bits (0-1):
        // bit 0 = HLT_AK8PFJetX_SoftDropMass40_PFAK8ParticleNetTauTau0p30
        // bit 1 = hltAK8SinglePFJets230SoftDropMass40PNetTauTauTag0p03
        
        if (hltPath.Contains("AK8PFJet") && hltPath.Contains("ParticleNetTauTau")) {
            if (hltPath.Contains("SoftDropMass40")) {
                return hasBit(0) || hasBit(1);
            }
        }
    }
    
    return false;
}

//////////
