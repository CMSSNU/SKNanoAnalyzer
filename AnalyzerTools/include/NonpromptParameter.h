#ifndef NonpromptParameter_h
#define NonpromptParameter_h

#include "TString.h"
#include <iostream>
using namespace std;

class NonpromptParameter{
public:
    NonpromptParameter(): _name(""), _ID(""), _scale(""), _syst(""), _selection("") {}
    ~NonpromptParameter() {}
    void SetName() {
        // Choose which systematics is running
        // If "Central", all _scale, _syst, _selection would be "Central"
        // Otherwise, only one of the _scale, _syst, _selection would not be "Central"
        TString syst = "Central";
        if (_scale != "Central") syst = _scale;
        if (_syst != "Central") syst = _syst;
        if (_selection != "Central") syst = _selection;
        _name = _ID+"/"+syst;
    }
    inline void SetID(TString ID) { _ID = ID; }
    inline void SetScale(TString scale) { _scale = scale; }
    inline void SetSyst(TString syst) { _syst = syst; }
    inline void SetSelection(TString selection) { _selection = selection; }

    TString GetName() { this->SetName(); return _name; }
    inline TString GetID() const {return _ID; }
    inline TString GetScale() const { return _scale; }
    inline TString GetSyst() const { return _syst; }
    inline TString GetSelection() const { return _selection; }
    inline void SetDefault() { _name = ""; 
                              _ID = ""; 
                              _scale = "Central"; 
                              _syst = "Central"; 
                              _selection = "Central"; }
    inline void Clear() { _name = ""; 
                          _ID = ""; 
                          _scale = ""; 
                          _syst = ""; 
                          _selection = ""; }
    inline void Print() const { cout << "Name: " << _name << ", Scale: " << _scale << ", Syst: " << _syst << ", Selection: " << _selection << endl; }

private:
    TString _name;
    TString _ID;
    TString _scale;
    TString _syst;
    TString _selection;
};

#endif