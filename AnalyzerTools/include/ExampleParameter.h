#ifndef ExampleParameter_h
#define ExampleParameter_h

#include "AnalyzerParameter.h"

class ExampleParameter : public AnalyzerParameter {
public:
    ExampleParameter();
    ~ExampleParameter();

    TString AdditionalParameter;

    // using struct to give more functionality to the enum
    typedef unsigned int tSyst;
    struct SYST {
        static constexpr tSyst Central = 0;
        static constexpr tSyst JetResUp = 1;
        static constexpr tSyst JetResDown = 2;
        static constexpr tSyst JetEnUp = 3;
        static constexpr tSyst JetEnDown = 4;
        static constexpr tSyst MuonEnUp = 5;
        static constexpr tSyst MuonEnDown = 6;
        static constexpr tSyst ElectronResUp = 7;
        static constexpr tSyst ElectronResDown = 8;
        static constexpr tSyst NSyst = 9;

        static TString GetSystType(tSyst value) {
            switch(value) {
                case Central: return "Central";
                case JetResUp: return "JetResUp";
                case JetResDown: return "JetResDown";
                case JetEnUp: return "JetEnUp";
                case JetEnDown: return "JetEnDown";
                case MuonEnUp: return "MuonEnUp";
                case MuonEnDown: return "MuonEnDown";
                case ElectronResUp: return "ElectronResUp";
                case ElectronResDown: return "ElectronResDown";
                default:
                    cerr << "[ExampleParameter::SYST::toString] Unknown systematics" << endl;
                    exit(ENODATA);
            }
        }
    };
    inline TString GetSystType() const { return SYST::GetSystType(syst); }
    tSyst syst;
    void Clear() override;
};

#endif
