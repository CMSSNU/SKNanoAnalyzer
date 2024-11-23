import os
import json

with open('NanoAODv12_HLT_Path.txt', 'r') as f:
	lines = f.readlines()
HLT = [x.replace('\n','').replace(' ','').replace('\t','') for x in lines]
HLTS = {}
for H in HLT:
	HLTS[H] = {'lumi':0}

 
#dump(HLTS, open('NanoAODv12_HLT_Path.txt.json','w'))
#save as json
with open('NanoAODv12_HLT_Path.json', 'w') as f:
	json.dump(HLTS, f, indent=4)