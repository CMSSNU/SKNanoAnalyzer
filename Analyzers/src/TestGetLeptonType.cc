#include "TestGetLeptonType.h"

TestGetLeptonType::TestGetLeptonType() {}
TestGetLeptonType::~TestGetLeptonType() {}

void TestGetLeptonType::initializeAnalyzer() {
    cout << "[TestGetLeptonType::initializeAnalyzer] Initializing GetLeptonType test analyzer" << endl;
    cout << "[TestGetLeptonType::initializeAnalyzer] DataEra = " << DataEra << endl;
    cout << "[TestGetLeptonType::initializeAnalyzer] IsDATA = " << IsDATA << endl;
    
    myCorr = new MyCorrection(DataEra, IsDATA?DataStream:MCSample, IsDATA);
    cout << "[TestGetLeptonType::initializeAnalyzer] MyCorrection initialized" << endl;
}

void TestGetLeptonType::executeEvent() {
    AllMuons = GetAllMuons();
    AllElectrons = GetAllElectrons();
    AllGens = GetAllGens();

    cout << "\n=== EVENT " << EventNumber << " ===" << endl;
    cout << "Number of Gen particles: " << AllGens.size() << endl;
    cout << "Number of Muons: " << AllMuons.size() << endl;
    cout << "Number of Electrons: " << AllElectrons.size() << endl;

    cout << "\n--- GEN PARTICLES INFO ---" << endl;
    for (size_t i = 0; i < AllGens.size(); i++) {
        const Gen& gen = AllGens[i];
        if (abs(gen.PID()) == 11 || abs(gen.PID()) == 13) {
            cout << "Gen[" << i << "] PID=" << gen.PID() 
                 << " Status=" << gen.Status()
                 << " Pt=" << gen.Pt() 
                 << " Eta=" << gen.Eta() 
                 << " Phi=" << gen.Phi()
                 << " Index=" << gen.Index()
                 << " MotherIndex=" << gen.MotherIndex()
                 << endl;
        }
    }

    cout << "\n--- MUON ANALYSIS ---" << endl;
    for (size_t i = 0; i < AllMuons.size(); i++) {
        const Muon& mu = AllMuons[i];
        cout << "Muon[" << i << "] Pt=" << mu.Pt() 
             << " Eta=" << mu.Eta() 
             << " Phi=" << mu.Phi() << endl;

        int leptonType = GetLeptonType(mu, AllGens);
        cout << "  -> GetLeptonType result: " << leptonType << endl;

        Gen matchedGen = GetGenMatchedLepton(mu, AllGens);
        if (!matchedGen.IsEmpty()) {
            cout << "  -> Matched Gen: Index=" << matchedGen.Index()
                 << " PID=" << matchedGen.PID()
                 << " Status=" << matchedGen.Status()
                 << " Pt=" << matchedGen.Pt()
                 << " MotherIndex=" << matchedGen.MotherIndex() << endl;

            cout << "  -> Gen particle GetLeptonType: " << GetLeptonType(matchedGen, AllGens) << endl;

            cout << "  -> Detailed GEN history for matched particle:" << endl;
            int currentIdx = matchedGen.Index();
            int depth = 0;
            while (currentIdx >= 0 && depth < 10) {
                if (currentIdx >= (int)AllGens.size()) break;
                const Gen& currentGen = AllGens[currentIdx];
                cout << "    [" << depth << "] Index=" << currentIdx 
                     << " PID=" << currentGen.PID()
                     << " Status=" << currentGen.Status()
                     << " MotherIndex=" << currentGen.MotherIndex() << endl;
                
                int motherIdx = currentGen.MotherIndex();
                if (motherIdx == currentIdx || motherIdx < 0) break;
                currentIdx = motherIdx;
                depth++;
            }
        } else {
            cout << "  -> No matched gen lepton found" << endl;
            
            Gen matchedPhoton = GetGenMatchedPhoton(mu, AllGens);
            if (!matchedPhoton.IsEmpty()) {
                cout << "  -> Matched Gen Photon: Index=" << matchedPhoton.Index()
                     << " PID=" << matchedPhoton.PID()
                     << " Status=" << matchedPhoton.Status()
                     << " Pt=" << matchedPhoton.Pt() << endl;
            }
        }
        cout << endl;
    }

    cout << "\n--- ELECTRON ANALYSIS ---" << endl;
    for (size_t i = 0; i < AllElectrons.size(); i++) {
        const Electron& el = AllElectrons[i];
        cout << "Electron[" << i << "] Pt=" << el.Pt() 
             << " Eta=" << el.Eta() 
             << " Phi=" << el.Phi() << endl;

        int leptonType = GetLeptonType(el, AllGens);
        cout << "  -> GetLeptonType result: " << leptonType << endl;

        Gen matchedGen = GetGenMatchedLepton(el, AllGens);
        if (!matchedGen.IsEmpty()) {
            cout << "  -> Matched Gen: Index=" << matchedGen.Index()
                 << " PID=" << matchedGen.PID()
                 << " Status=" << matchedGen.Status() 
                 << " Pt=" << matchedGen.Pt()
                 << " MotherIndex=" << matchedGen.MotherIndex() << endl;

            cout << "  -> Gen particle GetLeptonType: " << GetLeptonType(matchedGen, AllGens) << endl;

            cout << "  -> Detailed GEN history for matched particle:" << endl;
            int currentIdx = matchedGen.Index();
            int depth = 0;
            while (currentIdx >= 0 && depth < 10) {
                if (currentIdx >= (int)AllGens.size()) break;
                const Gen& currentGen = AllGens[currentIdx];
                cout << "    [" << depth << "] Index=" << currentIdx 
                     << " PID=" << currentGen.PID()
                     << " Status=" << currentGen.Status()
                     << " MotherIndex=" << currentGen.MotherIndex() << endl;
                
                int motherIdx = currentGen.MotherIndex();
                if (motherIdx == currentIdx || motherIdx < 0) break;
                currentIdx = motherIdx;
                depth++;
            }
        } else {
            cout << "  -> No matched gen lepton found" << endl;
            
            Gen matchedPhoton = GetGenMatchedPhoton(el, AllGens);
            if (!matchedPhoton.IsEmpty()) {
                cout << "  -> Matched Gen Photon: Index=" << matchedPhoton.Index()
                     << " PID=" << matchedPhoton.PID()
                     << " Status=" << matchedPhoton.Status()
                     << " Pt=" << matchedPhoton.Pt() << endl;
            }
        }
        cout << endl;
    }

    cout << "\n--- LEPTON TYPE MEANING ---" << endl;
    cout << "1: EW-Prompt" << endl;
    cout << "2: BSM-Prompt" << endl;
    cout << "3: EW/BSM-Prompt-Tau Daughter" << endl;
    cout << "4: Internal Conversion from Soft QED Radiation (PS-level)" << endl;
    cout << "5: Internal Conversion from Hard Process Photon (ME-level)" << endl;
    cout << "6: From Offshell W (mother == 37)" << endl;
    cout << "-1: Unmatched & not EW Conversion candidate" << endl;
    cout << "-2: Hadron Daughter" << endl;
    cout << "-3: Daughter of Tau from Hadron or Parton" << endl;
    cout << "-4: Internal Conversion Daughter having hadronic origin" << endl;
    cout << "-5: External Conversion from Hard process photon" << endl;
    cout << "-6: External conversion from t/EWV/EWlep" << endl;
    cout << "0: Error" << endl;
    cout << "\n========================\n" << endl;
}