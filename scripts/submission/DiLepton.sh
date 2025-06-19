#!/bin/bash
ERA=$1
CHANNEL=("EMu" "DiMu")

for ch in "${CHANNEL[@]}"; do
    echo "Submitting jobs for channel: $ch"

    if [[ $ch == "EMu" ]]; then
        DATASTREAM="MuonEG"
    else
        DATASTREAM="DoubleMuon"
    fi

    SKNano.py -a DiLepton -i $DATASTREAM -n 10 -e $ERA --userflags Run${ch},RunSyst --python
    SKNano.py -a DiLepton -i DYJets,TTLL_powheg,TTLJ_powheg -n 30 -e $ERA --userflags Run${ch},RunSyst --python
    SKNano.py -a DiLepton -i SampleLists/Run2NanoV9/DiLepton.txt -n 20 -e $ERA --userflags Run${ch},RunSyst --python
done
