#!/bin/bash

base_folder="/gv0/DATA/SKNano/Run3NanoAODv12/2022EE/MC/Skim/yeonjoon"
suffix=Vcb_FH


for folder in "$base_folder"/Skim_$suffix*; do
    if [ -d "$folder" ]; then
        # Remove the "Skim_" prefix from the folder name
        skim_tree_suffix=$(basename "$folder" | sed 's/^Skim_//')
        #get orig_pd from $folder, .../Skim_Vcb_DL_QCD_HT100to200 -> QCD_HT100to200
        orig_pd=$(basename "$folder" | sed 's/^Skim_//')
        orig_pd=$(echo $orig_pd | sed "s/${suffix}_//")
        echo "orig_pd: $orig_pd"

        
        if [ -n "$period_suffix" ]; then
            python3 sampleManager.py \
                --era 2022EE \
                --makeSkimTreeInfo \
                --skimTreeFolder "$base_folder" \
                --skimTreeSuffix "$suffix" \
                --skimTreeOrigPD ${orig_pd}
        fi
    fi
done

