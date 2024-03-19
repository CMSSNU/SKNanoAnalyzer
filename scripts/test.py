import ROOT
from ROOT import TestAnalyzer

module = TestAnalyzer()
module.SetTreeName("Events")
#module.AddFile("test/1A59D67A-325A-E742-BFFE-2FE5F69EB01B_Skim.root")
#module.AddFile("test/7B930101-EB91-4F4E-9B90-0861460DBD94_Skim.root")
module.AddFile("test/428c6754-4ad2-4c2f-8ec6-4f2b5dc60758.root")
module.Init()

module.SetOutfilePath("test.root")
#module.initializeAnalyzer()
module.Loop()
module.WriteHist()

