#include "TKinFitterDriver.h"

TKinFitterDriver::TKinFitterDriver(){

}


TKinFitterDriver::TKinFitterDriver(int DataYear_){

  DataYear = DataYear_;

  fitter = new TKinFitter("fitter","fitter");

  error_hadronic_top_b_jet.ResizeTo(1,1); 
  error_leptonic_top_b_jet.ResizeTo(1,1);
  error_hadronic_w_ch_jet1.ResizeTo(1,1);
  error_hadronic_w_ch_jet2.ResizeTo(1,1);
  error_lepton.ResizeTo(1,1);
  error_neutrino_pxpy.ResizeTo(2,2);

  error_hadronic_top_b_jet.Zero(); 
  error_leptonic_top_b_jet.Zero();
  error_hadronic_w_ch_jet1.Zero();
  error_hadronic_w_ch_jet2.Zero();
  error_lepton.Zero();
  error_neutrino_pxpy.Zero();

  ts_correction = new TSCorrection(DataYear);
  ts_correction->ReadFittedError("fit_error_pythia.txt");
  ts_correction->ReadFittedMean("fit_mean_pythia.txt");

  fit_hadronic_top_b_jet = new TFitParticlePt();
  fit_leptonic_top_b_jet = new TFitParticlePt();
  fit_hadronic_w_ch_jet1 = new TFitParticlePt();
  fit_hadronic_w_ch_jet2 = new TFitParticlePt();
  //fit_lepton = new TFitParticlePt();
  fit_neutrino_pxpy =  new TFitParticlePxPy();
  //fit_neutrino_pz =  new TFitParticlePz();

  constrain_hadronic_top_M = new TFitConstraintM("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, 172.5);
  //constrain_hadronic_top_MGaus = new TFitConstraintMGaus("hadronic_top_mass_constraint", "hadronic_top_mass_constraint", 0, 0, 172.5, 1.5);
  constrain_leptonic_top_M = new TFitConstraintM("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, 172.5);
  //constrain_leptonic_top_MGaus = new TFitConstraintMGaus("leptonic_top_mass_constraint", "leptonic_top_mass_constraint", 0, 0, 172.5, 1.5);
  //constrain_leptonic_W_M = new TFitConstraintM("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4);
  //constrain_leptonic_W_MGaus = new TFitConstraintMGaus("leptonic_w_mass_constraint", "leptonic_w_mass_constraint", 0, 0, 80.4, 2.085);
  //cout <<"TKinFitterDriver::TKinFitterDriver : initialized" << endl;
}


TKinFitterDriver::~TKinFitterDriver(){
  delete fitter;

  delete fit_hadronic_top_b_jet;
  delete fit_leptonic_top_b_jet;
  delete fit_hadronic_w_ch_jet1;
  delete fit_hadronic_w_ch_jet2;
  //delete fit_lepton;
  delete fit_neutrino_pxpy;
  //delete fit_neutrino_pz;

  delete constrain_hadronic_top_M;
  //delete constrain_hadronic_top_MGaus;
  delete constrain_leptonic_top_M;
  //delete constrain_leptonic_top_MGaus;
  //delete constrain_leptonic_W_M;
  //delete constrain_leptonic_W_MGaus;
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


void TKinFitterDriver::SetJetPtResolution(std::vector<float> jetPtResolution_){
  jet_pt_resolution_vector.clear();
  for(auto& x : jetPtResolution_){
    jet_pt_resolution_vector.push_back(x);
  }
}

void TKinFitterDriver::SetHadronicTopBJets(TLorentzVector jet_){
  hadronic_top_b_jet = jet_;
  double Pt = hadronic_top_b_jet.Pt();
  double Eta = hadronic_top_b_jet.Eta();
  double Phi = hadronic_top_b_jet.Phi();
  this->SetJetError(&error_hadronic_top_b_jet, Pt, Eta, Phi, "b");
  corr_hadronic_top_b_jet = ts_correction->GetCorrectedJet("b",hadronic_top_b_jet);
  fit_hadronic_top_b_jet->~TFitParticlePt();
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
  corr_leptonic_top_b_jet = ts_correction->GetCorrectedJet("b",leptonic_top_b_jet) + lepton + neutrino_pz;
  fit_leptonic_top_b_jet->~TFitParticlePt();
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
  fit_hadronic_w_ch_jet1->~TFitParticlePt();
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
  fit_hadronic_w_ch_jet2->~TFitParticlePt();
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


void TKinFitterDriver::SetLeptonicTopBJets(TLorentzVector jet_, double resolution_){
  leptonic_top_b_jet=jet_;
  double Pt = leptonic_top_b_jet.Pt();
  error_leptonic_top_b_jet(0,0) = resolution_*resolution_*Pt*Pt;
  //XXX when we vary only leptonic top b jet pt
  //corr_leptonic_top_b_jet = leptonic_top_b_jet + lepton + neutrino_pxpy + neutrino_pz;
  //XXX when we vary only leptonic top b jet pt and neutrino_pxpy
  corr_leptonic_top_b_jet = leptonic_top_b_jet + lepton + neutrino_pz;

  fit_leptonic_top_b_jet->~TFitParticlePt();
  new(fit_leptonic_top_b_jet) TFitParticlePt("leptonic_top_b_jet",
                                                    "leptonic_top_b_jet",
                                                    &corr_leptonic_top_b_jet,
                                                    &error_leptonic_top_b_jet
                                                   );
  //cout <<"TKinFitterDriver::SetLeptonicTopBJets : " << endl;
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


void TKinFitterDriver::SetLepton(TLorentzVector lepton_){
  lepton=lepton_;
  //double Pt = lepton.Pt();
  //double Eta = lepton.Eta();
  //this->SetError(&error_lepton, Pt, Eta);
  //error_lepton(0,0)=TMath::Power(0.01*Pt,2);
  //error_lepton(1,1)=TMath::Power(0.03*Eta,2);
  //error_lepton(2,2)=TMath::Power(0.03*lepton.Phi(),2);
  //fit_lepton->~TFitParticlePt();
  //new(fit_lepton) TFitParticlePt("lepton",
  //                                      "lepton",
  //                                      &lepton,
  //                                      &error_lepton
  //                                     );
  //cout << "TKinFitterDriver::SetLepton : " << endl;
}


void TKinFitterDriver::SetMET(TLorentzVector met_){
  METv = met_;
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

  double Pz = neutrino_pz_sol[i];
  neutrino_pxpy.SetPxPyPzE(met_.Px(),met_.Py(), 0., TMath::Sqrt(met_.E()*met_.E()+Pz*Pz));
  neutrino_pz.SetPxPyPzE(0., 0., Pz, 0.); //XXX Set energy as ZERO!!!

  double MET_error_px2=0., MET_error_py2=0.;
  for(unsigned int i(0); i<jet_pt_resolution_vector.size(); i++){
    double jet_pt_resolution = jet_pt_resolution_vector.at(i);
    double jet_pt  = jet_vector.at(i).Pt();
    double jet_phi = jet_vector.at(i).Phi();
    MET_error_px2 += TMath::Power(jet_pt_resolution*jet_pt*TMath::Cos(jet_phi), 2);
    MET_error_py2 += TMath::Power(jet_pt_resolution*jet_pt*TMath::Sin(jet_phi), 2);
  }

  error_neutrino_pxpy(0,0) = std::max(0.0001, MET_error_px2);  
  error_neutrino_pxpy(1,1) = std::max(0.0001, MET_error_py2);
  //error_neutrino_pxpy(0,0) = std::max(0.0001, MET_pt_shift*MET_pt_shift);  //XXX pt->px, phi->py
  //error_neutrino_pxpy(1,1) = std::max(0.0001, MET_phi_shift*MET_phi_shift); //XXX pt->px, phi->py
  fit_neutrino_pxpy->~TFitParticlePxPy();
  new(fit_neutrino_pxpy) TFitParticlePxPy("neutrino_pxpy",
                                          "neutrino_pxpy",
                                          &neutrino_pxpy,
                                          &error_neutrino_pxpy
                                         );
  /*
  fit_neutrino_pz->~TFitParticlePz();
  new(fit_neutrino_pz) TFitParticlePz("neutrino_pz",
		                      "neutrino_pz",
				      &neutrino_pz,
				      NULL
				      );
  */
  //cout << "TKinFitterDriver::SetNeutrino : " << endl;
}


void TKinFitterDriver::SetNeutrinoSmallerPz(TLorentzVector met_){
  this->SetMET(met_);
  this->Sol_Neutrino_Pz();
  if(IsRealNeuPz){
    if(fabs(neutrino_pz_sol[0]) < fabs(neutrino_pz_sol[1])){
      this->SetNeutrino(METv, 0);
    }
    else{
      this->SetNeutrino(METv, 1);
    }
  }
}

void TKinFitterDriver::SetCurrentPermutationJets(){

  hadronic_top_b_jet_idx=-1;
  leptonic_top_b_jet_idx=-1;
  hadronic_w_ch_jet1_idx=-1;
  hadronic_w_ch_jet2_idx=-1;

  for(UInt_t i=0; i<permutation_vector.size(); i++){
    JET_ASSIGNMENT jet_assignment_idx = permutation_vector.at(i);
    if( jet_assignment_idx == HADRONIC_TOP_B ) hadronic_top_b_jet_idx=i;
    else if( jet_assignment_idx == LEPTONIC_TOP_B ) leptonic_top_b_jet_idx=i;
    else if( jet_assignment_idx == W_CH_UP_TYPE ) hadronic_w_ch_jet1_idx=i;
    else if( jet_assignment_idx == W_CH_DOWN_TYPE ) hadronic_w_ch_jet2_idx=i;
  }
  //cout << k << l << m << n << endl;
  // using ts-correction
  //this->SetHadronicTopBJets( jet_vector.at(k) );
  //this->SetLeptonicTopBJets( jet_vector.at(l) );
  //this->SetWCHUpTypeJets( jet_vector.at(m) );
  //this->SetWCHDownTypeJets( jet_vector.at(n) );

  // using jetMET POG jer
  this->SetHadronicTopBJets( jet_vector.at(hadronic_top_b_jet_idx), jet_pt_resolution_vector.at(hadronic_top_b_jet_idx) );
  this->SetLeptonicTopBJets( jet_vector.at(leptonic_top_b_jet_idx), jet_pt_resolution_vector.at(leptonic_top_b_jet_idx) );
  this->SetWCHUpTypeJets( jet_vector.at(hadronic_w_ch_jet1_idx), jet_pt_resolution_vector.at(hadronic_w_ch_jet1_idx) );
  this->SetWCHDownTypeJets( jet_vector.at(hadronic_w_ch_jet2_idx), jet_pt_resolution_vector.at(hadronic_w_ch_jet2_idx) );

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
    if(btag_csv_vector.at(w_ch_up_type_jet_idx) <= btag_csv_vector.at(w_ch_down_type_jet_idx)){
      true_bjet_assignment=true;
    }
    else{
      //pass
    }
  }

  //cout << "TKinFitterDriver::Check_BJet_Assignment : " << endl; 
  return true_bjet_assignment;
}


bool TKinFitterDriver::Kinematic_Cut(){
  TLorentzVector hadronic_top = hadronic_top_b_jet + hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
  TLorentzVector leptonic_top = leptonic_top_b_jet + lepton + neutrino_pxpy + neutrino_pz;
  double hadronic_top_mass = hadronic_top.M();
  double leptonic_top_mass = leptonic_top.M();
  return (hadronic_top_mass > 100 && hadronic_top_mass < 240) &&
         (leptonic_top_b_jet+lepton).M() < 150 &&
         (fabs(hadronic_top.Phi()-leptonic_top.Phi()) > 1.5);
}


void TKinFitterDriver::SetConstraint(){
  //TODO: will update to be able to set top-mass
  // reset constrain
  constrain_hadronic_top_M->Clear();
  constrain_hadronic_top_M->addParticles1(fit_hadronic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  //constrain_hadronic_top_MGaus->Clear();
  //constrain_hadronic_top_MGaus->addParticles1(fit_hadronic_top_b_jet, fit_hadronic_w_ch_jet1, fit_hadronic_w_ch_jet2);
  constrain_leptonic_top_M->Clear();
  constrain_leptonic_top_M->addParticles1(fit_leptonic_top_b_jet, fit_neutrino_pxpy);
  //constrain_leptonic_top_MGaus->Clear();
  //constrain_leptonic_top_MGaus->addParticles1(fit_leptonic_top_b_jet, fit_lepton, fit_neutrino_pxpy, fit_neutrino_pz);
  //constrain_leptonic_W_M->Clear();
  //constrain_leptonic_W_M->addParticles1(fit_lepton, fit_neutrino_pxpy);
  //constrain_leptonic_W_MGaus->Clear();
  //constrain_leptonic_W_MGaus->addParticles1(fit_lepton, fit_neutrino_pxpy, fit_neutrino_pz);
}


void TKinFitterDriver::SetFitter(){
  fitter->reset();
  //add MeasParticles to vary Et,Eta,Phi
  fitter->addMeasParticle( fit_hadronic_top_b_jet );
  fitter->addMeasParticle( fit_leptonic_top_b_jet );
  fitter->addMeasParticle( fit_hadronic_w_ch_jet1 );
  fitter->addMeasParticle( fit_hadronic_w_ch_jet2 );
  //add UnmeasParticles not to vary Et,Eta,Phi
  // Also, Px, Py should be constrained
  //fitter->addMeasParticle( fit_lepton );
  fitter->addMeasParticle( fit_neutrino_pxpy );
  //fitter->addUnmeasParticle( fit_neutrino_pz );
  //fitter->addUnmeasParticle( fit_lepton );
  //fitter->addUnmeasParticle( fit_neutrino_pxpy );
  //add Constraint
  fitter->addConstraint( constrain_hadronic_top_M );
  //fitter->addConstraint( constrain_hadronic_top_MGaus );
  fitter->addConstraint( constrain_leptonic_top_M );
  //fitter->addConstraint( constrain_leptonic_top_MGaus );
  //fitter->addConstraint( constrain_leptonic_W_M );
  //fitter->addConstraint( constrain_leptonic_W_MGaus );
  //Set convergence criteria
  fitter->setMaxNbIter( 50 ); //50 is default
  fitter->setMaxDeltaS( 1e-2 );
  fitter->setMaxF( 1e-1 ); //1e-1 is default
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
  TLorentzVector leptonic_W = lepton + neutrino_pxpy + neutrino_pz;
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
  fit_result.hadronic_w_ch_jet1_idx = hadronic_w_ch_jet1_idx;
  fit_result.hadronic_w_ch_jet2_idx = hadronic_w_ch_jet2_idx;

  fit_result.hadronic_top_b_jet_pull = (*fit_hadronic_top_b_jet->getPull())(0,0);
  fit_result.leptonic_top_b_jet_pull = (*fit_leptonic_top_b_jet->getPull())(0,0);
  fit_result.hadronic_w_ch_jet1_pull = (*fit_hadronic_w_ch_jet1->getPull())(0,0);
  fit_result.hadronic_w_ch_jet2_pull = (*fit_hadronic_w_ch_jet2->getPull())(0,0);
                                      
  //
  ///////
  int is_w_ch_down_type_jet_b_tagged = 0;
  for(UInt_t i=0; i<permutation_vector.size(); i++){
    JET_ASSIGNMENT jet_assignment_idx = permutation_vector.at(i);
    bool IsBTagged = btag_vector.at(i);
    if( jet_assignment_idx == W_CH_DOWN_TYPE){
      if(IsBTagged){
        is_w_ch_down_type_jet_b_tagged=1;
      }
      else{
        //pass
      }
      break;
    }
  }
  fit_result.is_w_ch_down_type_jet_b_tagged = is_w_ch_down_type_jet_b_tagged;
  ///////
  //
  fit_result.chi2 = 9999;
  //if(fit_result.status!=-10){ // -10 means singular matrix
    // save S and F
    fit_result.currS = fitter->getS();
    fit_result.deltaS = fitter->getDeltaS();
    fit_result.hadronic_top_mass_F = constrain_hadronic_top_M->getCurrentValue();
    //fit_result.hadronic_top_mass_F = constrain_hadronic_top_MGaus->getCurrentValue();
    fit_result.leptonic_top_mass_F = constrain_leptonic_top_M->getCurrentValue();
    //fit_result.leptonic_top_mass_F = constrain_leptonic_top_MGaus->getCurrentValue();
    //fit_result.leptonic_w_mass_F = constrain_leptonic_W_M->getCurrentValue();
    //fit_result.leptonic_w_mass_F = constrain_leptonic_W_MGaus->getCurrentValue();
    // re-calculate chi2,
    //chi2 term comes from constraint is not accurate
    fit_result.chi2 = this->CalcChi2();
    fit_result.lambda = fitter->getLambda();

    // calc. dijet mass
    const TLorentzVector *fitted_jet1 = fit_hadronic_w_ch_jet1->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_jet2 = fit_hadronic_w_ch_jet2->getCurr4Vec(); // get address of fitted object
    const TLorentzVector *fitted_jet3 = fit_hadronic_top_b_jet->getCurr4Vec(); // get address of fitted object
    const TLorentzVector fitted_dijet = (*fitted_jet1) + (*fitted_jet2);
    const TLorentzVector fitted_dijet_high = fitted_jet3->Pt()>fitted_jet2->Pt() ? (*fitted_jet1) + (*fitted_jet3):fitted_dijet;
    fit_result.fitted_dijet_M = fitted_dijet.M(); // save dijet mass
    fit_result.fitted_dijet_M_high = fitted_dijet_high.M(); // save dijet mass
    TLorentzVector initial_dijet = hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
    TLorentzVector initial_dijet_high = fitted_jet3->Pt()>fitted_jet2->Pt() ? hadronic_w_ch_jet1 + hadronic_top_b_jet:initial_dijet;
    fit_result.initial_dijet_M = initial_dijet.M(); // save dijet mass
    fit_result.initial_dijet_M_high = initial_dijet_high.M(); // save dijet mass
    TLorentzVector corrected_dijet = corr_hadronic_w_ch_jet1 + corr_hadronic_w_ch_jet2;
    TLorentzVector corrected_dijet_high = fitted_jet3->Pt()>fitted_jet2->Pt() ? corr_hadronic_w_ch_jet1 + corr_hadronic_top_b_jet:corrected_dijet;
    fit_result.corrected_dijet_M = corrected_dijet.M(); // save dijet mass
    fit_result.corrected_dijet_M_high = corrected_dijet_high.M(); // save dijet mass
  //}
  //cout << "TKinFitterDriver::Fit : " << endl;

}


double TKinFitterDriver::CalcChi2(){
  double chi2=0;
  // jets
  chi2 += this->CalcEachChi2(fit_hadronic_top_b_jet);
  chi2 += this->CalcEachChi2(fit_leptonic_top_b_jet);
  chi2 += this->CalcEachChi2(fit_hadronic_w_ch_jet1);
  chi2 += this->CalcEachChi2(fit_hadronic_w_ch_jet2);
  //lepton
  //chi2 += this->CalcEachChi2(fit_lepton);
  //extra pt
  chi2 += this->CalcEachChi2(fit_neutrino_pxpy);
  // mass constraints
  //chi2 += this->CalcEachChi2(constrain_leptonic_W_M, 2.085);
  //chi2 += this->CalcEachChi2(constrain_leptonic_W_MGaus);
  //chi2 += this->CalcEachChi2(constrain_hadronic_top_MGaus);
  chi2 += this->CalcEachChi2(constrain_hadronic_top_M, 1.5);
  //chi2 += this->CalcEachChi2(constrain_leptonic_top_MGaus);
  chi2 += this->CalcEachChi2(constrain_leptonic_top_M, 1.5);
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
  }
  return S;
}


double TKinFitterDriver::CalcEachChi2(TFitConstraintM* ptr, double width){
  double S = 0.;
  double deltaY = ptr->getCurrentValue();
  S = (deltaY*deltaY)/(width*width);
  return S;
}


double TKinFitterDriver::CalcEachChi2(TFitConstraintMGaus* ptr){
  return ptr->getChi2();
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

  do{
      if(this->Check_BJet_Assignment() == false) continue;
      this->SetCurrentPermutationJets();
      this->Sol_Neutrino_Pz();
  	  if(IsRealNeuPz){
        if(fabs(neutrino_pz_sol[0]) < fabs(neutrino_pz_sol[1])){
  	      this->SetNeutrino(METv, 0);
        }
        else{
  	      this->SetNeutrino(METv, 1);
        }
  	  }
  	  else{
  	    this->SetNeutrino(METv, 0);
  	  }
      if(this->Kinematic_Cut() == false) continue;
      this->Fit();
  	  fit_result_vector.push_back(fit_result);
    }while(this->NextPermutation(UseLeading4Jets));
  std::sort(fit_result_vector.begin(), fit_result_vector.end(), Chi2Comparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), HadTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), LepTopPtComparing);
  //std::sort(fit_result_vector.begin(), fit_result_vector.end(), TopPtComparing);

  if(IsHighMassFitter){
    std::sort(fit_result_vector.begin(), fit_result_vector.end(), HighMassFitter);
  }

}

void TKinFitterDriver::FindMaxPtHadTopFit(bool IsMaxLepTopPt, bool IsClosestHadTopM, bool IsClosestLepTopM){
  fit_result_vector.clear();
  fit_result_vector.shrink_to_fit();
  // status -1 : fit not performed
  fit_result.status=-1;
  int maxHadTopPt_w_ch_up_type_jet_idx = -1;
  int maxHadTopPt_w_ch_down_type_jet_idx = -1;
  int maxHadTopPt_top_b_jet_idx = -1;
  int closestHadTopM_w_ch_up_type_jet_idx = -1;
  int closestHadTopM_w_ch_down_type_jet_idx = -1;
  int closestHadTopM_top_b_jet_idx = -1;

  int maxLepTopPt_top_b_jet_idx = -1;
  int closestLepTopM_top_b_jet_idx = -1;

  double maxHadTopPt=-1, closestHadTopM=-1;
  double maxLepTopPt=-1, closestLepTopM=-1;

  if(njets<4 || nbtags<2){
    return;
  }

  do{
      if(this->Check_BJet_Assignment() == false) continue;
      this->SetCurrentPermutationJets();
      this->Sol_Neutrino_Pz();
  	  if(IsRealNeuPz){
        if(fabs(neutrino_pz_sol[0]) < fabs(neutrino_pz_sol[1])){
  	      this->SetNeutrino(METv, 0);
        }
        else{
  	      this->SetNeutrino(METv, 1);
        }
  	  }
  	  else{
  	    this->SetNeutrino(METv, 0);
  	  }
      if(this->Kinematic_Cut() == false) continue;

       TLorentzVector hadronic_top = hadronic_top_b_jet + hadronic_w_ch_jet1 + hadronic_w_ch_jet2;
       TLorentzVector leptonic_top = leptonic_top_b_jet + lepton + neutrino_pxpy + neutrino_pz;
       double curHadTopPt=hadronic_top.Pt();
       double curHadTopM=hadronic_top.M();
       double curLepTopPt=leptonic_top.Pt();
       double curLepTopM=leptonic_top.M();
       if(curHadTopPt > maxHadTopPt){
         maxHadTopPt = curHadTopPt;
         maxHadTopPt_w_ch_up_type_jet_idx   = hadronic_w_ch_jet1_idx;
         maxHadTopPt_w_ch_down_type_jet_idx = hadronic_w_ch_jet2_idx;
         maxHadTopPt_top_b_jet_idx          = hadronic_top_b_jet_idx;
       }
       if( fabs(curHadTopM-172.5) < fabs(closestHadTopM-172.5)){
         closestHadTopM = curHadTopM;
         closestHadTopM_w_ch_up_type_jet_idx   = hadronic_w_ch_jet1_idx;
         closestHadTopM_w_ch_down_type_jet_idx = hadronic_w_ch_jet2_idx;
         closestHadTopM_top_b_jet_idx          = hadronic_top_b_jet_idx;

       }
       if(curLepTopPt > maxLepTopPt){
         maxLepTopPt = curLepTopPt;
         maxLepTopPt_top_b_jet_idx = leptonic_top_b_jet_idx;
       }
       if( fabs(curLepTopM-172.5) < fabs(closestLepTopM-172.5)){
         closestLepTopM = curLepTopM;
         closestLepTopM_top_b_jet_idx = leptonic_top_b_jet_idx;
       }

    }while(this->NextPermutation(false));
    //bool maxLepTopPt, bool closestHadTopM, bool closestLepTopM
    bool isNoUmbiguity = true;
    if(IsMaxLepTopPt){
      if(maxLepTopPt_top_b_jet_idx == maxHadTopPt_w_ch_up_type_jet_idx   ||
         maxLepTopPt_top_b_jet_idx == maxHadTopPt_w_ch_down_type_jet_idx ||
         maxLepTopPt_top_b_jet_idx == maxHadTopPt_top_b_jet_idx         
        ){
           isNoUmbiguity = false;
         }
    }
    else if(IsClosestLepTopM){
      if(closestLepTopM_top_b_jet_idx == maxHadTopPt_w_ch_up_type_jet_idx   ||
         closestLepTopM_top_b_jet_idx == maxHadTopPt_w_ch_down_type_jet_idx ||
         closestLepTopM_top_b_jet_idx == maxHadTopPt_top_b_jet_idx         
        ){
           isNoUmbiguity = false;
         }
    }


    if(IsClosestHadTopM){
      if(maxHadTopPt_w_ch_up_type_jet_idx   != closestHadTopM_w_ch_up_type_jet_idx  &&
         maxHadTopPt_w_ch_down_type_jet_idx != closestHadTopM_w_ch_down_type_jet_idx &&
         maxHadTopPt_top_b_jet_idx          != closestHadTopM_top_b_jet_idx          
        ){
           isNoUmbiguity = false;
         }
    }

    if(!IsMaxLepTopPt){
      maxLepTopPt_top_b_jet_idx = closestLepTopM_top_b_jet_idx;
    }
    else if(IsClosestLepTopM){
      if(maxLepTopPt_top_b_jet_idx != closestLepTopM_top_b_jet_idx){
        isNoUmbiguity = false;
      }
    }

    if(isNoUmbiguity == true){
      if(maxHadTopPt_top_b_jet_idx != -1 &&
         maxLepTopPt_top_b_jet_idx != -1 &&
         maxHadTopPt_w_ch_up_type_jet_idx != -1 &&
         maxHadTopPt_w_ch_down_type_jet_idx != -1
        ){
          this->SetHadronicTopBJets( jet_vector.at(maxHadTopPt_top_b_jet_idx), jet_pt_resolution_vector.at(maxHadTopPt_top_b_jet_idx) );
          this->SetLeptonicTopBJets( jet_vector.at(maxLepTopPt_top_b_jet_idx), jet_pt_resolution_vector.at(maxLepTopPt_top_b_jet_idx) );
          this->SetWCHUpTypeJets( jet_vector.at(maxHadTopPt_w_ch_up_type_jet_idx), jet_pt_resolution_vector.at(maxHadTopPt_w_ch_up_type_jet_idx) );
          this->SetWCHDownTypeJets( jet_vector.at(maxHadTopPt_w_ch_down_type_jet_idx), jet_pt_resolution_vector.at(maxHadTopPt_w_ch_down_type_jet_idx) );
   
          this->Fit();
          bool IsBTagged = btag_vector.at(maxHadTopPt_w_ch_down_type_jet_idx);
          fit_result.is_w_ch_down_type_jet_b_tagged = IsBTagged?1:0;
  	      fit_result_vector.push_back(fit_result);
         }
    }
    else{
      return;
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

double TKinFitterDriver::GetBestLambda(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).lambda;
  }
  return out==9999 ? -1. : out;
}

int TKinFitterDriver::GetBestDownTypeJetBTagged(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).is_w_ch_down_type_jet_b_tagged;
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


double TKinFitterDriver::GetBestHadronicTopMassF(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_top_mass_F;
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
    out =fit_result_vector.at(0).hadronic_w_ch_jet1_idx;
  }
  return out;
}

int TKinFitterDriver::GetBestHadronicWCHDownTypeJetIdx(){
  int out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_w_ch_jet2_idx;
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
    out =fit_result_vector.at(0).hadronic_w_ch_jet1_pull;
  }
  return out;
}
double TKinFitterDriver::GetBestHadronicWCHDowntypeJetIdxPull(){
  double out=-1;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).hadronic_w_ch_jet2_pull;
  }
  return out;
}





double TKinFitterDriver::GetBestLeptonicTopMassF(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_top_mass_F;
  }
  return out;
}


double TKinFitterDriver::GetBestLeptonicWMassF(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).leptonic_w_mass_F;
  }
  return out;
}


double TKinFitterDriver::GetBestDeltaS(){
  double out=-100;
  if(fit_result_vector.size()>0){
    out =fit_result_vector.at(0).deltaS;
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


std::vector<TKinFitterDriver::ResultContainer> TKinFitterDriver::GetResults(){
  return fit_result_vector;
}


bool TKinFitterDriver::NextPermutation(bool UseLeading4Jets){

  std::vector<TKinFitterDriver::JET_ASSIGNMENT>::iterator begin = permutation_vector.begin();
  std::vector<TKinFitterDriver::JET_ASSIGNMENT>::iterator end;
  if(UseLeading4Jets){
    end = begin+4;
  }
  else{
    end = permutation_vector.end();
  }
  return std::next_permutation(begin,end); //permutation in range of [begin,end)
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

  double k = 80.4*80.4/2.0 - lepton_mass*lepton_mass/2.0 + lepton.Px()*METv.Px() + lepton.Py()*METv.Py();
  double a = TMath::Power(lepton.Px(), 2.0) + TMath::Power(lepton.Py(), 2.0);
  double b = -2*k*lepton.Pz();                                                           
  double c = TMath::Power(lepton.Pt(), 2.0)*TMath::Power(METv.Pt(), 2.0) - TMath::Power(k, 2.0);

  double determinant = TMath::Power(b, 2.0) - 4*a*c;
  
  //cout << "determinant = " << determinant << endl;
  
  //real soluion
  if(determinant>=0){
    neutrino_pz_sol[0] = (-b + TMath::Sqrt(determinant))/(2*a);                      
    neutrino_pz_sol[1] = (-b - TMath::Sqrt(determinant))/(2*a);                      
    IsRealNeuPz = true;
  }
  else{
    neutrino_pz_sol[0] = -b/(2*a);
    this->Resol_Neutrino_Pt();
    IsRealNeuPz = false;
  }

}


void TKinFitterDriver::Resol_Neutrino_Pt(){

  //cout << "Kinematic_Fitter_MVA::Resol_Neutino_Pt" << endl;
    
  //recal MET
  double lepton_mass = lepton.M();
  double cosine = TMath::Cos(METv.Phi());
  double sine = TMath::Sin(METv.Phi());
  
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
      
      //cout << METv.Pt() << "\t" << met_recal[i] << "\t" << w_lnu.M() << "\t" << mass_diff[i] << endl;
    }
  //cout << endl;
  
  if(mass_diff[0]<mass_diff[1]) recal_METv = met_recal_vector[0];
  else recal_METv = met_recal_vector[1];
  
}


bool TKinFitterDriver::Chi2Comparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2){
  return (rc1.chi2 < rc2.chi2);
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
