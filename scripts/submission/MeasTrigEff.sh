#!/bin/bash
SKNano.py -a MeasTrigEff -i SingleElectron,EGamma,EGamma0,EGamma1 -n 10 -r Run2,Run3 --userflags MeasMuLegs
SKNano.py -a MeasTrigEff -i SingleMuon,Muon,Muon0,Muon1 -n 10 -r Run2,Run3 --userflags MeasElLegs
SKFlat.py -a MeasTrigEff -i DoubleMuon,Muon,Muon0,Muon1 -n 10 -r Run2,Run3 --userflags MeasDblMuDZ
SKFlat.py -a MeasTrigEff -i MuonEG -n 10 -r Run2,Run3 --userflags MeasEMuDZ
SKFlat.py -a MeasTrigEff -i TTLL_powheg,DYJets,DYJets10to50 -n 20 -r Run2,Run3 --userflags MeasMuLegs
SKFlat.py -a MeasTrigEff -i TTLL_powheg,DYJets,DYJets10to50 -n 20 -r Run2,Run3 --userflags MeasElLegs
SKFlat.py -a MeasTrigEff -i TTLL_powheg,DYJets,DYJets10to50 -n 20 -r Run2,Run3 --userflags MeasDblMuDZ
SKFlat.py -a MeasTrigEff -i TTLL_powheg,DYJets,DYJets10to50 -n 20 -r Run2,Run3 --userflags MeasEMuDZ