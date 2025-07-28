#!/bin/bash
ERA=$1
CHANNEL=("EMu" "DiMu")

for ch in "${CHANNEL[@]}"; do
    echo "Submitting jobs for channel: $ch"

    if [[ $ch == "EMu" ]]; then
        DATASTREAM="MuonEG"
    else
        DATASTREAM="DoubleMuon,Muon,Muon0,Muon1"
    fi

    SKNano.py -a DiLepton -i $DATASTREAM -n 10 -e $ERA --userflags Run${ch},RunSyst
    SKNano.py -a DiLepton -i DYJets,TTLL_powheg,TTLJ_powheg -n 30 -e $ERA --userflags Run${ch},RunSyst
    SKNano.py -a DiLepton -i SampleLists/Run3NanoV13/DiLepton.txt -n 10 -e $ERA --userflags Run${ch},RunSyst
done
