#ifndef TSCorrection_h
#define TSCorrection_h

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "TLorentzVector.h"
#include "TMath.h"
#include "TString.h"
#include "TFormula.h"

class TSCorrection{

public:

  TSCorrection();
  TSCorrection(int DataYear_);
  ~TSCorrection();

  void ReadFittedError(TString fileName);
  void ReadFittedMean(TString fileName);
  //void ReadFittedResponse(TString fileName);

  int DataYear;
  void SetDataYear(int i);

  double GetFittedError(TString key, double x);
  double GetFittedError(TString response_key, TString flavour_key, double x, double eta);
  double GetFittedMean(TString key, double x);
  double GetFittedMean(TString response_key, TString flavour_key, double x, double eta);
  //double GetFittedResponse(TString key, double x);

  bool PassResponseCut(TString key, double x, double response, double n_sigma=2.0);


  TLorentzVector GetCorrectedJet(TString flavour_key, TLorentzVector &jet);

  class MyFormula{

  public:

    MyFormula(){}
    ~MyFormula(){}
    TFormula* GetFormulaError(double eta);
    TFormula* GetFormulaMean(double eta);
    double EvalError(double x, double eta);
    double EvalMean(double x, double eta);

    std::vector<TFormula*> formulaError;
    std::vector<TFormula*> formulaMean;
    void ClearError();
    void ClearMean();
    int FindEtaBin(double eta);

  };

private:

  std::map<TString, TFormula*> MapFittedErrorFormulaByEta;
  std::map<TString, TFormula*> MapFittedMeanFormulaByEta;
  //std::map<TString, TFormula*> MapFittedResponseFormula;

  std::map<TString, MyFormula*> MapFittedErrorMyFormula;
  std::map<TString, MyFormula*> MapFittedMeanMyFormula;

};

#endif
