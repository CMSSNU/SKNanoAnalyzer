#include "GetEffLumi.h"
//#include "correction.h"
//using correction::CorrectionSet;
GetEffLumi::GetEffLumi(){};
GetEffLumi::~GetEffLumi(){};
void GetEffLumi::initializeAnalyzer(){
  myCorr = new MyCorrection(DataEra, IsDATA?DataStream:MCSample ,IsDATA);
  fChain->SetBranchStatus("*", 0);
  if(!IsDATA){
    fChain->SetBranchStatus("genWeight", 1);
  }
  sumW = 1.;
  sumSign = 1.;
}
void GetEffLumi::executeEvent() {
  double weight = 1.;
  double weight_sign = 1.;
  if(!IsDATA)
    {
      weight = MCweight(false,false);
      weight_sign = MCweight(true,false);
      FillHist("sumW", 0, weight, 1, 0., 1.);
      FillHist("sumSign", 0, weight_sign, 1, 0., 1.);
    }
  else{
    FillHist("NEvents", 0, 1, 1, 0., 1.);
  }


  return;
}

