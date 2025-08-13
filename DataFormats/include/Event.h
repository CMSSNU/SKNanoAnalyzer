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
    /*    
    enum class MET_Syst{
        CENTRAL,
        UE,
        JER,
        JES
    };
    */

    enum class MET_Type{
        PUPPI,
        CHS
    };

    void SetRunLumiEvent(int run, int lumi, int event) { j_run = run; j_lumi = lumi; j_event = event; }
    inline int run() const { return j_run; }
    inline int lumi() const { return j_lumi; }
    inline int event() const { return j_event; }
    void SetnPV(int nPV) { j_nPV = nPV;}
    inline int nPV() const { return j_nPV; }
    void SetnPileUp(int nPU) { j_nPV = nPU;}
    inline int nPileUp() const { return j_nPV; }
    void SetnTrueInt(float nTrueInt) { j_nTrueInt = nTrueInt;}
    inline float nTrueInt() const { return j_nTrueInt; }
    void SetnPVsGood(int nPVsGood) { j_nPVsGood = nPVsGood;}
    inline int nPVsGood() const { return j_nPVsGood; }
    void SetTrigger(const std::map<TString, pair<Bool_t *, float>>& TriggerMap);
    void setRho(float rho) { j_fixedGridRhoFastjetAll = rho; }
    inline float GetRho() const { return j_fixedGridRhoFastjetAll; }
    //void SetTrigger(RVec<TString> HLT_TriggerName) { j_HLT_TriggerName = HLT_TriggerName; }
    //void SetTrigger(TString HLT_TriggerName) { j_HLT_TriggerName = {HLT_TriggerName}; }

    bool PassTrigger(TString trig) const;
    bool PassTrigger(RVec<TString> trigs) const;
    float GetTriggerLumi(TString trig);
    bool IsPDForTrigger(TString trig, TString PD);

    //void SetMET(RVec<float> MET_pt, RVec<float> MET_phi);
    //Particle GetMETVector(Event::MET_Type MET_type, MyCorrection::variation syst = MyCorrection::variation::nom, Event::MET_Syst source = MET_Syst::CENTRAL) const;
    void SetMETVector(float MET_pt, float MET_phi, MET_Type MET_type) { 
        if (MET_type == MET_Type::CHS)
            j_METVector_CHS.SetPtEtaPhiM(MET_pt, 0, MET_phi, 0); 
        else if (MET_type == MET_Type::PUPPI)
            j_METVector_PUPPI.SetPtEtaPhiM(MET_pt, 0, MET_phi, 0);
        else
            throw std::runtime_error("Unknown MET_Type in SetMETVector");
    }
    inline Particle GetMETVector(MET_Type MET_type) const { 
        if (MET_type == MET_Type::CHS)
            return j_METVector_CHS; 
        else if (MET_type == MET_Type::PUPPI)
            return j_METVector_PUPPI; 
        else
            throw std::runtime_error("Unknown MET_Type in GetMETVector");
    }

    void SetGenMET(float GenMet_pt, float GenMet_phi) { j_GenMETVector.SetPtEtaPhiM(GenMet_pt, 0, GenMet_phi, 0); }
    Particle GetGenMETVector() const { return j_GenMETVector; }

    void SetEra(TString era) {
        j_DataEra = era;
        j_DataYear=TString(era(0, 4)).Atoi();
    }
    TString GetEra() const { return j_DataEra; }
    int GetYear() const { return j_DataYear; }

private:
    int j_run, j_lumi, j_event;
    const std::map<TString, pair<Bool_t*, float>>* j_HLT_TriggerMapPtr;
    int j_nPV;
    int j_nPVsGood;
    float j_nTrueInt;
    RVec<TString> j_HLT_TriggerName;
    //Particle j_METVector_PUPPI;
    //Particle j_METVector_PUPPI_UE_UP;
    //Particle j_METVector_PUPPI_UE_Down;
    //Particle j_METVector_PUPPI_JER_UP;
    //Particle j_METVector_PUPPI_JER_Down;
    //Particle j_METVector_PUPPI_JES_UP;
    //Particle j_METVector_PUPPI_JES_Down;
    Particle j_METVector_CHS;
    Particle j_METVector_PUPPI;
    Particle j_GenMETVector;

    int j_DataYear;
    TString j_DataEra;
    float j_fixedGridRhoFastjetAll;

    ClassDef(Event, 1)
};

#endif
