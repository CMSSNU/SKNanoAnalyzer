#ifndef Event_h
#define Event_h

#include "Particle.h"
#include "TString.h"
#include "TObject.h"
#include "MyCorrection.h"
using namespace std;
#include <map>

class Event: public TObject {
public:
    Event();
    ~Event();
    
    enum class MET_Syst{
        CENTRAL,
        UE,
        JER,
        JES
    };

    enum class MET_Type{
        PUPPI,
        PF
    };

    void SetnPV(int nPV) { j_nPV = nPV;}
    inline int nPV() const { return j_nPV; }
    void SetnPileUp(int nPU) { j_nPV = nPU;}
    inline int nPileUp() const { return j_nPV; }
    void SetnTrueInt(float nTrueInt) { j_nTrueInt = nTrueInt;}
    inline float nTrueInt() const { return j_nTrueInt; }
    void SetnPVsGood(int nPVsGood) { j_nPVsGood = nPVsGood;}
    inline int nPVsGood() const { return j_nPVsGood; }
    const void SetTrigger(const std::map<TString, pair<Bool_t *, float>>& TriggerMap);
    void setRho(float rho) { j_fixedGridRhoFastjetAll = rho; }
    inline float GetRho() const { return j_fixedGridRhoFastjetAll; }
    //void SetTrigger(RVec<TString> HLT_TriggerName) { j_HLT_TriggerName = HLT_TriggerName; }
    //void SetTrigger(TString HLT_TriggerName) { j_HLT_TriggerName = {HLT_TriggerName}; }

    const bool PassTrigger(TString trig);
    //bool PassTrigger(RVec<TString> trigs);
    float GetTriggerLumi(TString trig);
    bool IsPDForTrigger(TString trig, TString PD);

    void SetMET(RVec<float> MET_pt, RVec<float> MET_phi);
    Particle GetMETVector(Event::MET_Type MET_type, MyCorrection::variation syst = MyCorrection::variation::nom, Event::MET_Syst source = MET_Syst::CENTRAL) const;

    void SetEra(TString era) {
        j_DataEra = era;
        j_DataYear=TString(era(0, 4)).Atoi();
    }
    TString GetEra() const { return j_DataEra; }
    int GetYear() const { return j_DataYear; }

private:
    const std::map<TString, pair<Bool_t*, float>>* j_HLT_TriggerMapPtr;
    int j_nPV;
    int j_nPVsGood;
    float j_nTrueInt;
    RVec<TString> j_HLT_TriggerName;
    Particle j_METVector_PUPPI;
    Particle j_METVector_PUPPI_UE_UP;
    Particle j_METVector_PUPPI_UE_Down;
    Particle j_METVector_PUPPI_JER_UP;
    Particle j_METVector_PUPPI_JER_Down;
    Particle j_METVector_PUPPI_JES_UP;
    Particle j_METVector_PUPPI_JES_Down;


    int j_DataYear;
    TString j_DataEra;
    float j_fixedGridRhoFastjetAll;

    ClassDef(Event, 1)
};

#endif
