from ROOT import TString
from ROOT.VecOps import RVec, Sort, Reverse
from ROOT import DiLeptonBase
from ROOT import MyCorrection; myVar = MyCorrection.variation
from ROOT import JetTagging
from ROOT import Event, Muon, Electron, Jet

class DiLepton(DiLeptonBase):
    def __init__(self):
        super().__init__()

    def initializePyAnalyzer(self):
        self.initializeAnalyzer()

        # link flags
        self.channel = None
        self.run_syst = False

        if self.RunDiMu: self.channel = "RunDiMu"
        if self.RunEMu: self.channel = "RunEMu"
        if self.RunSyst: self.run_syst = True

        # systematics
        self.weightVariations = ["Central"]
        self.scaleVariations = []
        if not self.IsDATA and self.run_syst:
            self.scaleVariations.append(("JetResUp", "JetResDown"))
            self.scaleVariations.append(("JetEnUp", "JetEnDown"))
            self.scaleVariations.append(("ElectronResUp", "ElectronResDown"))
            self.scaleVariations.append(("ElectronEnUp", "ElectronEnDown"))
            self.scaleVariations.append(("MuonEnUp", "MuonEnDown"))
            if self.channel == "RunDiMu":
                self.weightVariations.append(("L1PrefireUp", "L1PrefireDown"))
                self.weightVariations.append(("DblMuTrigSFUp", "DblMuTrigSFDown"))
                self.weightVariations.append(("PileupReweightUp", "PileupReweightDown"))
                self.weightVariations.append(("MuonIDSFUp", "MuonIDSFDown"))
                self.weightVariations.append(("PileupJetIDSFUp", "PileupJetIDSFDown"))
            if self.channel == "RunEMu":
                self.weightVariations.append(("L1PrefireUp", "L1PrefireDown"))
                self.weightVariations.append(("EMuTrigSFUp", "EMuTrigSFDown"))
                self.weightVariations.append(("PileupReweightUp", "PileupReweightDown"))
                self.weightVariations.append(("MuonIDSFUp", "MuonIDSFDown"))
                self.weightVariations.append(("ElectronIDSFUp", "ElectronIDSFDown"))
                self.weightVariations.append(("PileupJetIDSFUp", "PileupJetIDSFDown"))
        self.systematics = self.weightVariations + self.scaleVariations

    def executeEvent(self):
        event = self.GetEvent()
        METv = event.GetMETVector(Event.MET_Type.PUPPI)
        rawJets = self.GetAllJets()
        if not self.PassMETFilter(METv, rawJets): return
        
        rawMuons = self.GetAllMuons()
        rawElectrons = self.GetAllElectrons()
        genParts = self.GetAllGens() if not self.IsDATA else None
        genJets = self.GetAllGenJets() if not self.IsDATA else None

        def processEvent(syst, apply_weight_variation=False):
            recoObjects = self.defineObjects(event, rawMuons, rawElectrons, rawJets, genJets, METv, syst)
            channel = self.selectEvent(event, recoObjects)
            if not channel: return
    
            if apply_weight_variation:
                assert syst == "Central", "Only Central weight variation is allowed"
                weights = self.getWeights(channel, event, recoObjects, genParts)
                self.fillObjects(channel, recoObjects, weights)
                for systSet in self.weightVariations[1:]:
                    syst_up, syst_down = systSet
                    weights_up = self.getWeights(channel, event, recoObjects, genParts, syst_up)
                    weights_down = self.getWeights(channel, event, recoObjects, genParts, syst_down)
                    self.fillObjects(channel, recoObjects, weights_up, syst_up)
                    self.fillObjects(channel, recoObjects, weights_down, syst_down)
            else:
                weights = self.getWeights(channel, event, recoObjects, genParts, syst)
                self.fillObjects(channel, recoObjects, weights, syst)
        
        processEvent("Central", apply_weight_variation=True)
        for scaleSet in self.scaleVariations:
            scale_up, scale_down = scaleSet
            processEvent(scale_up, apply_weight_variation=False)
            processEvent(scale_down, apply_weight_variation=False)
    
    
    def defineObjects(self, ev, rawMuons, rawElectrons, rawJets, genJets, METv, syst="Central"):
        # Create copies of the raw objects
        allMuons = RVec(Muon)(rawMuons)
        allElectrons = RVec(Electron)(rawElectrons)
        allJets = RVec(Jet)(rawJets)
        self.myCorr.METXYCorrection(METv, ev.run(), ev.nPV(), MyCorrection.XYCorrection_MetType.Type1PuppiMET)

        # Apply scale variations
        if syst == "ElectronEnUp":
            allElectrons = self.ScaleElectrons(allElectrons, "up")
        elif syst == "ElectronEnDown":
            allElectrons = self.ScaleElectrons(allElectrons, "down")
        elif syst == "ElectronResUp":
            allElectrons = self.SmearElectrons(allElectrons, "up")
        elif syst == "ElectronResDown":
            allElectrons = self.SmearElectrons(allElectrons, "down")
        elif syst == "MuonEnUp":
            allMuons = self.ScaleMuons(allMuons, "up")
        elif syst == "MuonEnDown":
            allMuons = self.ScaleMuons(allMuons, "down")
        elif syst == "JetEnUp":
            allJets = self.ScaleJets(allJets, "up")
        elif syst == "JetEnDown":
            allJets = self.ScaleJets(allJets, "down")
        elif syst == "JetResUp":
            allJets = self.SmearJets(allJets, genJets, "up")
        elif syst == "JetResDown":
            allJets = self.SmearJets(allJets, genJets, "down")
        else:
            assert self.checkValidSyst(syst), f"Invalid systematics: {syst}"

        # sort objects in pt order
        allMuons = Reverse(Sort(allMuons))
        allElectrons = Reverse(Sort(allElectrons))
        allJets = Reverse(Sort(allJets))

        looseMuons = self.SelectMuons(allMuons, self.MuonIDs.GetID("loose"), 10., 2.4)
        tightMuons = self.SelectMuons(looseMuons, self.MuonIDs.GetID("tight"), 10., 2.4)
        looseElectrons = self.SelectElectrons(allElectrons, self.ElectronIDs.GetID("loose"), 15., 2.5)
        tightElectrons = self.SelectElectrons(looseElectrons, self.ElectronIDs.GetID("tight"), 15., 2.5)
        max_jeteta = 2.4 if self.DataEra.Contains("2016") else 2.5
        tightJets = self.SelectJets(allJets, "tight", 20., max_jeteta)
        tightJets_vetoLep = self.JetsVetoLeptonInside(tightJets, looseElectrons, looseMuons, 0.4)
        tightJets_vetoLep_loosePileupID = self.SelectJets(tightJets_vetoLep, "loosePuId", 20., max_jeteta)
        bjets = RVec(Jet)()
        wp = self.myCorr.GetBTaggingWP(JetTagging.JetFlavTagger.DeepJet, JetTagging.JetFlavTaggerWP.Medium)
        for jet in tightJets_vetoLep_loosePileupID:
            btagScore = jet.GetBTaggerResult(JetTagging.JetFlavTagger.DeepJet)
            if btagScore > wp: bjets.emplace_back(jet)

        return {
            "looseMuons": looseMuons,
            "tightMuons": tightMuons,
            "looseElectrons": looseElectrons,
            "tightElectrons": tightElectrons,
            "tightJets": tightJets,
            "tightJets_vetoLep": tightJets_vetoLep,
            "tightJets_vetoLep_loosePileupID": tightJets_vetoLep_loosePileupID,
            "bjets": bjets,
            "genJets": genJets,
            "METv": METv
        }
    
    def selectEvent(self, ev, recoObjects):
        looseMuons = recoObjects["looseMuons"]
        tightMuons = recoObjects["tightMuons"]
        looseElectrons = recoObjects["looseElectrons"]
        tightElectrons = recoObjects["tightElectrons"]
        jets = recoObjects["tightJets_vetoLep_loosePileupID"]
        bjets = recoObjects["bjets"]
        METv = recoObjects["METv"]

        isDiMu = (len(tightMuons) == 2 and len(looseMuons) == 2 and \
                  len(tightElectrons) == 0 and len(looseElectrons) == 0)
        isEMu = (len(tightMuons) == 1 and len(looseMuons) == 1 and \
                 len(tightElectrons) == 1 and len(looseElectrons) == 1)
        
        if self.channel == "RunDiMu":
            if not isDiMu: return
        if self.channel == "RunEMu":
            if not isEMu: return
        
        ## DiMu selection
        if self.channel == "RunDiMu":
            if not ev.PassTrigger(self.DblMuTriggers): return
            mu1, mu2 = tuple(tightMuons)
            if not mu1.Pt() > 20.: return
            if not mu2.Pt() > 10.: return
            if not mu1.Charge()+mu2.Charge() == 0: return
            pair = mu1 + mu2
            if not pair.M() > 50.: return
            return "DiMu"
        ## EMu selection
        elif self.channel == "RunEMu":
            if not ev.PassTrigger(self.EMuTriggers): return
            mu = tightMuons.at(0)
            ele = tightElectrons.at(0)
            if not ((mu.Pt() > 20. and ele.Pt() > 15.) or (mu.Pt() > 10. and ele.Pt() > 25.)): return 
            if not mu.Charge()+ele.Charge() == 0: return 
            if not mu.DeltaR(ele) > 0.4: return
            if not jets.size() >= 2: return 
            if not bjets.size() >= 1: return 
            return "EMu"
        else:
            raise ValueError(f"Wrong channel {self.channel}")

    def checkValidSyst(self, syst):
        for systset in self.systematics:
            if type(systset) == str:
                if syst == systset: return True
            elif type(systset) == tuple:
                if syst in systset: return True
        return False

    def getWeights(self, channel, event, recoObjects, genParts, syst="Central"):
        if self.IsDATA:
            return {
                "weight": 1.
            }
        assert self.checkValidSyst(syst), f"[DiLepton::getWeights] Invalid systematics: {syst}"

        genWeight = self.MCweight()*event.GetTriggerLumi("Full")

        prefireWeight, pileupWeight, topPtWeight = 1., 1., 1.
        if syst == "L1PrefireUp": prefireWeight = self.GetL1PrefireWeight(myVar.up)
        elif syst == "L1PrefireDown": prefireWeight = self.GetL1PrefireWeight(myVar.down)
        else: prefireWeight = self.GetL1PrefireWeight(myVar.nom)
        
        if syst == "PileupReweightUp": pileupWeight = self.myCorr.GetPUWeight(event.nTrueInt(), myVar.up)
        elif syst == "PileupReweightDown": pileupWeight = self.myCorr.GetPUWeight(event.nTrueInt(), myVar.down)
        else: pileupWeight = self.myCorr.GetPUWeight(event.nTrueInt(), myVar.nom)
        
        topPtWeight = 1.
        if self.MCSample.Contains("TTLL") or self.MCSample.Contains("TTLJ"):
            topPtWeight = self.myCorr.GetTopPtReweight(genParts)

        muonRecoSF, muonIDSF = 1., 1.
        eleRecoSF, eleIDSF = 1., 1.
        trigSF = 1.
        electrons = recoObjects["tightElectrons"]
        muons = recoObjects["tightMuons"]

        muonRecoSF = self.myCorr.GetMuonRECOSF(muons)
        if syst == "MuonIDSFUp":
            muonIDSF = self.myCorr.GetMuonIDSF("TopHNT", muons, myVar.up)
        elif syst == "MuonIDSFDown":
            muonIDSF = self.myCorr.GetMuonIDSF("TopHNT", muons, myVar.down)
        else:
            muonIDSF = self.myCorr.GetMuonIDSF("TopHNT", muons)

        eleRecoSF = self.myCorr.GetElectronRECOSF(electrons)
        if syst == "ElectronIDSFUp":
            eleIDSF = self.myCorr.GetElectronIDSF("TopHNT", electrons, myVar.up)
        elif syst == "ElectronIDSFDown":
            eleIDSF = self.myCorr.GetElectronIDSF("TopHNT", electrons, myVar.down)
        else:
            eleIDSF = self.myCorr.GetElectronIDSF("TopHNT", electrons)

        if "EMu" in channel:
            if syst == "EMuTrigSFUp":
                trigSF = self.myCorr.GetEMuTriggerSF(electrons, muons, myVar.up)
            elif syst == "EMuTrigSFDown":
                trigSF = self.myCorr.GetEMuTriggerSF(electrons, muons, myVar.down)
            else:
                trigSF = self.myCorr.GetEMuTriggerSF(electrons, muons)
        elif "DiMu" in channel:
            if syst == "DblMuTrigSFUp":
                trigSF = self.myCorr.GetDblMuTriggerSF(muons, myVar.up)
            elif syst == "DblMuTrigSFDown":
                trigSF = self.myCorr.GetDblMuTriggerSF(muons, myVar.down)
            else:
                trigSF = self.myCorr.GetDblMuTriggerSF(muons)
        else:
            raise ValueError(f"[DiLepton::getWeights] Invalid channel: {channel}")

        pileupIDSF, btagSF = 1., 1.
        jets = recoObjects["tightJets_vetoLep_loosePileupID"]
        genJets = recoObjects["genJets"]
        matched_idx = self.GenJetMatching(jets, genJets, self.fixedGridRhoFastjetAll, 0.4, 10.)
        if syst == "PileupJetIDSFUp":
            pileupIDSF = self.myCorr.GetPileupJetIDSF(jets, matched_idx, "loose", myVar.up)
        elif syst == "PileupJetIDSFDown":
            pileupIDSF = self.myCorr.GetPileupJetIDSF(jets, matched_idx, "loose", myVar.down)
        else:
            pileupIDSF = self.myCorr.GetPileupJetIDSF(jets, matched_idx, "loose", myVar.nom)
        
        if "EMu" in channel:
            jets = recoObjects["tightJets_vetoLep_loosePileupID"]
            tagger, wp, method = JetTagging.JetFlavTagger.DeepJet, JetTagging.JetFlavTaggerWP.Medium, JetTagging.JetTaggingSFMethod.mujets
            btagSF = self.myCorr.GetBTaggingSF(jets, tagger, wp, method)
        
        return {
            "genWeight": genWeight,
            "prefireWeight": prefireWeight,
            "pileupWeight": pileupWeight,
            "topPtWeight": topPtWeight,
            "muonRecoSF": muonRecoSF,
            "muonIDSF": muonIDSF,
            "eleRecoSF": eleRecoSF,
            "eleIDSF": eleIDSF,
            "trigSF": trigSF,
            "pileupIDSF": pileupIDSF,
            "btagSF": btagSF
        }
    
    def fillObjects(self, channel, recoObjects, weights, syst="Central"):
        muons = recoObjects["tightMuons"]
        electrons = recoObjects["tightElectrons"]
        jets = recoObjects["tightJets_vetoLep_loosePileupID"]
        bjets = recoObjects["bjets"]
        METv = recoObjects["METv"]
        genJets = recoObjects["genJets"]
        weight = 1.

        if not self.IsDATA:
            genWeight = weights["genWeight"]
            prefireWeight = weights["prefireWeight"]
            pileupWeight = weights["pileupWeight"]
            topPtWeight = weights["topPtWeight"]
            muonRecoSF = weights["muonRecoSF"]
            muonIDSF = weights["muonIDSF"]
            eleRecoSF = weights["eleRecoSF"]
            eleIDSF = weights["eleIDSF"]
            trigSF = weights["trigSF"]
            pileupIDSF = weights["pileupIDSF"]
            btagSF = weights["btagSF"]
            weight = genWeight*prefireWeight*pileupWeight*topPtWeight*muonRecoSF*muonIDSF*eleRecoSF*eleIDSF*trigSF*pileupIDSF*btagSF

            self.FillHist(f"{channel}/{syst}/weights/genWeight", genWeight, 1., 200, -10000, 10000.)
            self.FillHist(f"{channel}/{syst}/weights/prefireWeight", prefireWeight, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/pileupWeight", pileupWeight, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/topPtWeight", topPtWeight, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/muonRecoSF", muonRecoSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/muonIDSF", muonIDSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/eleRecoSF", eleRecoSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/eleIDSF", eleIDSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/trigSF", trigSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/pileupIDSF", pileupIDSF, 1., 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/weights/btagSF", btagSF, 1., 100, -5., 5.)
        
        for idx, mu in enumerate(muons, start=1):
            self.FillHist(f"{channel}/{syst}/muons/{idx}/pt", mu.Pt(), weight, 300, 0., 300.)
            self.FillHist(f"{channel}/{syst}/muons/{idx}/eta", mu.Eta(), weight, 48, -2.4, 2.4)
            self.FillHist(f"{channel}/{syst}/muons/{idx}/phi", mu.Phi(), weight, 64, -3.2, 3.2)
            self.FillHist(f"{channel}/{syst}/muons/{idx}/mass", mu.M(), weight, 10, 0., 1.)
        for idx, ele in enumerate(electrons, start=1):
            self.FillHist(f"{channel}/{syst}/electrons/{idx}/pt", ele.Pt(), weight, 300, 0., 300.)
            self.FillHist(f"{channel}/{syst}/electrons/{idx}/eta", ele.Eta(), weight, 50, -2.5, 2.5)
            self.FillHist(f"{channel}/{syst}/electrons/{idx}/phi", ele.Phi(), weight, 64, -3.2, 3.2)
            self.FillHist(f"{channel}/{syst}/electrons/{idx}/mass", ele.M(), weight, 100, 0., 1.)

        for idx, jet in enumerate(jets, start=1):
            self.FillHist(f"{channel}/{syst}/jets/{idx}/pt", jet.Pt(), weight, 300, 0., 300.)
            self.FillHist(f"{channel}/{syst}/jets/{idx}/eta", jet.Eta(), weight, 48, -2.4, 2.4)
            self.FillHist(f"{channel}/{syst}/jets/{idx}/phi", jet.Phi(), weight, 64, -3.2, 3.2)
            self.FillHist(f"{channel}/{syst}/jets/{idx}/mass", jet.M(), weight, 100, 0., 100.)
        for idx, bjet in enumerate(bjets, start=1):
            self.FillHist(f"{channel}/{syst}/bjets/{idx}/pt", bjet.Pt(), weight, 300, 0., 300.)
            self.FillHist(f"{channel}/{syst}/bjets/{idx}/eta", bjet.Eta(), weight, 48, -2.4, 2.4)
            self.FillHist(f"{channel}/{syst}/bjets/{idx}/phi", bjet.Phi(), weight, 64, -3.2, 3.2)
            self.FillHist(f"{channel}/{syst}/bjets/{idx}/mass", bjet.M(), weight, 100, 0., 100.)
        self.FillHist(f"{channel}/{syst}/jets/size", jets.size(), weight, 20, 0., 20.)
        self.FillHist(f"{channel}/{syst}/bjets/size", bjets.size(), weight, 15, 0., 15.)
        self.FillHist(f"{channel}/{syst}/METv/pt", METv.Pt(), weight, 300, 0., 300.)
        self.FillHist(f"{channel}/{syst}/METv/phi", METv.Phi(), weight, 64, -3.2, 3.2)
        
        if "DiMu" in channel:
            pair = muons.at(0) + muons.at(1)
            self.FillHist(f"{channel}/{syst}/pair/pt", pair.Pt(), weight, 300, 0., 300.)
            self.FillHist(f"{channel}/{syst}/pair/eta", pair.Eta(), weight, 100, -5., 5.)
            self.FillHist(f"{channel}/{syst}/pair/phi", pair.Phi(), weight, 64, -3.2, 3.2)
            self.FillHist(f"{channel}/{syst}/pair/mass", pair.M(), weight, 300, 0., 300.)

if __name__ == "__main__":
    module = DiLepton()
    module.SetTreeName("Events")
    module.LogEvery = 5000
    module.IsDATA = False
    module.MCSample = "TTLL_powheg"
    module.xsec = 88.29
    module.sumW = 7695841652.167358
    module.sumSign = 105859990.0
    module.SetEra("2022")
    module.Userflags = RVec(TString)(["RunEMu", "RunSyst"])
    module.AddFile("NANOAOD_255.root")
    module.MaxEvent = max(1, int(module.fChain.GetEntries()/100))
    module.SetOutfilePath("hist.root")
    module.Init()
    module.initializePyAnalyzer()
    module.Loop()
    module.WriteHist()
