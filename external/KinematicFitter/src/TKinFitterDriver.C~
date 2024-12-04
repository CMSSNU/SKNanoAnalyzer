#include "TKinFitterDriver.h"

TKinFitterDriver::TKinFitterDriver(){

}


TKinFitterDriver::TKinFitterDriver(int DataYear_, Double_t top_mass_){
  //if(gSystem->Load("librat.so")){  
  //  cout << "TKinFitterDriver::TKinFitterDriver,   librat.so is not loaded" << endl;
  //  exit(EXIT_FAILURE);   
  //}

  top_mass = top_mass_;

  IsFitExtraJets = true;

  DataYear = DataYear_;

  fitter = new TKinFitter("fitter","fitter");

  error_hadronic_top_b_jet.ResizeTo(1,1); 
  error_leptonic_top_b_jet.ResizeTo(1,1);
  error_hadronic_w_ch_jet1.ResizeTo(1,1);
  error_hadronic_w_ch_jet2.ResizeTo(1,1);
  error_lepton.ResizeTo(1,1);
  error_neutrino_pxpypz.ResizeTo(3,3);
  error_uncl_pxpy.ResizeTo(2,2);

  error_hadronic_top_b_jet.Zero(); 
  error_leptonic_top_b_jet.Zero();
  error_hadronic_w_ch_jet1.Zero();
  error_hadronic_w_ch_jet2.Zero();
  error_lepton.Zero();
  error_neutrino_pxpypz.Zero();
  error_uncl_pxpy.Zero();

  ts_correction = new TSCorrection(DataYear);
  //ts_correction->ReadFittedError("fit_error_pythia.txt");
  //ts_correction->ReadFittedMean("fit_mean_pythia.txt");

  fit_hadronic_top_b_jet = new TFitParticlePt();
  fit_leptonic_top_b_jet = new TFitParticlePt();
  fit_hadronic_w_ch_jet1 = new TFitParticlePt();
  fit_hadronic_w_ch_jet2 = new TFitParticlePt();
  fit_lepton = new TFitParticlePt();
  fit_neutrino_pxpypz =  new TFitParticleMCCart();
  fit_uncl_pxpy       =  new TFitParticleMCCart();
  //fit_neutrino_pz =  new TFitParticlePz();

  //constrain_hadronic_top_M = new TFitConstraintM("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, top_mass);
  constrain_hadronic_top_MGaus = new TFitConstraintMGaus("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, (Double_t)top_mass, 1.5);
  //constrain_leptonic_top_M = new TFitConstraintM("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, top_mass);
  constrain_leptonic_top_MGaus = new TFitConstraintMGaus("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, (Double_t)top_mass, 1.5);
  //constrain_leptonic_W_M = new TFitConstraintM("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4);
  constrain_leptonic_W_MGaus = new TFitConstraintMGaus("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4, 2.085);
  //constrain_hadronic_W_M = new TFitConstraintM("hadronic_w_mass_constraint", "hadronic_w_mass_constraint", 0, 0, 80.4);
  //constrain_hadronic_W_MGaus = new TFitConstraintMGaus("hadronic_w_mass_constraint", "hadronic_w_mass_constraint", 0, 0, 80.4, 2.085);
  //cout <<"TKinFitterDriver::TKinFitterDriver : initialized" << endl;

  //
  constrain_pX = new TFitConstraintEp("pX","pX",TFitConstraintEp::component::pX,0.);
  constrain_pY = new TFitConstraintEp("pY","pY",TFitConstraintEp::component::pY,0.);
  constrain_w_ch_pX = new TFitConstraintEp("w_ch_pX","w_ch_pX",TFitConstraintEp::component::pX,0.);
  constrain_w_ch_pY = new TFitConstraintEp("w_ch_pY","w_ch_pY",TFitConstraintEp::component::pY,0.);
}


TKinFitterDriver::~TKinFitterDriver(){
  delete fitter;

  delete fit_hadronic_top_b_jet;
  delete fit_leptonic_top_b_jet;
  delete fit_hadronic_w_ch_jet1;
  delete fit_hadronic_w_ch_jet2;
  delete fit_lepton;

  if(IsFitExtraJets){
    for(auto* fit_par:fit_extra_jets){
      delete fit_par;
    }
  }
  delete fit_neutrino_pxpypz;
  delete fit_uncl_pxpy;
  //delete fit_neutrino_pz;

  //delete constrain_hadronic_top_M;
  delete constrain_hadronic_top_MGaus;
  //delete constrain_leptonic_top_M;
  delete constrain_leptonic_top_MGaus;
  //delete constrain_leptonic_W_M;
  delete constrain_leptonic_W_MGaus;
  //delete constrain_hadronic_W_M;
  //delete constrain_hadronic_W_MGaus;

  delete constrain_pX;
  delete constrain_pY;
  delete constrain_w_ch_pX;
  delete constrain_w_ch_pY;

  delete ts_correction;
}


void TKinFitterDriver::SetDataYear(int i){
  DataYear = i;
}


void TKinFitterDriver::SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                                     std::vector<bool> btag_vector_,
                                     TLorentzVector lepton_,
                                     TLorentzVector met_){

  //cout << "jet vector size " << jet_vector_.size() << endl; 
  //cout << "btag vector size " << btag_vector_.size() << endl; 
  jet_vector.clear();
  for(UInt_t i=0; i<jet_vector_.size(); i++){
    jet_vector.push_back(jet_vector_.at(i));
  }
  btag_vector.clear();
  for(UInt_t i=0; i<btag_vector_.size(); i++){
    btag_vector.push_back(btag_vector_.at(i));
  }
  njets = jet_vector.size();
  nbtags = std::accumulate(btag_vector.begin(), btag_vector.end(),0);

  permutation_vector.clear();
  for(int i(0); i<njets; i++){
    if(i==0) permutation_vector.push_back( HADRONIC_TOP_B );
    else if(i==1) permutation_vector.push_back( LEPTONIC_TOP_B );
    else if(i==2) permutation_vector.push_back( W_CH_UP_TYPE );
    else if(i==3) permutation_vector.push_back( W_CH_DOWN_TYPE );
    else permutation_vector.push_back( NONE );
    
  } 
  METv = met_;
  this->SetLepton(lepton_);
  
  //cout <<"TKinFitterDriver::SetAllObjects : " << endl;
}


void TKinFitterDriver::SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                                     std::vector<double> btag_csv_vector_,
                                     double btag_cut_,
                                     TLorentzVector lepton_,
                                     TLorentzVector met_){
  btag_csv_vector = btag_csv_vector_;
  std::vector<bool> btag_vector_;
  for(UInt_t i=0; i<btag_csv_vector.size(); i++){
    if(btag_csv_vector.at(i)>btag_cut_){
      btag_vector_.push_back(true);
    }
    else{
      btag_vector_.push_back(false);
    }
  }

  this->SetAllObjects(jet_vector_,
                      btag_vector_,
                      lepton_,
                      met_
                     );

}


void TKinFitterDriver::SetJetPtResolution(std::vector<double> jetPtResolution_){
  jet_pt_resolution_vector.clear();
  for(auto& x : jetPtResolution_){
    jet_pt_resolution_vector.push_back(x);
  }
}


void TKinFitterDriver::SetJetBregCorr(std::vector<double> jetPtCorrection_){
  jet_pt_bRegCorr_vector.clear();
  for(auto& x : jetPtCorrection_){
    jet_pt_bRegCorr_vector.push_back(x);
  }
}


void TKinFitterDriver::SetJetBregRes(std::vector<double> jetPtResolution_){
  jet_pt_bRegRes_vector.clear();
  for(auto& x : jetPtResolution_){
    jet_pt_bRegRes_vector.push_back(x);
  }
}


void TKinFitterDriver::SetJetCregCorr(std::vector<double> jetPtCorrection_){
  jet_pt_cRegCorr_vector.clear();
  for(auto& x : jetPtCorrection_){
    jet_pt_cRegCorr_vector.push_back(x);
  }
}


void TKinFitterDriver::SetJetCregRes(std::vector<double> jetPtResolution_){
  jet_pt_cRegRes_vector.clear();
  for(auto& x : jetPtResolution_){
    jet_pt_cRegRes_vector.push_back(x);
  }
}


void TKinFitterDriver::SetHadronicTopBJets(TLorentzVector jet_){
  hadronic_top_b_jet = jet_;
  double Pt = hadronic_top_b_jet.Pt();
  double Eta = hadronic_top_b_jet.Eta();
  double Phi = hadronic_top_b_jet.Phi();
  this->SetJetError(&error_hadronic_top_b_jet, Pt, Eta, Phi, "b");
  corr_hadronic_top_b_jet = ts_correction->GetCorrectedJet("b",hadronic_top_b_jet);
  //fit_hadronic_top_b_jet->~TFitParticlePt();
  fit_hadronic_top_b_jet->reset();
  new(fit_hadronic_top_b_jet) TFitParticlePt("hadronic_top_b_jet",
                                                    "hadronic_top_b_jet",
                                                    &corr_hadronic_top_b_jet,
                                                    &error_hadronic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetHadronicTopBJets : " << endl;

}


void TKinFitterDriver::SetLeptonicTopBJets(TLorentzVector jet_){
  leptonic_top_b_jet=jet_;
  double Pt = leptonic_top_b_jet.Pt();
  double Eta = leptonic_top_b_jet.Eta();
  double Phi = leptonic_top_b_jet.Phi();
  this->SetJetError(&error_leptonic_top_b_jet, Pt, Eta, Phi, "b");
  //XXX when we vary only leptonic top b jet pt
  //corr_leptonic_top_b_jet = ts_correction->GetCorrectedJet("b",leptonic_top_b_jet) + lepton + neutrino_pxpy + neutrino_pz;
  //XXX when we vary only leptonic top b jet pt and neutrino_pxpy
  //corr_leptonic_top_b_jet = ts_correction->GetCorrectedJet("b",leptonic_top_b_jet) + lepton + neutrino_pz;
  corr_leptonic_top_b_jet = ts_correction->GetCorrectedJet("b",leptonic_top_b_jet);
  //fit_leptonic_top_b_jet->~TFitParticlePt();
  fit_leptonic_top_b_jet->reset();
  new(fit_leptonic_top_b_jet) TFitParticlePt("leptonic_top_b_jet",
                                                    "leptonic_top_b_jet",
                                                    &corr_leptonic_top_b_jet,
                                                    &error_leptonic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetLeptonicTopBJets : " << endl;
}



void TKinFitterDriver::SetWCHUpTypeJets(TLorentzVector jet_){
  hadronic_w_ch_jet1=jet_;
  double Pt = hadronic_w_ch_jet1.Pt();
  double Eta = hadronic_w_ch_jet1.Eta();
  double Phi = hadronic_w_ch_jet1.Phi();
  this->SetJetError(&error_hadronic_w_ch_jet1, Pt, Eta, Phi, "udsc");
  corr_hadronic_w_ch_jet1 = ts_correction->GetCorrectedJet("udsc",hadronic_w_ch_jet1);
  //fit_hadronic_w_ch_jet1->~TFitParticlePt();
  fit_hadronic_w_ch_jet1->reset();
  new(fit_hadronic_w_ch_jet1) TFitParticlePt("hadronic_w_ch_jet1",
                                                    "hadronic_w_ch_jet1",
                                                    &corr_hadronic_w_ch_jet1,
                                                    &error_hadronic_w_ch_jet1
                                                   );
  //cout <<"TKinFitterDriver::SetWCHUpTypeJets : " << endl;
}


void TKinFitterDriver::SetWCHDownTypeJets(TLorentzVector jet_){
  hadronic_w_ch_jet2=jet_;
  double Pt = hadronic_w_ch_jet2.Pt();
  double Eta = hadronic_w_ch_jet2.Eta();
  double Phi = hadronic_w_ch_jet2.Phi();
  TString flav =  nbtags>2 ? "b":"udsc";
  this->SetJetError(&error_hadronic_w_ch_jet2, Pt, Eta, Phi, flav);
  corr_hadronic_w_ch_jet2 = ts_correction->GetCorrectedJet(flav, hadronic_w_ch_jet2);
  //fit_hadronic_w_ch_jet2->~TFitParticlePt();
  fit_hadronic_w_ch_jet2->reset();
  new(fit_hadronic_w_ch_jet2) TFitParticlePt("hadronic_w_ch_jet2",
                                                    "hadronic_w_ch_jet2",
                                                    &corr_hadronic_w_ch_jet2,
                                                    &error_hadronic_w_ch_jet2
                                                   );
  //cout << "TKinFitterDriver::SetWCHDownTypeJets : " << endl;
}

void TKinFitterDriver::SetHadronicTopBJets(TLorentzVector jet_, double resolution_){
  hadronic_top_b_jet = jet_;
  double Pt = hadronic_top_b_jet.Pt();
  error_hadronic_top_b_jet(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_top_b_jet = hadronic_top_b_jet;
  fit_hadronic_top_b_jet->~TFitParticlePt();
  new(fit_hadronic_top_b_jet) TFitParticlePt("hadronic_top_b_jet",
                                                    "hadronic_top_b_jet",
                                                    &corr_hadronic_top_b_jet,
                                                    &error_hadronic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetHadronicTopBJets : " << endl;

}


void TKinFitterDriver::SetHadronicTopBJets(unsigned jet_idx){
  hadronic_top_b_jet = jet_vector.at(jet_idx);
  double corr_       = jet_pt_bRegCorr_vector.at(jet_idx);
  double resolution_ = jet_pt_bRegRes_vector.at(jet_idx);
  double Pt  = corr_ * hadronic_top_b_jet.Pt();
  // + 10% of jet pT to jet resolution
  //resolution_ += 0.1 * Pt;
  double Eta = hadronic_top_b_jet.Eta();
  double Phi = hadronic_top_b_jet.Phi();
  double M   = corr_ * hadronic_top_b_jet.M();
  hadronic_top_b_jet.SetPtEtaPhiM(Pt, Eta, Phi, M);
  error_hadronic_top_b_jet(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_top_b_jet = hadronic_top_b_jet;
  fit_hadronic_top_b_jet->~TFitParticlePt();
  new(fit_hadronic_top_b_jet) TFitParticlePt("hadronic_top_b_jet",
                                                    "hadronic_top_b_jet",
                                                    &corr_hadronic_top_b_jet,
                                                    &error_hadronic_top_b_jet
                                                   );

  //propagate to MET
  corr_METv = corr_METv - ( hadronic_top_b_jet  - jet_vector.at(jet_idx) );
  //remove pz component
  corr_METv.SetPtEtaPhiM(corr_METv.Pt(), 0., corr_METv.Phi(), 0.);

  //cout <<"TKinFitterDriver::SetHadronicTopBJets : " << endl;

}


void TKinFitterDriver::SetLeptonicTopBJets(TLorentzVector jet_, double resolution_){
  leptonic_top_b_jet=jet_;
  double Pt = leptonic_top_b_jet.Pt();
  error_leptonic_top_b_jet(0,0) = resolution_*resolution_*Pt*Pt;
  //XXX when we vary only leptonic top b jet pt
  //corr_leptonic_top_b_jet = leptonic_top_b_jet + lepton + neutrino_pxpy + neutrino_pz;
  //XXX when we vary only leptonic top b jet pt and neutrino_pxpy
  //corr_leptonic_top_b_jet = leptonic_top_b_jet + lepton + neutrino_pz;
  corr_leptonic_top_b_jet = leptonic_top_b_jet;

  fit_leptonic_top_b_jet->~TFitParticlePt();
  new(fit_leptonic_top_b_jet) TFitParticlePt("leptonic_top_b_jet",
                                                    "leptonic_top_b_jet",
                                                    &corr_leptonic_top_b_jet,
                                                    &error_leptonic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetLeptonicTopBJets : " << endl;
}


void TKinFitterDriver::SetLeptonicTopBJets(unsigned jet_idx){
  leptonic_top_b_jet = jet_vector.at(jet_idx);
  double corr_       = jet_pt_bRegCorr_vector.at(jet_idx);
  double resolution_ = jet_pt_bRegRes_vector.at(jet_idx);
  double Pt  = corr_ * leptonic_top_b_jet.Pt();
  // + 10% of jet pT to jet resolution
  //resolution_ += 0.1 * Pt;
  double Eta = leptonic_top_b_jet.Eta();
  double Phi = leptonic_top_b_jet.Phi();
  double M   = corr_ * leptonic_top_b_jet.M();
  leptonic_top_b_jet.SetPtEtaPhiM(Pt, Eta, Phi, M);
  error_leptonic_top_b_jet(0,0) = resolution_*resolution_*Pt*Pt;
  corr_leptonic_top_b_jet = leptonic_top_b_jet;
  fit_leptonic_top_b_jet->~TFitParticlePt();
  new(fit_leptonic_top_b_jet) TFitParticlePt("leptonic_top_b_jet",
                                                    "leptonic_top_b_jet",
                                                    &corr_leptonic_top_b_jet,
                                                    &error_leptonic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetLeptonicTopBJets : " << endl;
  //propagate to MET
  corr_METv = corr_METv - ( leptonic_top_b_jet  - jet_vector.at(jet_idx) );
  //remove pz component
  corr_METv.SetPtEtaPhiM(corr_METv.Pt(), 0., corr_METv.Phi(), 0.);

}


void TKinFitterDriver::SetWCHUpTypeJets(TLorentzVector jet_, double resolution_){
  hadronic_w_ch_jet1=jet_;
  double Pt = hadronic_w_ch_jet1.Pt();
  error_hadronic_w_ch_jet1(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_w_ch_jet1 = hadronic_w_ch_jet1;
  fit_hadronic_w_ch_jet1->~TFitParticlePt();
  new(fit_hadronic_w_ch_jet1) TFitParticlePt("hadronic_w_ch_jet1",
                                                    "hadronic_w_ch_jet1",
                                                    &corr_hadronic_w_ch_jet1,
                                                    &error_hadronic_w_ch_jet1
                                                   );
  //cout <<"TKinFitterDriver::SetWCHUpTypeJets : " << endl;
}


void TKinFitterDriver::SetWCHUpTypeJets(unsigned jet_idx){
  hadronic_w_ch_jet1 = jet_vector.at(jet_idx);
  // nbtags>2 ? "b":"udsc";
  // btag_vector.at(jet_idx)
  double corr_       = nbtags>2 ?  jet_pt_cRegCorr_vector.at(jet_idx):1.;
  double resolution_ = nbtags>2 ?  jet_pt_cRegRes_vector.at(jet_idx):jet_pt_resolution_vector.at(jet_idx);
  double Pt  = corr_ * hadronic_w_ch_jet1.Pt();
  // + 10% of jet pT to jet resolution
  //resolution_ += 0.1 * Pt;
  double Eta = hadronic_w_ch_jet1.Eta();
  double Phi = hadronic_w_ch_jet1.Phi();
  double M   = corr_ * hadronic_w_ch_jet1.M();
  hadronic_w_ch_jet1.SetPtEtaPhiM(Pt, Eta, Phi, M);
  error_hadronic_w_ch_jet1(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_w_ch_jet1 = hadronic_w_ch_jet1;
  fit_hadronic_w_ch_jet1->~TFitParticlePt();
  new(fit_hadronic_w_ch_jet1) TFitParticlePt("hadronic_w_ch_jet1",
                                                    "hadronic_w_ch_jet1",
                                                    &corr_hadronic_w_ch_jet1,
                                                    &error_hadronic_w_ch_jet1
                                                   );

  corr_METv = corr_METv - ( hadronic_w_ch_jet1 - jet_vector.at(jet_idx) );
  //remove pz component
  corr_METv.SetPtEtaPhiM(corr_METv.Pt(), 0., corr_METv.Phi(), 0.);
  //cout <<"TKinFitterDriver::SetWCHUpTypeJets : " << endl;

}


void TKinFitterDriver::SetWCHDownTypeJets(TLorentzVector jet_, double resolution_){
  hadronic_w_ch_jet2=jet_;
  double Pt = hadronic_w_ch_jet2.Pt();
  error_hadronic_w_ch_jet2(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_w_ch_jet2 = hadronic_w_ch_jet2;
  fit_hadronic_w_ch_jet2->~TFitParticlePt();
  new(fit_hadronic_w_ch_jet2) TFitParticlePt("hadronic_w_ch_jet2",
                                                    "hadronic_w_ch_jet2",
                                                    &corr_hadronic_w_ch_jet2,
                                                    &error_hadronic_w_ch_jet2
                                                   );
  //cout << "TKinFitterDriver::SetWCHDownTypeJets : " << endl;
}


void TKinFitterDriver::SetWCHDownTypeJets(unsigned jet_idx){
  hadronic_w_ch_jet2 = jet_vector.at(jet_idx);
  // nbtags>2 ? "b":"udsc";
  // btag_vector.at(jet_idx)
  double corr_       = nbtags>2 ?  jet_pt_bRegCorr_vector.at(jet_idx):1.;
  double resolution_ = nbtags>2 ?  jet_pt_bRegRes_vector.at(jet_idx):jet_pt_resolution_vector.at(jet_idx);
  double Pt  = corr_ * hadronic_w_ch_jet2.Pt();
  // + 10% of jet pT to jet resolution
  //resolution_ += 0.1 * Pt;
  double Eta = hadronic_w_ch_jet2.Eta();
  double Phi = hadronic_w_ch_jet2.Phi();
  double M   = corr_ * hadronic_w_ch_jet2.M();
  hadronic_w_ch_jet2.SetPtEtaPhiM(Pt, Eta, Phi, M);
  error_hadronic_w_ch_jet2(0,0) = resolution_*resolution_*Pt*Pt;
  corr_hadronic_w_ch_jet2 = hadronic_w_ch_jet2;
  fit_hadronic_w_ch_jet2->~TFitParticlePt();
  new(fit_hadronic_w_ch_jet2) TFitParticlePt("hadronic_w_ch_jet2",
                                                    "hadronic_w_ch_jet2",
                                                    &corr_hadronic_w_ch_jet2,
                                                    &error_hadronic_w_ch_jet2
                                                   );

  corr_METv = corr_METv - ( hadronic_w_ch_jet2 - jet_vector.at(jet_idx) );
  //remove pz component
  corr_METv.SetPtEtaPhiM(corr_METv.Pt(), 0., corr_METv.Phi(), 0.);
  //cout <<"TKinFitterDriver::SetWCHDownTypeJets : " << endl;

}


void TKinFitterDriver::SetLepton(TLorentzVector lepton_){
  lepton=lepton_;
  double Pt = lepton.Pt();
  double Eta = lepton.Eta();
  error_lepton(0,0)=TMath::Power(0.02*Pt,2);
  //fit_lepton->~TFitParticlePt();
  fit_lepton->reset();
  new(fit_lepton) TFitParticlePt("lepton",
                                        "lepton",
                                        &lepton,
                                        &error_lepton
                                       );
  //cout << "TKinFitterDriver::SetLepton : " << endl;
}

void TKinFitterDriver::SetExtraJets(std::vector<Int_t> extra_jet_){
  corr_extra_jets.clear();
  // push back extra jets
  for(unsigned i(0); i != extra_jet_.size(); i++){
    corr_extra_jets.push_back(jet_vector.at(extra_jet_.at(i)));
    Double_t Pt_extra = jet_vector.at(extra_jet_.at(i)).Pt();
    error_extra_jets.emplace_back(1,1);
    error_extra_jets.back().Zero();
    Double_t extra_resolution_pt = jet_pt_resolution_vector.at(extra_jet_.at(i));
    error_extra_jets.back()(0,0) = extra_resolution_pt*extra_resolution_pt*Pt_extra*Pt_extra;
  }


  //exit function
  if(!IsFitExtraJets){
    return;
  }

  // make TAbsFitParticle object
  for(auto* fit_par:fit_extra_jets){
    delete fit_par;
  }
  fit_extra_jets.clear();
  for(unsigned i(0); i != corr_extra_jets.size(); i++){
    fit_extra_jets.push_back(new TFitParticlePt("extra_jets",
                                                "extra_jets",
                                                &corr_extra_jets.at(i),
                                                &error_extra_jets.at(i)
                                               )
                            );
  }

}

void TKinFitterDriver::SetMET(TLorentzVector met_){
  METv = met_;
  corr_METv = met_;
}


void TKinFitterDriver::SetMETShift(double met_pt_up, double met_pt_down, double met_phi_up, double met_phi_down){
  
  double met_pt = METv.Pt();
  double met_phi = METv.Phi();
  MET_pt_shift = std::max(fabs(met_pt_up-met_pt), fabs(met_pt-met_pt_down));
  MET_phi_shift = std::max(fabs(met_phi_up-met_phi), fabs(met_phi-met_phi_down));
}


void TKinFitterDriver::SetMETShift(double met_shiftX, double met_shiftY){
  
  double met_px = METv.Px();
  double met_py = METv.Py();
  double met_phi = METv.Phi();

  MET_pt_shift = met_shiftX; //XXX pt->px, phi->py
  MET_phi_shift = met_shiftY; //XXX pt->px, phi->py
  //MET_pt_shift = met_shiftX*TMath::Cos(met_phi) - met_shiftY*TMath::Sin(met_phi);
  //MET_phi_shift = met_shiftX*TMath::Sin(met_phi) + met_shiftY*TMath::Cos(met_phi);
  //
  //cout << "debug" << endl;
  //cout << MET_pt_shift << endl;
  //cout << MET_phi_shift << endl;
}


void TKinFitterDriver::SetNeutrino(TLorentzVector met_, int i){

  double met_px = met_.Px();
  double met_py = met_.Py();
  double pz = neutrino_pz_sol.at(i);
  neutrino_pxpypz.SetPxPyPzE(met_px, met_py, pz, TMath::Sqrt(met_.E()*met_.E()+pz*pz));
  neutrino_vec3 = neutrino_pxpypz.Vect();

  //fit_neutrino_pxpypz->~TFitParticleMCCart();
  fit_neutrino_pxpypz->reset();
  new(fit_neutrino_pxpypz) TFitParticleMCCart("neutrino_pxpypz",
                                          "neutrino_pxpypz",
                                          &neutrino_vec3,
                                          0.,
                                          nullptr
                                         );
 
  // unclustered part.
  Double_t uncl_pX=0., uncl_pY=0.;
  // jet Px, Py negative sum
  for(auto& jet : jet_vector){
    uncl_pX -= jet.Px();
    uncl_pY -= jet.Py();
  }
  // letpon Px, Py negative sum
  uncl_pX -= lepton.Px();
  uncl_pY -= lepton.Py();
  // met Px, Py negative sum
  // use original MET instead of corrected MET
  uncl_pX -= METv.Px();
  uncl_pY -= METv.Py();

  //
  uncl_pxpy.SetPxPyPzE(uncl_pX, uncl_pY, 0., TMath::Sqrt(uncl_pX*uncl_pX+uncl_pY*uncl_pY));
  uncl_vec3 = uncl_pxpy.Vect();
  error_uncl_pxpy(0,0) = TMath::Power(uncl_pX*0.2, 2);
  error_uncl_pxpy(1,1) = TMath::Power(uncl_pY*0.2, 2);
  //error_uncl_pxpy(2,2) = 1e-6;


  //
  fit_uncl_pxpy->reset();
  new(fit_uncl_pxpy) TFitParticlePxPy("uncl_pxpy",
                                        "uncl_pxpy",
                                        &uncl_pxpy,
					                    &error_uncl_pxpy
                                       );

  

}


void TKinFitterDriver::SetNeutrinoSmallerPz(TLorentzVector met_){
  this->SetMET(met_);
  this->Sol_Neutrino_Pz();
  if(IsRealNeuPz){
    if(fabs(neutrino_pz_sol.at(0)) < fabs(neutrino_pz_sol.at(1))){
      this->SetNeutrino(corr_METv, 0);
    }
    else{
      this->SetNeutrino(corr_METv, 1);
    }
  }
}

void TKinFitterDriver::SetCurrentPermutationJets(){

  hadronic_top_b_jet_idx=-1;
  leptonic_top_b_jet_idx=-1;
  w_ch_up_type_jet_idx=-1;
  w_ch_down_type_jet_idx=-1;

  std::vector<Int_t> extra_jets;

  for(UInt_t i=0; i<permutation_vector.size(); i++){
    JET_ASSIGNMENT jet_assignment_idx = permutation_vector.at(i);
    if( jet_assignment_idx == HADRONIC_TOP_B ) hadronic_top_b_jet_idx=i;
    else if( jet_assignment_idx == LEPTONIC_TOP_B ) leptonic_top_b_jet_idx=i;
    else if( jet_assignment_idx == W_CH_UP_TYPE ) w_ch_up_type_jet_idx=i;
    else if( jet_assignment_idx == W_CH_DOWN_TYPE ) w_ch_down_type_jet_idx=i;
    else extra_jets.push_back(i);
  }
  //cout << k << l << m << n << endl;
  // using ts-correction
  //this->SetHadronicTopBJets(jet_vector.at(hadronic_top_b_jet_idx));
  //this->SetLeptonicTopBJets(jet_vector.at(leptonic_top_b_jet_idx));
  //this->SetWCHUpTypeJets(   jet_vector.at(w_ch_up_type_jet_idx)  );
  //this->SetWCHDownTypeJets( jet_vector.at(w_ch_down_type_jet_idx));

  // using jetMET POG jer
  //this->SetHadronicTopBJets( jet_vector.at(hadronic_top_b_jet_idx), jet_pt_resolution_vector.at(hadronic_top_b_jet_idx) );
  //this->SetLeptonicTopBJets( jet_vector.at(leptonic_top_b_jet_idx), jet_pt_resolution_vector.at(leptonic_top_b_jet_idx) );
  //this->SetWCHUpTypeJets(    jet_vector.at(w_ch_up_type_jet_idx), jet_pt_resolution_vector.at(w_ch_up_type_jet_idx) );
  //this->SetWCHDownTypeJets(  jet_vector.at(w_ch_down_type_jet_idx), jet_pt_resolution_vector.at(w_ch_down_type_jet_idx) );
  //this->SetExtraJets( extra_jets);

  // using b/c res and jetMET POG jet
  // reset METv
  corr_METv = METv;
  this->SetHadronicTopBJets(hadronic_top_b_jet_idx);
  this->SetLeptonicTopBJets(leptonic_top_b_jet_idx);
  this->SetWCHUpTypeJets(w_ch_up_type_jet_idx);
  this->SetWCHDownTypeJets(w_ch_down_type_jet_idx);
  this->SetExtraJets( extra_jets);

}


bool TKinFitterDriver::Check_BJet_Assignment(){

  int nbtags_in_four_jets=0;
  int w_ch_up_type_jet_idx=-1;
  int w_ch_down_type_jet_idx=-1;
  for(UInt_t i=0; i<permutation_vector.size(); i++){
    JET_ASSIGNMENT jet_assignment_idx = permutation_vector.at(i);
    bool IsBTagged = btag_vector.at(i);
    if(nbtags>=2){
      if( jet_assignment_idx == HADRONIC_TOP_B && IsBTagged ) nbtags_in_four_jets+=1;
      else if( jet_assignment_idx == LEPTONIC_TOP_B && IsBTagged ) nbtags_in_four_jets+=1;

      if( jet_assignment_idx == W_CH_UP_TYPE)   w_ch_up_type_jet_idx   = i;
      else if( jet_assignment_idx == W_CH_DOWN_TYPE) w_ch_down_type_jet_idx = i;
    }


  }

  bool true_bjet_assignment=false;
  if(nbtags>=2 && nbtags_in_four_jets==2){
    if(btag_csv_vector.size()>0){
      if(btag_csv_vector.at(w_ch_up_type_jet_idx) <= btag_csv_vector.at(w_ch_down_type_jet_idx)){
        true_bjet_assignment=true;
      }
      else{
        //pass
      }
    }
    else{
      true_bjet_assignment=true;
    }
  }

  //cout << "TKinFitterDriver::Check_BJet_Assignment : " << endl; 
  return true_bjet_assignment;
}


bool TKinFitterDriver::Kinematic_Cut(){
  TLorentzVector hadronic_w_ch = hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
  TLorentzVector hadronic_top  = hadronic_top_b_jet + hadronic_w_ch;
  TLorentzVector leptonic_top = leptonic_top_b_jet + lepton + neutrino_pxpypz;
  double hadronic_top_mass = hadronic_top.M();
  double hadronic_w_ch_mass = hadronic_w_ch.M();

  //std::cout << "hadronic_top mass: " << hadronic_top_mass << std::endl;
  //std::cout << "mbl              : " << (leptonic_top_b_jet+lepton).M() << std::endl;
  //std::cout << "top dphi              : " << fabs(hadronic_top.DeltaPhi(leptonic_top)) << std::endl;
  double Mbl = (leptonic_top_b_jet+lepton).M();
  double prefit_chi2 = ( hadronic_top_mass - 172.5)*( hadronic_top_mass - 172.5)/(25*25);
         prefit_chi2 += ( Mbl - 105 )*( Mbl - 105)/(30*30);
         // jet pt cut. one of b tagged jet in hadronic side allowed to be below 30GeV
  double prefit_chi2_cut = 6.18; //chi2(k=2) 2 sigma 95.45% CI

  if(nbtags==2){
    if(njets==4){
      //pass
    }
    else if(njets==5){
      prefit_chi2_cut = 4.61; // 1.64sigma 90% CI
    }
    else if(njets==6){
      prefit_chi2_cut = 3.22; // 1.28sigma 80% CI
    }
    else{
      prefit_chi2_cut = 2.30; // 1 sigma 68% CI 
    }
  
  }
  else{ // more than 3b
    if(njets==4){
      prefit_chi2_cut = 4.61; // 1.64sigma 90% CI
    }
    else if(njets==5){
      prefit_chi2_cut = 3.22; // 1.28sigma 80% CI
    }
    else{
      prefit_chi2_cut = 2.30; // 1 sigma 68% CI 
    } 
  }

  return (hadronic_w_ch_jet1.Pt() > 30. &&
          (hadronic_w_ch_jet2.Pt() > (nbtags>2 ? 25.:30.)) && 
          hadronic_top_b_jet.Pt() > 30. &&
          leptonic_top_b_jet.Pt() > 30.
         ) &&
         // tighter M_top cut to compansate increasing combinatorics
         //
         ( (hadronic_top_mass > 130) && (hadronic_top_mass < 210) ) &&
         ( Mbl < 150) &&
         //
         //( prefit_chi2 < prefit_chi2_cut )// chi2(k=2) distribution 2 sigma value 
         // leptonic_top mass cut removed.
         // tt_dphi cut removed (Jul 29 2022). tt_dphi is sensitive to ISR
         // tt_dphi cut come back (Feb 24 2023).
         // tt_dphi simulation unc. thought to be covered by systematic unc.
         (fabs(hadronic_top.DeltaPhi(leptonic_top)) > 1.5);
}


bool TKinFitterDriver::Kinematic_Cut_LD(){
  TLorentzVector hadronic_w_ch = hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
  TLorentzVector hadronic_top  = hadronic_top_b_jet + hadronic_w_ch;
  TLorentzVector leptonic_top = leptonic_top_b_jet + lepton + neutrino_pxpypz;
  double hadronic_top_mass = hadronic_top.M();
  double hadronic_w_ch_mass = hadronic_w_ch.M();

  //std::cout << "hadronic_top mass: " << hadronic_top_mass << std::endl;
  //std::cout << "mbl              : " << (leptonic_top_b_jet+lepton).M() << std::endl;
  //std::cout << "top dphi              : " << fabs(hadronic_top.DeltaPhi(leptonic_top)) << std::endl;
  double Mbl = (leptonic_top_b_jet+lepton).M();
  double prefit_chi2 = ( hadronic_top_mass - 172.5)*( hadronic_top_mass - 172.5)/(25*25);
         prefit_chi2 += ( Mbl - 105 )*( Mbl - 105)/(30*30);
         // jet pt cut. one of b tagged jet in hadronic side allowed to be below 30GeV
  double prefit_chi2_cut = 6.18; //chi2(k=2) 2 sigma 95.45% CI

  if(nbtags==2){
    if(njets==4){
      //pass
    }
    else if(njets==5){
      prefit_chi2_cut = 4.61; // 1.64sigma 90% CI
    }
    else if(njets==6){
      prefit_chi2_cut = 3.22; // 1.28sigma 80% CI
    }
    else{
      prefit_chi2_cut = 2.30; // 1 sigma 68% CI 
    }
  
  }
  else{ // more than 3b
    if(njets==4){
      prefit_chi2_cut = 4.61; // 1.64sigma 90% CI
    }
    else if(njets==5){
      prefit_chi2_cut = 3.22; // 1.28sigma 80% CI
    }
    else{
      prefit_chi2_cut = 2.30; // 1 sigma 68% CI 
    } 
  }

  return (hadronic_w_ch_jet1.Pt() > 30. &&
          (hadronic_w_ch_jet2.Pt() > (nbtags>2 ? 25.:30.)) && 
          hadronic_top_b_jet.Pt() > 30. &&
          leptonic_top_b_jet.Pt() > 30.
         ) &&
         // tighter M_top cut to compansate increasing combinatorics
         //
         ( (hadronic_top_mass > 100) && (hadronic_top_mass < 240) ) &&
         ( Mbl < 170) &&
         //
         //( prefit_chi2 < prefit_chi2_cut )// chi2(k=2) distribution 2 sigma value 
         // leptonic_top mass cut removed.
         // tt_dphi cut removed (Jul 29 2022). tt_dphi is sensitive to ISR
         // tt_dphi cut come back (Feb 24 2023).
         // tt_dphi simulation unc. thought to be covered by systematic unc.
         (fabs(hadronic_top.DeltaPhi(leptonic_top)) > 1.5);
}


bool TKinFitterDriver::Quality_Cut(){

  //std::cout << "njets  : " << njets << std::endl;
  //std::cout << "chi2 1 : " << fabs(this->CalcFitQuality(fit_hadronic_top_b_jet)) << std::endl;
  //std::cout << "chi2 2 : " << fabs(this->CalcFitQuality(fit_leptonic_top_b_jet)) << std::endl;
  //std::cout << "chi2 3 : " << fabs(this->CalcFitQuality(fit_hadronic_w_ch_jet1)) << std::endl;
  //std::cout << "chi2 4 : " << fabs(this->CalcFitQuality(fit_hadronic_w_ch_jet2)) << std::endl;

  // 230117: abs 3sigma -> 1sigma same direction
  // abs 3sigma
  //return fabs(this->CalcFitQuality(fit_hadronic_top_b_jet))<3 && 
  //       fabs(this->CalcFitQuality(fit_leptonic_top_b_jet))<3 &&
  //       fabs(this->CalcFitQuality(fit_hadronic_w_ch_jet1))<3 &&
  //       fabs(this->CalcFitQuality(fit_hadronic_w_ch_jet2))<3;
  //
  // 1 sigma same direction, 3 hadronic side jets
  return (this->CalcFitQuality(fit_hadronic_top_b_jet) + this->CalcFitQuality(fit_hadronic_w_ch_jet1) + this->CalcFitQuality(fit_hadronic_w_ch_jet2) < 3 ) &&
         (this->CalcFitQuality(fit_hadronic_top_b_jet) + this->CalcFitQuality(fit_hadronic_w_ch_jet1) + this->CalcFitQuality(fit_hadronic_w_ch_jet2) > -3 ) ;
}

void TKinFitterDriver::SetConstraint(){
  //TODO: will update to be able to set top-mass

  // reset constrain
  //
  //constrain_leptonic_top_MGaus = new TFitConstraintMGaus("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, top_mass, 1.5);

  //constrain_top_M->Clear();
  //new(constrain_top_M) TFitConstraintM("top_mass_constraint", "top_mass_constraint", 0, 0, 0.);
  //constrain_top_M->addParticles1(fit_hadronic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  //constrain_top_M->addParticles2(fit_leptonic_top_b_jet, fit_lepton, fit_neutrino_pxpypz);
  //constrain_hadronic_top_M->Clear();
  //new(constrain_hadronic_top_M) TFitConstraintM("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, top_mass);
  //constrain_hadronic_top_M->addParticles1(fit_hadronic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  //constrain_hadronic_top_MGaus->Clear();
  constrain_hadronic_top_MGaus->~TFitConstraintMGaus();
  new(constrain_hadronic_top_MGaus) TFitConstraintMGaus("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, (Double_t)top_mass, 1.5);
  constrain_hadronic_top_MGaus->addParticles1(fit_hadronic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  //constrain_leptonic_top_M->Clear();
  //new(constrain_leptonic_top_M) TFitConstraintM("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, top_mass);
  //constrain_leptonic_top_M->addParticles1(fit_leptonic_top_b_jet, fit_neutrino_pxpy);
  //constrain_leptonic_top_M->addParticles1(fit_leptonic_top_b_jet, fit_lepton, fit_neutrino_pxpypz);
  //constrain_leptonic_top_MGaus->Clear();
  constrain_leptonic_top_MGaus->~TFitConstraintMGaus();
  new(constrain_leptonic_top_MGaus) TFitConstraintMGaus("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, (Double_t)top_mass, 1.5);
  constrain_leptonic_top_MGaus->addParticles1(fit_leptonic_top_b_jet, fit_lepton, fit_neutrino_pxpypz);
  //constrain_leptonic_W_M->Clear();
  //new(constrain_leptonic_W_M) TFitConstraintM("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4);
  //constrain_leptonic_W_M->addParticles1(fit_neutrino_pxpypz, fit_lepton); // lepton is included in fit_neutrino_pz
  //constrain_leptonic_W_MGaus->Clear();
  constrain_leptonic_W_MGaus->~TFitConstraintMGaus();
  new(constrain_leptonic_W_MGaus) TFitConstraintMGaus("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4, 2.085);
  constrain_leptonic_W_MGaus->addParticles1(fit_lepton, fit_neutrino_pxpypz);
  //constrain_hadronic_W_M->Clear();
  //new(constrain_hadronic_W_M) TFitConstraintM("hadronic_w_mass_constraint", "hadronic_w_mass_constraint", 0, 0, 80.4);
  //constrain_hadronic_W_M->addParticles1(fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  //constrain_hadronic_W_MGaus->Clear();
  //constrain_hadronic_W_MGaus->addParticles1(fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  
  constrain_w_ch_pX->Clear();
  constrain_w_ch_pY->Clear();
  new(constrain_w_ch_pX) TFitConstraintEp("w_ch_pX","w_ch_pX",TFitConstraintEp::component::pX,0.);
  new(constrain_w_ch_pY) TFitConstraintEp("w_ch_pY","w_ch_pY",TFitConstraintEp::component::pY,0.);
  constrain_w_ch_pX->addParticle(fit_hadronic_w_ch_jet1);
  constrain_w_ch_pX->addParticle(fit_hadronic_w_ch_jet2);
  constrain_w_ch_pY->addParticle(fit_hadronic_w_ch_jet1);
  constrain_w_ch_pY->addParticle(fit_hadronic_w_ch_jet2);

  constrain_pX->Clear();
  constrain_pY->Clear();
  new(constrain_pX) TFitConstraintEp("pX","pX",TFitConstraintEp::component::pX,0.);
  new(constrain_pY) TFitConstraintEp("pY","pY",TFitConstraintEp::component::pY,0.);

  //
  for(auto*  fit_par: std::vector<TAbsFitParticle*>({fit_hadronic_top_b_jet, fit_leptonic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2})){
    constrain_pX->addParticle(fit_par);
    constrain_pY->addParticle(fit_par);
  }
  //

  if(IsFitExtraJets){
    for(auto* fit_par : fit_extra_jets){
      constrain_pX->addParticle(fit_par);
      constrain_pY->addParticle(fit_par);
    }
  }
  for(auto*  fit_par: std::vector<TAbsFitParticle*>({fit_lepton, fit_neutrino_pxpypz})){
    constrain_pX->addParticle(fit_par);
    constrain_pY->addParticle(fit_par);
  }
  // if uncl_pxpy is zero vector, do not add to 'particle' and 'constraint'
  if(uncl_pxpy.Px()!=0 || uncl_pxpy.Py()!=0){
    constrain_pX->addParticle(fit_uncl_pxpy);
    constrain_pY->addParticle(fit_uncl_pxpy);
  }
  //
  Double_t pX = constrain_pX->getInitValue();
  Double_t pY = constrain_pY->getInitValue();


  //if(pX != 0 || pY != 0){
  //  cout << "TKinFitterDriver: check pT balence" << endl;
  //  cout << pX <<"\t\t\t\t" << pY << endl;
  //}

  constrain_pX->setConstraint(pX);
  constrain_pY->setConstraint(pY);
  //cout << "constrain_pX : " << pX << endl;
  //cout << "constrain_pY : " << pY << endl;

}


void TKinFitterDriver::SetFitter(){
  fitter->reset();


  //add MeasParticles to vary Et,Eta,Phi
  fitter->addMeasParticle( fit_hadronic_top_b_jet );
  fitter->addMeasParticle( fit_leptonic_top_b_jet );
  fitter->addMeasParticle( fit_hadronic_w_ch_jet1 );
  fitter->addMeasParticle( fit_hadronic_w_ch_jet2 );

  if(IsFitExtraJets){
    for(auto* fit_par : fit_extra_jets){
      fitter->addMeasParticle( fit_par );
    }
  }
  // if uncl_pxpy is zero vector, do not add to 'particle' and 'constraint'
  if(uncl_pxpy.Px()!=0 || uncl_pxpy.Py()!=0){
    fitter->addMeasParticle( fit_uncl_pxpy );
  }

  //add UnmeasParticles not to vary Et,Eta,Phi
  // Also, Px, Py should be constrained
  fitter->addMeasParticle( fit_lepton );
  fitter->addUnmeasParticle( fit_neutrino_pxpypz );
  //fitter->addUnmeasParticle( fit_neutrino_pz );
  //fitter->addUnmeasParticle( fit_lepton );
  //fitter->addUnmeasParticle( fit_neutrino_pxpy );
  //add Constraint
  //fitter->addConstraint( constrain_top_M );
  //fitter->addConstraint( constrain_hadronic_top_M );
  fitter->addConstraint( constrain_hadronic_top_MGaus );
  //fitter->addConstraint( constrain_leptonic_top_M );
  fitter->addConstraint( constrain_leptonic_top_MGaus );
  //fitter->addConstraint( constrain_leptonic_W_M );
  fitter->addConstraint( constrain_leptonic_W_MGaus );
  //fitter->addConstraint( constrain_hadronic_W_M );
  //fitter->addConstraint( constrain_hadronic_W_MGaus );

  //fitter->addConstraint( constrain_w_ch_pX );
  //fitter->addConstraint( constrain_w_ch_pY );
  fitter->addConstraint( constrain_pX );
  fitter->addConstraint( constrain_pY );

  //Set convergence criteria
  fitter->setMaxNbIter( 1000 ); //50 is default
  fitter->setMaxDeltaS( 1e-2 );
  fitter->setMaxF( 1e-2 ); //1e-1 is default
  fitter->setVerbosity(1);
  //cout << "TKinFitterDriver::SetFitter : " << endl;
}


void TKinFitterDriver::Fit(){
  this->SetConstraint();
  this->SetFitter();
  this->SaveResults();
}


void TKinFitterDriver::SaveResults(){

  fit_result.status=-1; // -1 means fit not performed
  fit_result.status = fitter->fit();
  // save kinematic variable before fit
  TLorentzVector hadronic_top = hadronic_top_b_jet + hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
  TLorentzVector leptonic_W = neutrino_pxpypz + lepton;
  TLorentzVector leptonic_top = leptonic_top_b_jet + leptonic_W;
  fit_result.hadronic_top_M = hadronic_top.M();
  fit_result.hadronic_top_pt = hadronic_top.Pt();
  fit_result.leptonic_top_M = leptonic_top.M();
  fit_result.leptonic_top_pt = leptonic_top.Pt();
  fit_result.leptonic_W_M = leptonic_W.M();
  fit_result.IsRealNeuPz = IsRealNeuPz;

  fit_result.hadronic_top_b_pt = hadronic_top_b_jet.Pt();
  fit_result.leptonic_top_b_pt = leptonic_top_b_jet.Pt();
  fit_result.w_ch_up_type_pt = hadronic_w_ch_jet1.Pt();
  fit_result.w_ch_down_type_pt = hadronic_w_ch_jet2.Pt();
  
  fit_result.hadronic_top_b_jet_idx = hadronic_top_b_jet_idx;
  fit_result.leptonic_top_b_jet_idx = leptonic_top_b_jet_idx;
  fit_result.w_ch_up_type_jet_idx = w_ch_up_type_jet_idx;
  fit_result.w_ch_down_type_jet_idx = w_ch_down_type_jet_idx;

  fit_result.hadronic_top_b_jet_pull = (*fit_hadronic_top_b_jet->getPull())(0,0);
  fit_result.leptonic_top_b_jet_pull = (*fit_leptonic_top_b_jet->getPull())(0,0);
  fit_result.w_ch_up_type_jet_pull = (*fit_hadronic_w_ch_jet1->getPull())(0,0);
  fit_result.w_ch_down_type_jet_pull = (*fit_hadronic_w_ch_jet2->getPull())(0,0);
                                      
  //
  ///////
  int down_type_jet_b_tagged = 0;
  for(UInt_t i=0; i<permutation_vector.size(); i++){
    JET_ASSIGNMENT jet_assignment_idx = permutation_vector.at(i);
    bool IsBTagged = btag_vector.at(i);
    if( jet_assignment_idx == W_CH_DOWN_TYPE){
      if(IsBTagged){
        down_type_jet_b_tagged=1;
      }
      else{
        //pass
      }
      break;
    }
  }
  fit_result.down_type_jet_b_tagged = down_type_jet_b_tagged;
  ///////
  //
  fit_result.chi2 = 9999;
  //if(fit_result.status!=-10){ // -10 means singular matrix
    // save S and F
    //fit_result.currS = fitter->getS();
    //fit_result.deltaS = fitter->getDeltaS();
    //fit_result.hadronic_top_mass_F = constrain_hadronic_top_M->getCurrentValue();
    //fit_result.hadronic_top_mass_F = constrain_hadronic_top_MGaus->getCurrentValue();
    //fit_result.leptonic_top_mass_F = constrain_leptonic_top_M->getCurrentValue();
    //fit_result.leptonic_top_mass_F = constrain_leptonic_top_MGaus->getCurrentValue();
    //fit_result.leptonic_w_mass_F = constrain_leptonic_W_M->getCurrentValue();
    //fit_result.leptonic_w_mass_F = constrain_leptonic_W_MGaus->getCurrentValue();
    //}
    // re-calculate chi2,
    //chi2 term comes from constraint is not accurate
    fit_result.chi2 = this->CalcChi2("all");
    fit_result.chi2_lep = this->CalcChi2("lep");
    fit_result.chi2_had = this->CalcChi2("had");
    //fit_result.lambda = fitter->getLambda();

    // calc. dijet mass
    const TLorentzVector *fitted_jet1 = fit_hadronic_w_ch_jet1->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_jet2 = fit_hadronic_w_ch_jet2->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_jet3 = fit_hadronic_top_b_jet->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_jet4 = fit_leptonic_top_b_jet->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_neutrino = fit_neutrino_pxpypz->getCurr4Vec(); // get address of fitted object
    const TLorentzVector fitted_dijet = (*fitted_jet1) + (*fitted_jet2);
    //const TLorentzVector fitted_dijet_high = fitted_jet3->Pt()>fitted_jet2->Pt() ? (*fitted_jet1) + (*fitted_jet3):fitted_dijet;

    // if jet2 comes from top decaying to heavy H+, jet2 is the softest one.
    bool condition1 = fitted_jet2->Pt() < fitted_jet3->Pt();
    bool condition2 = fitted_jet2->Pt() < fitted_jet1->Pt();
    bool condition3 = fitted_jet2->Pt() > fitted_jet3->Pt();
    bool condition4 = fitted_jet2->Pt() > fitted_jet1->Pt();


    bool had_top_condition = ( condition1 && condition2 ); // || ( condition3 && condition4 );
    bool lep_top_condition = true;

    const TLorentzVector fitted_dijet_high = had_top_condition && lep_top_condition ? (*fitted_jet1) + (*fitted_jet3):fitted_dijet;
    // good for SM, 080, 150, not good for 160
    //const TLorentzVector fitted_dijet_high = fitted_jet4->DeltaR(*fitted_jet3) < fitted_jet4->DeltaR(*fitted_jet2)  ? (*fitted_jet1) + (*fitted_jet3):fitted_dijet;
    const TLorentzVector fitted_dijet_new1 = fitted_jet3->Pt()>fitted_jet2->Pt()+30 ? (*fitted_jet1) + (*fitted_jet3):fitted_dijet;
    fit_result.fitted_dijet_M = fitted_dijet.M(); // save dijet mass
    fit_result.fitted_dijet_M_high = fitted_dijet_high.M(); // save dijet mass

    TLorentzVector initial_dijet = hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
    TLorentzVector corrected_dijet = corr_hadronic_w_ch_jet1 + corr_hadronic_w_ch_jet2;
    fit_result.initial_dijet_M = initial_dijet.M(); // save dijet mass
    fit_result.fitted_dijet_M_new2 = this->ComparingInHadTopRestFrame(fitted_jet1,fitted_jet2,fitted_jet3);
    TLorentzVector corrected_dijet_high = fitted_jet3->Pt()>fitted_jet2->Pt() ? corr_hadronic_w_ch_jet1 + corr_hadronic_top_b_jet:corrected_dijet;
    fit_result.corrected_dijet_M = corrected_dijet.M(); // save dijet mass
    fit_result.corrected_dijet_M_high = corrected_dijet_high.M(); // save dijet mass


}

double TKinFitterDriver::ComparingInHadTopRestFrame(const TLorentzVector *jet1,const TLorentzVector *jet2, const TLorentzVector *had_top_b_jet){
  const TLorentzVector fitted_had_top = *jet1 + *jet2 + *had_top_b_jet;
  const TLorentzVector w_ch_cand1 = *jet1 + *jet2;
  const TLorentzVector w_ch_cand2 = *jet1 + *had_top_b_jet;

  TVector3 had_top_boost_vector = fitted_had_top.BoostVector();
  TLorentzVector w_ch_cand1_top_rest  = TLorentzVector(w_ch_cand1.Px(), w_ch_cand1.Py(), w_ch_cand1.Pz(), w_ch_cand1.E());
  TLorentzVector top_b_cand1_top_rest = TLorentzVector(had_top_b_jet->Px(), had_top_b_jet->Py(), had_top_b_jet->Pz(), had_top_b_jet->E());
  TLorentzVector w_ch_cand2_top_rest  = TLorentzVector(w_ch_cand2.Px(), w_ch_cand2.Py(), w_ch_cand2.Pz(), w_ch_cand2.E());
  TLorentzVector top_b_cand2_top_rest = TLorentzVector(jet2->Px(), jet2->Py(), jet2->Pz(), jet2->E());

  w_ch_cand1_top_rest.Boost(-had_top_boost_vector);
  top_b_cand1_top_rest.Boost(-had_top_boost_vector);
  w_ch_cand2_top_rest.Boost(-had_top_boost_vector);
  top_b_cand2_top_rest.Boost(-had_top_boost_vector);

  double out_mass;
  if(fabs(w_ch_cand1_top_rest.DeltaPhi(top_b_cand1_top_rest)) > fabs(w_ch_cand2_top_rest.DeltaPhi(top_b_cand2_top_rest))){
    out_mass = w_ch_cand1.M();
  }
  else{
    out_mass = w_ch_cand2.M();
  }
  return out_mass;
}

double TKinFitterDriver::CalcChi2(TString option){
  double chi2=0;
  // jets
  if(option == "had" || option == "all"){
    chi2 += this->CalcEachChi2(fit_hadronic_top_b_jet);
    chi2 += this->CalcEachChi2(fit_leptonic_top_b_jet);
    chi2 += this->CalcEachChi2(fit_hadronic_w_ch_jet1);
    chi2 += this->CalcEachChi2(fit_hadronic_w_ch_jet2);
  }
  //if(option == "lep" || option == "all"){
  ////lepton
  //chi2 += this->CalcEachChi2(fit_lepton);
  ////extra pt
  //chi2 += this->CalcEachChi2(fit_neutrino_pxpypz);
  //}
  //
  if(option == "extra" || option == "all"){

    if(IsFitExtraJets){
      for(auto* fit_par : fit_extra_jets){
        chi2 += this->CalcEachChi2(fit_par);
      }
    }
    chi2 += this->CalcEachChi2(fit_uncl_pxpy);
  }
  // mass constraints
  //chi2 += this->CalcEachChi2(constrain_leptonic_W_M, 2.085);
  //chi2 += this->CalcEachChi2(constrain_hadronic_W_M, 2.085);
  //chi2 += this->CalcEachChi2(constrain_hadronic_W_MGaus, 2.085);
  chi2 += this->CalcEachChi2(constrain_leptonic_W_MGaus, 2.085);
  chi2 += this->CalcEachChi2(constrain_hadronic_top_MGaus, 1.5);
  //chi2 += this->CalcEachChi2(constrain_hadronic_top_M, 1.5);
  chi2 += this->CalcEachChi2(constrain_leptonic_top_MGaus, 1.5);
  //chi2 += this->CalcEachChi2(constrain_leptonic_top_M, 1.5);
  return chi2;
}

double TKinFitterDriver::CalcEachChi2(TAbsFitParticle* ptr){

  const TMatrixD* iniPar = ptr->getParIni();
  const TMatrixD* currPar = ptr->getParCurr();
  const TMatrixD* covMatrix = ptr->getCovMatrix();
  double S = 0.;

  for(int i(0); i<iniPar->GetNcols(); i++){
    double deltaY = (*iniPar)(i,i) - (*currPar)(i,i);
    S += deltaY*deltaY/(*covMatrix)(i,i);
    //std::cout << "currPar    : " << (*currPar)(i,i)  << std::endl; 
    //std::cout << "iniPar     : " << (*iniPar)(i,i)   << std::endl; 
    //std::cout << "covMatrix : " << (*covMatrix)(i,i) << std::endl; 
  }
  return S;
}


double TKinFitterDriver::CalcFitQuality(TAbsFitParticle* ptr){

  const TMatrixD* iniPar = ptr->getParIni();
  const TMatrixD* currPar = ptr->getParCurr();
  const TMatrixD* covMatrix = ptr->getCovMatrix();
  double pull = 0.;

  int i = 0; // first index is jet pT
  double deltaY =  (*iniPar)(i,i) - (*currPar)(i,i);
  double sigma;
  // 1 sigma of jet resolution
  sigma  = 1 * TMath::Sqrt( (*covMatrix)(i,i) );


  pull = deltaY/sigma;
    //std::cout << "currPar    : " << (*currPar)(i,i)  << std::endl; 
    //std::cout << "iniPar     : " << (*iniPar)(i,i)   << std::endl; 
    //std::cout << "covMatrix : " << (*covMatrix)(i,i) << std::endl; 
  return pull;
}


double TKinFitterDriver::CalcPull(TAbsFitParticle* ptr){
  //(*fit_hadronic_top_b_jet->getPull())(0,0);
  const TMatrixD* covMatrix = ptr->getPull();
  double S = 0.;

  //ptr->Print();
  // 4 parm for jets, 1 parm for lep, 3 parm for neu
  // 8 params total, 3 mass constraint
  for(int i(0); i< min(1,covMatrix->GetNrows()); i++){ // min() is used not to include neu pz fit err. 
    double pull = (*covMatrix)(i,0);
    S += pull*pull;
    //cout << "i col: " << i << "\t\tS: " << S << endl;
  }
  return S;
}


double TKinFitterDriver::CalcEachChi2(TFitConstraintM* ptr, double width){
  double S = 0.;
  double deltaY = ptr->getCurrentValue();
  S = (deltaY*deltaY)/(width*width);
  return S;
}


double TKinFitterDriver::CalcEachChi2(TFitConstraintMGaus* ptr, double width){
  Double_t S = 0.;
  const TMatrixD* currPar = ptr->getParCurr();
  Double_t deltaY = 1 - (*currPar)(0,0);
  S = (deltaY*deltaY)/(width*width); //here width means: (mass width)/(mean mass)
  return S;
}

void TKinFitterDriver::Set_had_top_mass_LD(TH1F* histo){
  had_top_mass_LD = histo;
}

void TKinFitterDriver::Set_lep_top_mass_LD(TH1F* histo){
  lep_top_mass_LD = histo;
}

void TKinFitterDriver::Set_mbl_LD(TH1F* histo){
  mbl_LD = histo;
}

double TKinFitterDriver::GetBinContent(TH1* hist, double valx){
  double xmin=hist->GetXaxis()->GetXmin();
  double xmax=hist->GetXaxis()->GetXmax();
  if(xmin>=0) valx=fabs(valx);
  if(valx<xmin) valx=xmin+0.001;
  if(valx>xmax) valx=xmax-0.001;
  return hist->GetBinContent(hist->FindBin(valx));
}

double TKinFitterDriver::CalcLD(){
  TLorentzVector hadronic_w_ch = hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
  TLorentzVector hadronic_top  = hadronic_top_b_jet + hadronic_w_ch;
  TLorentzVector leptonic_top = leptonic_top_b_jet + lepton + corr_METv;
  double hadronic_top_mass = hadronic_top.M();
  double leptonic_top_mass = leptonic_top.M();
  double hadronic_w_ch_mass = hadronic_w_ch.M();

  double had_top_b_score = btag_csv_vector.at(hadronic_top_b_jet_idx);
  double lep_top_b_score = btag_csv_vector.at(leptonic_top_b_jet_idx);

  //std::cout << "hadronic_top mass: " << hadronic_top_mass << std::endl;
  //std::cout << "mbl              : " << (leptonic_top_b_jet+lepton).M() << std::endl;
  //std::cout << "top dphi              : " << fabs(hadronic_top.DeltaPhi(leptonic_top)) << std::endl;
  double Mbl = (leptonic_top_b_jet+lepton).M();
  double LD;
  LD  = this->GetBinContent(had_top_mass_LD, hadronic_top_mass);
  LD *= this->GetBinContent(lep_top_mass_LD, leptonic_top_mass);
  LD *= this->GetBinContent(mbl_LD, Mbl);
  //LD *= this->GetBinContent(lep_top_b_score_LD, had_top_b_score);
  //LD *= this->GetBinContent(lep_top_b_score_LD, lep_top_b_score);
  
  return LD;
}


void TKinFitterDriver::FitCurrentPermutation(){
  fit_result_vector.clear();
  this->Fit();
  fit_result_vector.push_back(fit_result);
}


void TKinFitterDriver::FindBestChi2Fit(bool UseLeading4Jets, bool IsHighMassFitter){
  fit_result_vector.clear();
  fit_result_vector.shrink_to_fit();
  // status -1 : fit not performed
  fit_result.status=-1;


  if(njets<4 || nbtags<2){
    return;
  }
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  //cout << "start jet permutation" << endl;
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  do{
      if(this->Check_BJet_Assignment() == false) continue;
      this->SetCurrentPermutationJets();
      this->Sol_Neutrino_Pz();
      for(unsigned int i_pz(0); i_pz<neutrino_pz_sol.size(); i_pz++) { // neu. pz loop
	//set pz
  	this->SetNeutrino(corr_METv, i_pz);

        if(this->Kinematic_Cut() == false) continue;
        //to initialize fitting parameters
        this->SetCurrentPermutationJets();

        //cout << "pass kin. cut" << endl;
        this->Fit();
        //XXX
        if(this->Quality_Cut() == false) continue;
        //cout << "pass qual. cut" << endl;
            //cout << "result hadronic top mass :" << fit_result.hadronic_top_M << endl;
            //cout << "result hadronic top pt :" << fit_result.hadronic_top_pt << endl;
            //cout << "result dijet mass        :" << fit_result.fitted_dijet_M << endl;
            //cout << "result chi2_had          :" << fit_result.chi2_had << endl;
            //cout << "result chi2_lep          :" << fit_result.chi2_lep << endl;
            //cout << "result chi2              :" << fit_result.chi2 << endl;
  	    fit_result_vector.push_back(fit_result);
      }
    }while(this->NextPermutation(UseLeading4Jets));
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  //cout << "end jet permutation" << endl;
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  std::sort(fit_result_vector.begin(), fit_result_vector.end(), Chi2Comparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), HadTopMComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), HadTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), LepTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), TopPtComparing);

  if(IsHighMassFitter){
    std::sort(fit_result_vector.begin(), fit_result_vector.end(), HighMassFitter);
  }

}


void TKinFitterDriver::FindBestLDFit(bool UseLeading4Jets, bool IsHighMassFitter){
  fit_result_vector.clear();
  fit_result_vector.shrink_to_fit();
  // status -1 : fit not performed
  fit_result.status=-1;


  if(njets<4 || nbtags<2){
    return;
  }
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  //cout << "start jet permutation" << endl;
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    this->FindBestLDPermutaion(UseLeading4Jets);
  //cout << "best LD found" << endl;
    this->SetCurrentPermutationJets();
  //cout << "set best LD permutation jets" << endl;
    this->Sol_Neutrino_Pz();
    for(unsigned int i_pz(0); i_pz<neutrino_pz_sol.size(); i_pz++) { // neu. pz loop
      //if(this->Check_BJet_Assignment() == false) break;
      //to initialize fitting parameters
      this->SetCurrentPermutationJets();
	  //set pz
  	  this->SetNeutrino(corr_METv, i_pz);

      if(this->Kinematic_Cut_LD() == false) continue;
      //cout << "pass kin. cut" << endl;
      this->Fit();
      //XXX
      //if(this->Quality_Cut() == false) continue;
      //cout << "pass qual. cut" << endl;
      //cout << "result hadronic top mass :" << fit_result.hadronic_top_M << endl;
      //cout << "result hadronic top pt :" << fit_result.hadronic_top_pt << endl;
      //cout << "result dijet mass        :" << fit_result.fitted_dijet_M << endl;
      //cout << "result chi2_had          :" << fit_result.chi2_had << endl;
      //cout << "result chi2_lep          :" << fit_result.chi2_lep << endl;
      //cout << "result chi2              :" << fit_result.chi2 << endl;
  	  fit_result_vector.push_back(fit_result);
    }
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  //cout << "end jet permutation" << endl;
  //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  std::sort(fit_result_vector.begin(), fit_result_vector.end(), Chi2Comparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), HadTopMComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), HadTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), LepTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), TopPtComparing);

  if(IsHighMassFitter){
    std::sort(fit_result_vector.begin(), fit_result_vector.end(), HighMassFitter);
  }

}


int TKinFitterDriver::GetStatus(){
  return fit_result.status;
}


double TKinFitterDriver::GetChi2(){
  return fit_result.chi2;
}


double TKinFitterDriver::GetFittedDijetMass(){
  return fit_result.fitted_dijet_M;
}


double TKinFitterDriver::GetInitialDijetMass(){
  return fit_result.initial_dijet_M;
}


double TKinFitterDriver::GetCorrectedDijetMass(){
  return fit_result.corrected_dijet_M;
}


int TKinFitterDriver::GetBestStatus(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).status;
  }
  return out;
}


double TKinFitterDriver::GetBestChi2(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).chi2;
  }
  return out==9999 ? -1. : out;
}

//double TKinFitterDriver::GetBestLambda(){
//  double out=-1;
//  if(fit_result_vector.size()>0){
//    out =fit_result_vector.at(0).lambda;
//  }
//  return out==9999 ? -1. : out;
//}

int TKinFitterDriver::GetBestDownTypeJetBTagged(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).down_type_jet_b_tagged;
  }
  return out;
}

double TKinFitterDriver::GetBestFittedDijetMass(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).fitted_dijet_M;
  }
  return out;
}


double TKinFitterDriver::GetBestFittedDijetMass_high(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).fitted_dijet_M_high;
  }
  return out;
}


double TKinFitterDriver::GetBestInitialDijetMass(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).initial_dijet_M;
  }
  return out;
}


double TKinFitterDriver::GetBestInitialDijetMass_high(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).initial_dijet_M_high;
  }
  return out;
}


double TKinFitterDriver::GetBestCorrectedDijetMass(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).corrected_dijet_M;
  }
  return out;
}


double TKinFitterDriver::GetBestCorrectedDijetMass_high(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).corrected_dijet_M_high;
  }
  return out;
}


double TKinFitterDriver::GetBestHadronicTopMass(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_top_M;
  }
  return out;
}

double TKinFitterDriver::GetBestHadronicTopPt(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_top_pt;
  }
  return out;
}

double TKinFitterDriver::GetBestLeptonicTopMass(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_top_M;
  }
  return out;
}


double TKinFitterDriver::GetBestLeptonicWMass(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_W_M;
  }
  return out;
}


bool TKinFitterDriver::GetBestIsRealNeuPz(){
  bool out=false;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).IsRealNeuPz;
  }
  return out;
}




int TKinFitterDriver::GetBestHadronicTopBJetIdx(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_top_b_jet_idx;
  }
  return out;
}

int TKinFitterDriver::GetBestLeptonicTopBJetIdx(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_top_b_jet_idx;
  }
  return out;
}

int TKinFitterDriver::GetBestHadronicWCHUpTypeJetIdx(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).w_ch_up_type_jet_idx;
  }
  return out;
}

int TKinFitterDriver::GetBestHadronicWCHDownTypeJetIdx(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).w_ch_down_type_jet_idx;
  }
  return out;
}

double TKinFitterDriver::GetBestHadronicTopBJetPull(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_top_b_jet_pull;
  }
  return out;
}
double TKinFitterDriver::GetBestLeptonicTopBJetPull(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_top_b_jet_pull;
  }
  return out;
}



double TKinFitterDriver::GetBestHadronicWCHUptypeJetIdxPull(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).w_ch_up_type_jet_pull;
  }
  return out;
}
double TKinFitterDriver::GetBestHadronicWCHDowntypeJetIdxPull(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).w_ch_down_type_jet_pull;
  }
  return out;
}


std::vector<double> TKinFitterDriver::GetHadronicTopMassVector(bool IsConverge){
  std::vector<double> out_vector;
  for(auto &x : fit_result_vector){
    if(IsConverge==true && x.status==0){
      out_vector.push_back(x.hadronic_top_M);
    }
    else if(IsConverge==false && x.status!=0){
      out_vector.push_back(x.hadronic_top_M);
    }
  }
  return out_vector;
}


std::vector<double> TKinFitterDriver::GetHadronicTopBPtVector(bool IsConverge){
  std::vector<double> out_vector;
  for(auto &x : fit_result_vector){
    if(IsConverge==true && x.status==0){
      out_vector.push_back(x.hadronic_top_b_pt);
    }
    else if(IsConverge==false && x.status!=0){
      out_vector.push_back(x.hadronic_top_b_pt);
    }
  }
  return out_vector;
}


std::vector<double> TKinFitterDriver::GetLeptonicTopBPtVector(bool IsConverge){
  std::vector<double> out_vector;
  for(auto &x : fit_result_vector){
    if(IsConverge==true && x.status==0){
      out_vector.push_back(x.leptonic_top_b_pt);
    }
    else if(IsConverge==false && x.status!=0){
      out_vector.push_back(x.leptonic_top_b_pt);
    }
  }
  return out_vector;
}


std::vector<double> TKinFitterDriver::GetWCHDownTypePtVector(bool IsConverge){
  std::vector<double> out_vector;
  for(auto &x : fit_result_vector){
    if(IsConverge==true && x.status==0){
      out_vector.push_back(x.w_ch_up_type_pt);
    }
    else if(IsConverge==false && x.status!=0){
      out_vector.push_back(x.w_ch_up_type_pt);
    }
  }
  return out_vector;
}


std::vector<double> TKinFitterDriver::GetWCHUpTypePtVector(bool IsConverge){
  std::vector<double> out_vector;
  for(auto &x : fit_result_vector){
    if(IsConverge==true && x.status==0){
      out_vector.push_back(x.w_ch_down_type_pt);
    }
    else if(IsConverge==false && x.status!=0){
      out_vector.push_back(x.w_ch_down_type_pt);
    }
  }
  return out_vector;
}


const std::vector<TKinFitterDriver::ResultContainer>* TKinFitterDriver::GetResults(){
  return &fit_result_vector;
}


bool TKinFitterDriver::NextPermutation(bool UseLeading4Jets){

  std::vector<TKinFitterDriver::JET_ASSIGNMENT>::iterator begin = permutation_vector.begin();
  std::vector<TKinFitterDriver::JET_ASSIGNMENT>::iterator end   = permutation_vector.end();
  if(UseLeading4Jets){
    end = begin+4;
  }
  else{
    if(end - begin > 6){ // upto 6th leading jets
      end = begin+6;
    }
    else{
      //pass
    }
  }
  return std::next_permutation(begin,end); //permutation in range of [begin,end)
}


void TKinFitterDriver::FindBestLDPermutaion(bool UseLeading4Jets){

    std::vector<TKinFitterDriver::JET_ASSIGNMENT> best_LD_permutation_vector;
    // copy vector
    best_LD_permutation_vector.clear();
    for(auto x:permutation_vector){
      best_LD_permutation_vector.push_back(x);
    }
    float best_LD = -9999;
    // jet permutation
    do{
      if(this->Check_BJet_Assignment() == false) continue;
      this->SetCurrentPermutationJets();
      float curr_LD = this->CalcLD();
      // update best LD permutation
      if(curr_LD > best_LD){
        best_LD = curr_LD;
        // copy vector
        best_LD_permutation_vector.clear();
        for(auto x:permutation_vector){
          best_LD_permutation_vector.push_back(x);
        }
      }
    }while(this->NextPermutation(UseLeading4Jets));
    // copy vector
    permutation_vector.clear();
    for(auto x:best_LD_permutation_vector){
      permutation_vector.push_back(x);
    }
}


void TKinFitterDriver::SetJetError(TMatrixD *matrix,  double Pt, double Eta, double Phi, TString flavour_key){
  (*matrix)(0,0) = TMath::Power(Pt*this->JetErrorPt(Pt, Eta, flavour_key), 2);
  //(*matrix)(1,1) = TMath::Power(Eta*this->JetErrorEta(Et, Eta, flavour_key), 2);
  //(*matrix)(2,2) = TMath::Power(Phi*this->JetErrorPhi(Et, Eta, flavour_key), 2);
}


void TKinFitterDriver::SetUnclError(TMatrixD *matrix, TLorentzVector &met){
  TLorentzVector extra_pt(0.,0.,0.,0.);
  extra_pt -= met;
  extra_pt -= lepton;
  extra_pt -= hadronic_top_b_jet;
  extra_pt -= leptonic_top_b_jet;
  extra_pt -= hadronic_w_ch_jet1;
  extra_pt -= hadronic_w_ch_jet2;

  double Pt = extra_pt.Pt();
  double Eta = 0.;
  double error_Pt = Pt*this->JetErrorPt(Pt, Eta, "udscb");
  //(*matrix)(0,0) = error_Pt*error_Pt;
  //(*matrix)(1,1) = error_Pt*error_Pt;
  (*matrix)(0,0) = error_Pt*error_Pt;
  (*matrix)(1,1) = error_Pt*error_Pt;
}


double TKinFitterDriver::JetErrorPt(double Pt, double Eta, TString flavour_key){
  return ts_correction->GetFittedError("Pt", flavour_key, Pt, Eta);
}


double TKinFitterDriver::JetErrorEta(double Pt, double Eta, TString flavour_key){
  return ts_correction->GetFittedError("Eta", flavour_key, Pt, Eta);
}


double TKinFitterDriver::JetErrorPhi(double Pt, double Eta, TString flavour_key){
  return ts_correction->GetFittedError("Phi", flavour_key, Pt, Eta);
}


void TKinFitterDriver::Sol_Neutrino_Pz(){
  
  double lepton_mass =  lepton.M();

  double k = 80.4*80.4/2.0 - lepton_mass*lepton_mass/2.0 + lepton.Px()*corr_METv.Px() + lepton.Py()*corr_METv.Py();
  double a = TMath::Power(lepton.Px(), 2.0) + TMath::Power(lepton.Py(), 2.0);
  double b = -2*k*lepton.Pz();                                                           
  double c = TMath::Power(lepton.Pt(), 2.0)*TMath::Power(corr_METv.Pt(), 2.0) - TMath::Power(k, 2.0);

  double determinant = TMath::Power(b, 2.0) - 4*a*c;
  
  //cout << "determinant = " << determinant << endl;
  

  neutrino_pz_sol.clear();
  //real soluion
  if(determinant>=0){
    double sol_1 = (-b - TMath::Sqrt(determinant))/(2*a); 
    double sol_2 = (-b + TMath::Sqrt(determinant))/(2*a);
    neutrino_pz_sol.push_back(sol_1);
    neutrino_pz_sol.push_back(sol_2);
    // more pz points
    //neutrino_pz_sol.push_back(sol_1 * 1.2);
    //neutrino_pz_sol.push_back(sol_1 * 1.4);
    //neutrino_pz_sol.push_back(sol_1 / 1.2);
    //neutrino_pz_sol.push_back(sol_1 / 1.4);
    //neutrino_pz_sol.push_back(sol_2 * 1.2);
    //neutrino_pz_sol.push_back(sol_2 * 1.4);
    //neutrino_pz_sol.push_back(sol_2 / 1.2);
    //neutrino_pz_sol.push_back(sol_2 / 1.4);
    IsRealNeuPz = true;
  }
  else{
    double sol_1 = -b/(2*a);
    neutrino_pz_sol.push_back( sol_1 );
    // more pz points
    //neutrino_pz_sol.push_back( sol_1 * 1.1);
    //neutrino_pz_sol.push_back( sol_1 * 1.2);
    //neutrino_pz_sol.push_back( sol_1 * 1.3);
    //neutrino_pz_sol.push_back( sol_1 * 1.4);
    //neutrino_pz_sol.push_back( sol_1 / 1.1);
    //neutrino_pz_sol.push_back( sol_1 / 1.2);
    //neutrino_pz_sol.push_back( sol_1 / 1.3);
    //neutrino_pz_sol.push_back( sol_1 / 1.4);
    this->Resol_Neutrino_Pt();
    IsRealNeuPz = false;
  }

}


void TKinFitterDriver::Resol_Neutrino_Pt(){

  //cout << "Kinematic_Fitter_MVA::Resol_Neutino_Pt" << endl;
    
  //recal MET
  double lepton_mass = lepton.M();
  double cosine = TMath::Cos(corr_METv.Phi());
  double sine = TMath::Sin(corr_METv.Phi());
  
  double a = lepton.E()*lepton.E() - lepton.Pz()*lepton.Pz() - TMath::Power(lepton.Px()*cosine + lepton.Py()*sine , 2.0);
  double b = (lepton.Px()*cosine + lepton.Py()*sine)*(lepton_mass*lepton_mass - 80.4*80.4);
  double determinant = TMath::Power(lepton_mass*lepton_mass - 80.4*80.4, 2.)*(lepton.E()*lepton.E() - lepton.Pz()*lepton.Pz());
  
  //cout << "a = " << a << endl;
  //cout << "b = " << b << endl;
  //cout << "det = "<< determinant << endl;

  double met_recal[2];
  met_recal[0] = (-b + TMath::Sqrt(determinant))/2./a;
  met_recal[1] = (-b - TMath::Sqrt(determinant))/2./a;
  
  double mass_diff[2];
  TLorentzVector met_recal_vector[2];
  for(Int_t i=0; i<2; i++)
    {
      met_recal_vector[i].SetPxPyPzE(met_recal[i]*cosine, met_recal[i]*sine, 0, met_recal[i]);
      
      TLorentzVector w_lnu;
      w_lnu = lepton + met_recal_vector[i];
      
      double w_lnu_mass = w_lnu.M();
      mass_diff[i] = TMath::Abs(80.4 - w_lnu_mass);
      
      //cout << corr_METv.Pt() << "\t" << met_recal[i] << "\t" << w_lnu.M() << "\t" << mass_diff[i] << endl;
    }
  //cout << endl;
  
  if(mass_diff[0]<mass_diff[1]) recal_METv = met_recal_vector[0];
  else recal_METv = met_recal_vector[1];
  
}


bool TKinFitterDriver::Chi2Comparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  return (rc1.chi2 < rc2.chi2);
}
bool TKinFitterDriver::HadTopMComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  // if chi2 is same compare hadronic_top_M
  return (fabs(rc1.hadronic_top_M-172.5) < fabs(rc2.hadronic_top_M-172.5)) ||  (fabs(rc1.hadronic_top_M-172.5) == fabs(rc2.hadronic_top_M-172.5) && (rc1.chi2 < rc2.chi2));
}

bool TKinFitterDriver::HighMassFitter(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  bool out; 
  out=(rc1.chi2 < rc2.chi2) ||
      (
       (rc1.chi2 == rc2.chi2) &&
       (rc1.hadronic_top_b_pt < rc1.w_ch_down_type_pt) &&
       (rc2.hadronic_top_b_pt >= rc2.w_ch_down_type_pt)
      );
  return out;
}


bool TKinFitterDriver::HadTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  return (rc1.hadronic_top_pt > rc2.hadronic_top_pt);
}


bool TKinFitterDriver::LepTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  return (rc1.leptonic_top_pt > rc2.leptonic_top_pt);
}


bool TKinFitterDriver::TopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  return (rc1.chi2 < 10.) && (rc1.hadronic_top_pt > rc2.hadronic_top_pt);
}
