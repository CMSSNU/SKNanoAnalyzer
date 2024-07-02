#include "GetEffLumi.h"
//#include "correction.h"
//using correction::CorrectionSet;

GetEffLumi::GetEffLumi() {
fChain->SetBranchStatus("*",0);
  fChain->SetBranchStatus("IsData",1);
  if(!IsDATA)
    {
      fChain->SetBranchStatus("gen_weight",1);
      if(fChain->GetBranch("weight_Scale")) fChain->SetBranchStatus("weight_Scale",1);
    }
}
GetEffLumi::~GetEffLumi() {}

void GetEffLumi::executeEvent() {
  double weight = 1.;
  double weight_sign = 1.;
  if(!IsDATA)
    {
      weight = MCweight(false,false);
      weight_sign = MCweight(true,false);
    }

  FillHist("sumW", 0, weight, 1, 0., 1.);
  FillHist("sumSign", 0, weight_sign, 1, 0., 1.);
  return;
}

