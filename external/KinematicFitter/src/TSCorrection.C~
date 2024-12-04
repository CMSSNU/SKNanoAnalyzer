#include "TSCorrection.h"
TSCorrection::TSCorrection(){
}

TSCorrection::TSCorrection(int DataYear_){
  DataYear=DataYear_;
}

TSCorrection::~TSCorrection(){

  for(auto& mapit : MapFittedErrorFormulaByEta){
    delete mapit.second;
  }
  for(auto& mapit : MapFittedMeanFormulaByEta){
    delete mapit.second;
  }
  /*
  for(auto& mapit : MapFittedResponseFormula){
    delete mapit.second;
  }
  */
}

void TSCorrection::ReadFittedError(TString fileName){

  TString datapath = getenv("CMSSW_BASE");
  datapath += "/src/SNuAnalytics/data/";
  datapath = datapath+"TSCorrection/"+TString::Itoa(DataYear,10)+"/";
  std::ifstream reader(datapath + fileName);

  TString key, formula;
  while(reader >> key >> formula){
    MapFittedErrorFormulaByEta[key] = new TFormula(key,formula);
  }

}

void TSCorrection::ReadFittedMean(TString fileName){

  TString datapath = getenv("CMSSW_BASE");
  datapath += "/src/SNuAnalytics/data/";
  datapath = datapath+"TSCorrection/"+TString::Itoa(DataYear,10)+"/";
  std::ifstream reader(datapath + fileName);

  TString key, formula;
  while(reader >> key >> formula){
    MapFittedMeanFormulaByEta[key] = new TFormula(key,formula);
  }
  
}
/*
void TSCorrection::ReadFittedResponse(TString fileName){

  TString datapath = getenv("DATA_DIR");
  datapath = datapath+"/"+TString::Itoa(DataYear,10)+"/TSCorrection/";
  std::ifstream reader(datapath + fileName);

  TString key, formula;
  while(reader >> key >> formula){
    MapFittedResponseFormula[key] = new TFormula(key,formula);
  }
}
*/
void TSCorrection::SetDataYear(int i){
  DataYear = i;
}

double TSCorrection::GetFittedError(TString key, double x){
  return MapFittedErrorFormulaByEta[key]->Eval(x);
}

double TSCorrection::GetFittedError(TString response_key, TString flavour_key, double x, double eta){

  TString key = Form("%s_%s_",response_key.Data(),flavour_key.Data());
  auto mapit = MapFittedErrorMyFormula.find(key);
  if(mapit == MapFittedErrorMyFormula.end()){
    MapFittedErrorMyFormula[key] = new TSCorrection::MyFormula();
    mapit = MapFittedErrorMyFormula.find(key);
    mapit->second->ClearError();
    for(auto& x : MapFittedErrorFormulaByEta){
      if(x.first.Contains(key)){
        mapit->second->formulaError.push_back(x.second);
      }
    }
    if(mapit->second->formulaError.size()!=9){
      std::cout << "Error: TSCorrection::GetFittedError"<< std::endl;
      exit(EXIT_FAILURE);
    }
  }
  //debug
  //std::cout << "Test: TSCorrection::GetFittedError"<< std::endl;
  //std::cout << mapit->second->EvalError(x, eta) << std::endl;
  return std::max(0.01, mapit->second->EvalError(x, eta));
}

double TSCorrection::GetFittedMean(TString key, double x){
  return MapFittedMeanFormulaByEta[key]->Eval(x);
}

double TSCorrection::GetFittedMean(TString response_key, TString flavour_key, double x, double eta){

  TString key = Form("%s_%s_",response_key.Data(),flavour_key.Data());
  auto mapit = MapFittedMeanMyFormula.find(key);
  if(mapit == MapFittedMeanMyFormula.end()){
    MapFittedMeanMyFormula[key] = new TSCorrection::MyFormula();
    mapit = MapFittedMeanMyFormula.find(key);
    mapit->second->ClearMean();
    for(auto& x : MapFittedMeanFormulaByEta){
      if(x.first.Contains(key)){
        mapit->second->formulaMean.push_back(x.second);
      }
    }
    if(mapit->second->formulaMean.size()!=9){
      std::cout << "Mean: TSCorrection::GetFittedMean"<< std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return mapit->second->EvalMean(x, eta);
}
/*
double TSCorrection::GetFittedResponse(TString key, double x){
  return MapFittedResponseFormula[key]->Eval(x);
}
*/
bool TSCorrection::PassResponseCut(TString key, double x, double response, double n_sigma){

  double mean = this->GetFittedMean(key, x);
  double sigma = this->GetFittedError(key, x);
  double max = mean + n_sigma*sigma;
  double min = mean - n_sigma*sigma;

  return (response > min && response < max);
}

TLorentzVector TSCorrection::GetCorrectedJet(TString flavour_key, TLorentzVector &jet){
  double Pt = jet.Pt();
  double Eta = jet.Eta();
  double Phi = jet.Phi();
  double M = jet.M();
  double corr = this->GetFittedMean("Pt", flavour_key, Pt, Eta);
  TLorentzVector out_vector;
  double corr_Pt = Pt*(corr+1);
  out_vector.SetPtEtaPhiM(corr_Pt, Eta, Phi, M );
  return out_vector;
}


TFormula* TSCorrection::MyFormula::GetFormulaError(double eta){
  return formulaError.at(FindEtaBin(eta));
}

TFormula* TSCorrection::MyFormula::GetFormulaMean(double eta){
  return formulaMean.at(FindEtaBin(eta));
}

double TSCorrection::MyFormula::EvalError(double x, double eta){
  return formulaError.at(FindEtaBin(eta))->Eval(x);
}

double TSCorrection::MyFormula::EvalMean(double x, double eta){
  return formulaMean.at(FindEtaBin(eta))->Eval(x);
}

void TSCorrection::MyFormula::ClearError(){
  for(auto& x : formulaError){
    delete x;
  }
  formulaError.clear();
}

void TSCorrection::MyFormula::ClearMean(){
  for(auto& x : formulaMean){
    delete x;
  }
  formulaMean.clear();
}

int TSCorrection::MyFormula::FindEtaBin(double eta){

 int i=-1;

 if(fabs(eta)<0.174) i=0;
 else if(fabs(eta)<0.348) i=1;
 else if(fabs(eta)<0.522) i=2;
 else if(fabs(eta)<0.696) i=3;
 else if(fabs(eta)<0.879) i=4;
 else if(fabs(eta)<1.131) i=5;
 else if(fabs(eta)<1.392) i=6;
 else if(fabs(eta)<1.74) i=7;
 else if(fabs(eta)<2.4) i=8;
 else{
  //std::cout << "WARNING eta>=2.4 : "<< eta << std::endl;
  i=8;
  //exit(EXIT_FAILURE);
 }

 return i;

}

