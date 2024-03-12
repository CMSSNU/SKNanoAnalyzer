from ROOT import SKNanoLoader
from ROOT import Muon

loader = SKNanoLoader()
loader.SetTreeName("Events")
loader.AddFile("test/1A59D67A-325A-E742-BFFE-2FE5F69EB01B_Skim.root")
loader.AddFile("test/7B930101-EB91-4F4E-9B90-0861460DBD94_Skim.root")
loader.Init()

for evt in loader.fChain:
    print(evt.Muon_pt)
    break
