#ifndef Vcb_DL_h
#define Vcb_DL_h

#include "Vcb.h"

class Vcb_DL : public Vcb 
{
public:
    bool PassBaseLineSelection(bool remove_flavtagging_cut = false) override;
    inline bool CheckChannel() override
    {
        if (channel != Channel::MM && channel != Channel::EE && channel != Channel::ME) return false;
        return true;
    };

    inline float MCNormalization() override { 
        if (channel == Channel::MM){
            return MCweight()*ev.GetTriggerLumi(Mu_Trigger[DataEra.Data()]);
        }
        else if (channel == Channel::EE){
            return MCweight()*ev.GetTriggerLumi(El_Trigger[DataEra.Data()]);
        }
        else if (channel == Channel::ME){
            return MCweight()*(ev.GetTriggerLumi(Mu_Trigger[DataEra.Data()]));
        }
        else throw std::runtime_error("Invalid channel");
    }

    inline std::string GetRegionString() override
    {
        return "CR_DL";
    }

    Vcb_DL();
    ~Vcb_DL() override = default;
    unique_ptr<CorrectionSet> cset_TriggerSF;

};

#endif
