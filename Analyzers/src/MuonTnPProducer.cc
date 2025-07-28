#include "MuonTnPProducer.h"

MuonTnPProducer::MuonTnPProducer(){}
MuonTnPProducer::~MuonTnPProducer(){}

void MuonTnPProducer::WriteHist(){
    GetOutfile()->mkdir("muon");
    GetOutfile()->cd("muon");
    newtree->Write();
    GetOutfile()->cd();
}

void MuonTnPProducer::initializeAnalyzer(){
    GetOutfile()->cd();
    newtree=new TTree("Events","Events");

    newtree->Branch("run",&RunNumber);
    newtree->Branch("event",&EventNumber);
    newtree->Branch("lumi",&LumiBlock);
  
    if(!IsDATA){
        newtree->Branch("genWeight",&genWeight);
        newtree->Branch("PUweight",&PUweight);
        newtree->Branch("PUweight_up",&PUweight_up);
        newtree->Branch("PUweight_down",&PUweight_down);
        newtree->Branch("prefireweight",&prefireweight);
        newtree->Branch("prefireweight_up",&prefireweight_up);
        newtree->Branch("prefireweight_down",&prefireweight_down);
        newtree->Branch("zptweight",&zptweight);
        newtree->Branch("z0weight",&z0weight);
    }

    newtree->Branch("probe_isTracker",&probe_isTracker);
    newtree->Branch("probe_isGlobal",&probe_isGlobal);
    newtree->Branch("probe_isSA",&probe_isSA);
    newtree->Branch("probe_isSA_unique",&probe_isSA_unique);
    newtree->Branch("probe_isTight",&probe_isTight);
    newtree->Branch("probe_isMedium",&probe_isMedium);
    newtree->Branch("probe_isMedium2016a",&probe_isMedium2016a);
    newtree->Branch("probe_isLoose",&probe_isLoose);
    newtree->Branch("probe_TkIsoLoose",&probe_TkIsoLoose);
    newtree->Branch("probe_PFIsoTight",&probe_PFIsoTight);
    newtree->Branch("probe_IsoMu24",&probe_IsoMu24);
    newtree->Branch("probe_IsoMu27",&probe_IsoMu27);
    newtree->Branch("probe_Mu17Leg1",&probe_Mu17Leg1);
    newtree->Branch("probe_Mu8Leg2",&probe_Mu8Leg2);

    newtree->Branch("probe_pt",&probe_pt);
    newtree->Branch("probe_pt_cor",&probe_pt_cor);
    newtree->Branch("probe_eta",&probe_eta);
    newtree->Branch("probe_phi",&probe_phi);
    newtree->Branch("probe_q",&probe_q);
    newtree->Branch("probe_pfRelIso03",&probe_pfRelIso03);
    newtree->Branch("probe_pfRelIso04",&probe_pfRelIso04);
    newtree->Branch("probe_tkRelIso",&probe_tkRelIso);
    newtree->Branch("probe_miniPFRelIso",&probe_miniPFRelIso);
    newtree->Branch("probe_sip3d",&probe_sip3d);
    newtree->Branch("probe_dz",&probe_dz);
    newtree->Branch("probe_dxy",&probe_dxy);

    newtree->Branch("tag_IsoMu24",&tag_IsoMu24);
    newtree->Branch("tag_IsoMu27",&tag_IsoMu27);
    newtree->Branch("tag_isTight",&tag_isTight);
    newtree->Branch("tag_isMedium",&tag_isMedium);
    newtree->Branch("tag_isMedium2016a",&tag_isMedium2016a);
    newtree->Branch("tag_isLoose",&tag_isLoose);
    newtree->Branch("tag_TkIsoLoose",&tag_TkIsoLoose);
    newtree->Branch("tag_PFIsoTight",&tag_PFIsoTight);

    newtree->Branch("tag_pt",&tag_pt);
    newtree->Branch("tag_pt_cor",&tag_pt_cor);
    newtree->Branch("tag_eta",&tag_eta);
    newtree->Branch("tag_phi",&tag_phi);
    newtree->Branch("tag_q",&tag_q);
    newtree->Branch("tag_pfRelIso03",&tag_pfRelIso03);
    newtree->Branch("tag_pfRelIso04",&tag_pfRelIso04);
    newtree->Branch("tag_tkRelIso",&tag_tkRelIso);
    newtree->Branch("tag_miniPFRelIso",&tag_miniPFRelIso);
    newtree->Branch("tag_sip3d",&tag_sip3d);
    newtree->Branch("tag_dz",&tag_dz);
    newtree->Branch("tag_dxy",&tag_dxy);

    newtree->Branch("pair_probeMultiplicity",&pair_probeMultiplicity);
    newtree->Branch("pair_mass",&pair_mass);
    newtree->Branch("pair_mass_cor",&pair_mass_cor);
    newtree->Branch("pair_pt",&pair_pt);
    newtree->Branch("pair_pt_cor",&pair_pt_cor);
    newtree->Branch("pair_EMTF",&pair_EMTF);

    if(!IsDATA){
        newtree->Branch("probe_gen_matched",&probe_gen_matched);
        newtree->Branch("tag_gen_matched",&tag_gen_matched);
        newtree->Branch("pair_gen_matched",&pair_gen_matched);
        newtree->Branch("pair_gen_mass",&pair_gen_mass);
        newtree->Branch("probe_gen_pt",&probe_gen_pt);
        newtree->Branch("probe_gen_eta",&probe_gen_eta);
        newtree->Branch("probe_gen_phi",&probe_gen_phi);
        newtree->Branch("probe_gen_dR",&probe_gen_dR);
        newtree->Branch("probe_gen_reldpt",&probe_gen_reldpt);
        newtree->Branch("tag_gen_pt",&tag_gen_pt);
        newtree->Branch("tag_gen_eta",&tag_gen_eta);
        newtree->Branch("tag_gen_phi",&tag_gen_phi);
        newtree->Branch("tag_gen_dR",&tag_gen_dR);
        newtree->Branch("tag_gen_reldpt",&tag_gen_reldpt);
    }
    myCorr = new MyCorrection(DataEra, DataPeriod, MCSample, IsDATA);
}

void MuonTnPProducer::executeEvent(){
    Event ev = GetEvent();
    RVec<Jet> jets = GetAllJets();
    if (!PassNoiseFilter(jets, ev)) return;

    RVec<Muon> muons = GetAllMuons(); // Rochester Correction applied
    RVec<TrigObj> trigObjs = GetAllTrigObjs();
    map<Muon, Gen> genmatching;
    if (!IsDATA) {
        // Initialize gen particles
        RVec<LHE> lhes = GetAllLHEs();
        RVec<Gen> gens = GetAllGens();
        GetLHEParticles(lhes, lhe_p0, lhe_p1, lhe_l0, lhe_l1, lhe_j0);
        GetGenParticles(gens, gen_p0, gen_p1, gen_l0_bare, gen_l1_bare, 0);

        RVec<Gen> gens_to_match = {gen_l0_bare,gen_l1_bare};
        for(Gen &gen: gens_to_match) {
            RVec<Muon> cands;
            for(const auto &muon: muons) {
                //cout << "gen.DeltaR(muon) = " << gen.DeltaR(muon) << endl;
	              if(gen.DeltaR(muon)<0.2) cands.emplace_back(muon);
            }
            float min_dpt=1000.;
            Muon *matched = nullptr;
            for(Muon &cand:cands) {
	              float dpt=fabs((cand.Pt()-gen.Pt())/gen.Pt());
	              if (dpt<min_dpt && genmatching.find(cand)==genmatching.end()) {
	                  min_dpt=dpt;
	                  matched=&cand;
	              }
            }
            if(matched) genmatching[*matched] = gen;
        }
        //cout << "genmatching.size() = " << genmatching.size() << endl;
        genWeight= MCweight()*ev.GetTriggerLumi("Full");
        using myVar = MyCorrection::variation;
        PUweight= myCorr->GetPUWeight(ev.nTrueInt(), myVar::nom);
        PUweight_up= myCorr->GetPUWeight(ev.nTrueInt(), myVar::up);
        PUweight_down= myCorr->GetPUWeight(ev.nTrueInt(), myVar::down);
        prefireweight= GetL1PrefireWeight(myVar::nom);
        prefireweight_up= GetL1PrefireWeight(myVar::up);
        prefireweight_down= GetL1PrefireWeight(myVar::down);
        zptweight=1.0;
        z0weight=1.0;
    }
    
    for(Muon &tag:muons) {
        tag_IsoMu24=PassSLT(tag, trigObjs, 24.);
        tag_IsoMu27=PassSLT(tag, trigObjs, 27.);
        if(!(tag_IsoMu24||tag_IsoMu27)) continue;
            
        tag_isTight=tag.isPOGTightId();
        tag_isMedium=tag.isPOGMediumId();
        tag_isLoose=tag.isPOGLooseId();
        tag_TkIsoLoose=tag.PassID(Muon::MuonID::POG_TKISO_LOOSE);
        tag_PFIsoTight=tag.PassID(Muon::MuonID::POG_PFISO_TIGHT);
        tag_pt=tag.MiniAODPt();
        tag_pt_cor=tag.Pt();
        tag_eta=tag.Eta();
        tag_phi=tag.Phi();
        tag_q=tag.Charge();
        tag_pfRelIso03=tag.PfRelIso03();
        tag_pfRelIso04=tag.PfRelIso04();
        tag_tkRelIso=tag.TkRelIso();
        tag_miniPFRelIso=tag.MiniPFRelIso();
        tag_sip3d=tag.SIP3D();
        tag_dz=tag.dZ();
        tag_dxy=tag.dXY();

        // Find probe muons
        RVec<Muon> probe_muons;
        for(Muon &probe: muons) {
            if (&tag==&probe) continue;
            if ((tag+probe).M()<50) continue;
            if (tag.Charge()+probe.Charge()!=0) continue;
            probe_muons.emplace_back(probe);
        }
        for(Muon &probe: probe_muons) {
            probe_isTracker=probe.isTracker();
            probe_isGlobal=probe.isGlobal();
            probe_isSA=probe.isStandalone();
            probe_isSA_unique=probe_isSA;
            if (probe_isSA) {
	            for(const auto &dup:muons) {
	                if(&dup==&probe) continue;
	                if(!dup.isGlobal()) continue;
	                if(probe.DeltaR(dup)>0.2) continue;
	                if(fabs(probe.Pt()/dup.Pt()-1)>0.3) continue;
	                probe_isSA_unique=false;
	            }
	        } 
        
            probe_isTight=probe.isPOGTightId();
            probe_isMedium=probe.isPOGMediumId();
            probe_isLoose=probe.isPOGLooseId();
            probe_TkIsoLoose=probe.PassID(Muon::MuonID::POG_TKISO_LOOSE);
            probe_PFIsoTight=probe.PassID(Muon::MuonID::POG_PFISO_TIGHT);
            probe_IsoMu24=PassSLT(probe, trigObjs, 24.);
            probe_IsoMu27=PassSLT(probe, trigObjs, 27.);
            probe_Mu17Leg1=PassDLT(probe, trigObjs, 17.);
            probe_Mu8Leg2=PassDLT(probe, trigObjs, 8.);
            probe_pt=probe.MiniAODPt();
            probe_pt_cor=probe.Pt();
            probe_eta=probe.Eta();
            probe_phi=probe.Phi();
            probe_q=probe.Charge();
            probe_pfRelIso03=probe.PfRelIso03();
            probe_pfRelIso04=probe.PfRelIso04();
            probe_tkRelIso=probe.TkRelIso();
            probe_miniPFRelIso=probe.MiniPFRelIso();
            probe_sip3d=probe.SIP3D();
            probe_dz=probe.dZ();
            probe_dxy=probe.dXY();

            Particle pair=tag+probe;
            pair_probeMultiplicity=probe_muons.size();
            pair_mass=pair.M();
            pair_mass_cor=pair.M();
            pair_pt=pair.Pt();
            pair_pt_cor=pair.Pt();
            pair_EMTF=tag_eta*probe_eta > 0 && fabs(tag_eta) > 0.9 && fabs(probe_eta) > 0.9 && fabs(tag_phi-probe_phi) < 70/180.*3.141592;

            if (!IsDATA) {
                const Gen probe_gen = genmatching.find(probe) != genmatching.end() ? genmatching[probe] : Gen();
                const Gen tag_gen = genmatching.find(tag) != genmatching.end() ? genmatching[tag] : Gen();
                
                probe_gen_matched = false;
                probe_gen_pt = -999.;
                probe_gen_eta = -999.;
                probe_gen_phi = -999.;
                probe_gen_dR = -999.;
                probe_gen_reldpt = -999.;

                tag_gen_matched = false;
                tag_gen_pt = -999.;
                tag_gen_eta = -999.;
                tag_gen_phi = -999.;
                tag_gen_dR = -999.;
                tag_gen_reldpt = -999.;

                pair_gen_matched = false;
                pair_gen_mass = -999.;

	            if(!probe_gen.IsEmpty()) {
                    probe_gen_matched = true;
            	    probe_gen_pt=probe_gen.Pt();
	                probe_gen_eta=probe_gen.Eta();
	                probe_gen_phi=probe_gen.Phi();
	                probe_gen_dR=probe_gen.DeltaR(probe);
	                probe_gen_reldpt=(probe.Pt()-probe_gen.Pt())/probe_gen.Pt();
	            }
                if(!tag_gen.IsEmpty()) {
                    tag_gen_matched = true;
                    tag_gen_pt=tag_gen.Pt();
                    tag_gen_eta=tag_gen.Eta();
                    tag_gen_phi=tag_gen.Phi();
                    tag_gen_dR=tag_gen.DeltaR(tag);
                    tag_gen_reldpt=(tag.Pt()-tag_gen.Pt())/tag_gen.Pt();
                }
                if (!probe_gen.IsEmpty() && !tag_gen.IsEmpty()) {
                    pair_gen_matched=true;
                    pair_gen_mass=(probe_gen+tag_gen).M();
                } else {
                    pair_gen_matched=false;
                    pair_gen_mass=-999.;
                }
            }
            newtree->Fill();
        }
    }
}

void MuonTnPProducer::GetLHEParticles(const RVec<LHE> &lhes, LHE &p0, LHE &p1, LHE &l0, LHE &l1, LHE &j0) {
    p0 = LHE();
    p1 = LHE();
    l0 = LHE();
    l1 = LHE();
    j0 = LHE();
    if (!lhes.size()) return;
    for (const auto &lhe : lhes) {
        if (p0.IsEmpty() && lhe.Status() == -1 && lhe.Eta() > 0) p0 = lhe;
        if (p1.IsEmpty() && lhe.Status() == -1 && lhe.Eta() < 0) p1 = lhe;
        if (l0.IsEmpty() && (abs(lhe.PdgId()) == 11 || abs(lhe.PdgId()) == 13 || abs(lhe.PdgId()) == 15)) l0 = lhe;
        if ((!l0.IsEmpty()) && (abs(lhe.PdgId()) == 11 || abs(lhe.PdgId()) == 13 || abs(lhe.PdgId()) == 15)) l1 = lhe;
        if (lhe.Status() == 1) {
            if (abs(lhe.PdgId()) <= 6 || lhe.PdgId() == 21) {
                if (lhe.Pt() > j0.Pt()) j0 = lhe;
            }
        }
    }
    if (p0.PdgId() == 0 || p1.PdgId() == 0 || l0.PdgId() == 0 || l1.PdgId() == 0) {
        cerr << "[MuonTnPProducer::GetLHEParticles] something is wrong" << endl;
        exit(EXIT_FAILURE);
    }
    if (l0.Pt() < l1.Pt()) {
        LHE temp = l0;
        l0 = l1;
        l1 = temp;
    }
}

void MuonTnPProducer::GetGenParticles(const RVec<Gen> &gens, Gen &parton0, Gen &parton1, Gen &l0, Gen &l1, int mode) {
    // mode 0:bare 1:dressed01 2:dressed04 3:beforeFSR
    parton0 = Gen();
    parton1 = Gen();
    l0 = Gen();
    l1 = Gen();
    RVec<Gen> leptons;
    RVec<Gen> photons;
    for (const auto &gen : gens) {
        if (! gen.isPrompt()) continue;
        const unsigned int genpid = abs(gen.PID());
        if (gen.isHardProcess()) {
            if (genpid < 7 || genpid == 21 || genpid == 22) {
                if (parton0.IsEmpty()) parton0 = gen;
                else if (parton1.IsEmpty()) parton1 = gen;
            }
        }
        if (gen.Status() == 1) {
            if (genpid == 11 || genpid == 13) leptons.emplace_back(gen);
            else if (genpid == 22) photons.emplace_back(gen);
        }
    }
    const float max_dR = 0.4;
    for (const auto &lepton : leptons) {
        if (lepton.PID() != lhe_l0.PdgId()) continue;
        if (lepton.DeltaR(lhe_l0) > max_dR) continue;
        if (fabs(lepton.E() - lhe_l0.E()) < fabs(l0.E() - lhe_l0.E())) {
            l0 = lepton;
        }
    }
    if (l0.IsEmpty()) {
        for (const auto &lepton : leptons) {
            if (lepton.PID() != lhe_l0.PdgId()) continue;
            if (l0.IsEmpty() || lepton.DeltaR(lhe_l0) < l0.DeltaR(lhe_l0)) {
                l0 = lepton;
            }
        }
    }
    for (const auto &lepton : leptons) {
        if (lepton.PID() != lhe_l1.PdgId()) continue;
        if (lepton.DeltaR(lhe_l1) > max_dR) continue;
        if (fabs(lepton.E() - lhe_l1.E()) < fabs(l1.E() - lhe_l1.E())) {
            l1 = lepton;
        }
    }
    if (l1.IsEmpty()) {
        for (const auto &lepton : leptons) {
            if (lepton.PID() != lhe_l1.PdgId()) continue;
            if (l1.IsEmpty() || lepton.DeltaR(lhe_l1) < l1.DeltaR(lhe_l1)) {
                l1 = lepton;
            }
        }
    }
    if (l0.Pt() < l1.Pt()) {
        Gen tmp = l0;
        l0 = l1;
        l1 = tmp;
    }
    if (mode >= 3) {
        if (leptons.size() >= 4) {
            for (int i = 0; i < leptons.size(); i++) {
                if (leptons.at(i).Index() == l0.Index() || leptons.at(i).Index() == l1.Index()) continue;
                for (int j = i+1; j < leptons.size(); j++) {
                    if (leptons.at(j).Index() == l0.Index() || leptons.at(j).Index() == l1.Index()) continue;
                    if (!(leptons.at(i).PID() + leptons.at(j).PID() == 0)) continue;
                    RVec<int> history_i = TrackGenSelfHistory(leptons[i], gens);
                    RVec<int> history_j = TrackGenSelfHistory(leptons[j], gens);
                    if (history_i.at(1) == history_j.at(1)) {
                        photons.emplace_back(leptons[i]);
                        photons.emplace_back(leptons[j]);
                    }
                }
            }
        }
        for (const auto &photon: photons) {
            RVec<int> history = TrackGenSelfHistory(photon, gens);
            if (gens[history.at(1)].PID() == l0.PID()) l0 += photon;
            else if (gens[history.at(1)].PID() == l1.PID()) l1 += photon;
            else if (gens[history.at(1)].PID() == 23) { // for minnlo+photos
                if (photon.DeltaR(l0) < photon.DeltaR(l1)) l0 += photon;
                else l1 += photon;
            }
        }
    } else if (mode >= 1) {
        const float delr = mode == 1? 0.1: 0.4;
        for (const auto &photon: photons) {
            if (l0.DeltaR(photon) > delr && l1.DeltaR(photon) > delr) continue;
            if (l0.DeltaR(photon) < l1.DeltaR(photon)) l0 += photon;
            else l1 += photon;
        }
    }
}

bool MuonTnPProducer::PassSLT(const Muon &muon, const RVec<TrigObj> &trigObjs, const float pt_cut){
    for(const auto &trigObj : trigObjs){
        if (! trigObj.isMuon()) continue;
        if (! (trigObj.DeltaR(muon) < 0.3)) continue;
        if (trigObj.hasBit(3) && trigObj.Pt() > pt_cut) {
            return true;
        }         
    }
    return false;
}

bool MuonTnPProducer::PassDLT(const Muon &muon, const RVec<TrigObj> &trigObjs, const float pt_cut){
    for(const auto &trigObj : trigObjs){
        if (! trigObj.isMuon()) continue;
        if (! (trigObj.DeltaR(muon) < 0.3)) continue;
        if (trigObj.hasBit(0) && trigObj.hasBit(4) && trigObj.Pt() > pt_cut) {
            return true;
        }
    }
    return false;
}
