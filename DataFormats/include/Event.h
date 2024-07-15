#ifndef Event_h
#define Event_h

#include "Particle.h"
#include "TString.h"
#include "TObject.h"
using namespace std;
#include <map>

class Event: public TObject {
public:
    Event();
    ~Event();

    void SetnPV(int nPV) { j_nPV = nPV;}
    inline int nPV() const { return j_nPV; }
    void SetnPileUp(int nPU) { j_nPV = nPU;}
    inline int nPileUp() const { return j_nPV; }
    void SetnTrueInt(float nTrueInt) { j_nTrueInt = nTrueInt;}
    inline float nTrueInt() const { return j_nTrueInt; }
    void SetnPVsGood(int nPVsGood) { j_nPVsGood = nPVsGood;}
    inline int nPVsGood() const { return j_nPVsGood; }
    void SetTrigger(RVec<TString> HLT_TriggerName, std::map<TString, Bool_t*> TriggerMap);
    void SetTrigger(TString HLT_TriggerName, std::map<TString, Bool_t*> TriggerMap);
    //void SetTrigger(RVec<TString> HLT_TriggerName) { j_HLT_TriggerName = HLT_TriggerName; }
    //void SetTrigger(TString HLT_TriggerName) { j_HLT_TriggerName = {HLT_TriggerName}; }

    bool PassTrigger(TString trig);
    //bool PassTrigger(RVec<TString> trigs);
    float GetTriggerLumi(TString trig);
    bool IsPDForTrigger(TString trig, TString PD);

    void SetMET(float pt, float phi);
    inline Particle GetMETVector() const {return j_METVector;}

    void SetEra(TString era) {
        j_DataEra = era;
        j_DataYear=TString(era(0, 4)).Atoi();
    }
    TString GetEra() const { return j_DataEra; }
    int GetYear() const { return j_DataYear; }

private:

    std::map<TString, bool*> j_HLT_PassTrigger;
    int j_nPV;
    int j_nPVsGood;
    float j_nTrueInt;
    RVec<TString> j_HLT_TriggerName;
    Particle j_METVector;
    int j_DataYear;
    TString j_DataEra;

    ClassDef(Event, 1)
};

#endif
