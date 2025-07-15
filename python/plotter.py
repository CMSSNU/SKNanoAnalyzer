import ROOT
import cmsstyle as CMS
from array import array

PALLETE = [
        ROOT.TColor.GetColor("#5790fc"),
        ROOT.TColor.GetColor("#f89c20"),
        ROOT.TColor.GetColor("#e42536"),
        ROOT.TColor.GetColor("#964a8b"),
        ROOT.TColor.GetColor("#9c9ca1"),
        ROOT.TColor.GetColor("#7a21dd")
]

PALLETE_LONG = [
        ROOT.TColor.GetColor("#3f90da"),
        ROOT.TColor.GetColor("#ffa90e"),
        ROOT.TColor.GetColor("#bd1f01"),
        ROOT.TColor.GetColor("#94a4a2"),
        ROOT.TColor.GetColor("#832db6"),
        ROOT.TColor.GetColor("#a96b59"),
        ROOT.TColor.GetColor("#e76300"),
        ROOT.TColor.GetColor("#b9ac70"),
        ROOT.TColor.GetColor("#717581"),
        ROOT.TColor.GetColor("#92dadd")
]

class KinematicCanvas():
    def __init__(self, hists, config, ref=None):
        super().__init__()

        if abs(config["xRange"][0]) == abs(config["xRange"][1]):
            # xRange is symmetric, no need to handle overflow
            # Sumw2 should be called for error calculation
            if ref is not None:
                ref.Sumw2()
            for hist in hists.values():
                hist.Sumw2()
        else:
            # Get overflow and add to the last bin
            if ref is not None:
                last_bin = ref.FindBin(config["xRange"][-1])
                overflow, overflow_err = 0., 0.
                for idx in range(last_bin+1, ref.GetNbinsX()+1):
                    overflow += ref.GetBinContent(idx)
                    overflow_err += ref.GetBinError(idx)**2
                overflow_err = ROOT.TMath.Sqrt(overflow_err)
                ref.SetBinContent(last_bin, ref.GetBinContent(last_bin)+overflow)
                ref.SetBinError(last_bin, ROOT.TMath.Sqrt(ref.GetBinError(last_bin)**2 + overflow_err**2))

            for hist in hists.values():
                last_bin = hist.FindBin(config["xRange"][-1])
                overflow, overflow_err = 0., 0.
                for idx in range(last_bin+1, hist.GetNbinsX()+1):
                    overflow += hist.GetBinContent(idx)
                    overflow_err += hist.GetBinError(idx)**2
                overflow_err = ROOT.TMath.Sqrt(overflow_err)
                hist.SetBinContent(last_bin, hist.GetBinContent(last_bin)+overflow)
                hist.SetBinError(last_bin, ROOT.TMath.Sqrt(hist.GetBinError(last_bin)**2 + overflow_err**2))
        
        self.ref = ref
        self.hists = hists
        self.ref_line = ROOT.TLine(config["xRange"][0], 1., config["xRange"][-1], 1.)

        if len(hists) > 6:
            self.PALLETE = PALLETE_LONG
        else:
            self.PALLETE = PALLETE

        # Set histogram binning
        if "rebin" in config.keys():
            self.ref.Rebin(config["rebin"])
            for hist in self.hists.values():
                hist.Rebin(config["rebin"])
        elif len(config["xRange"]) > 2:
            # Create variable binning array from config
            bins = array('d', config["xRange"])
            # Rebin histograms with variable bin sizes
            self.ref = self.ref.Rebin(len(bins)-1, self.ref.GetName()+"_rebin", bins)
            for name, hist in self.hists.items():
                self.hists[name] = hist.Rebin(len(bins)-1, hist.GetName()+"_rebin", bins)
        else:
            pass

        # if "ref" exists, make ratio histograms
        if ref is not None:
            self.ratio_hists = {}
            for name, hist in self.hists.items():
                self.ratio_hists[name] = hist.Clone(f"{name}_ratio")
                self.ratio_hists[name].Divide(self.ref)
        
        ymin, ymax = 0, ref.GetMaximum()*2
        if "logy" in config.keys() and config['logy']:
            if ref.GetMinimum() > 0:
                ymin = ref.GetMinimum()*0.5
            else:
                ymin = 1.
            ymax = ref.GetMaximum()*1e3

        # make canvas
        CMS.SetEnergy(13.6)
        CMS.SetLumi("")
        CMS.SetExtraText("Simulation Preliminary")
        self.canv = CMS.cmsDiCanvas("", 
                                    config["xRange"][0], config["xRange"][-1], 
                                    ymin, ymax, 
                                    config["yRange"][0], config["yRange"][1], 
                                    config["xTitle"], config["yTitle"], 
                                    config["ratioTitle"], 
                                    square=True, 
                                    iPos=11, 
                                    extraSpace=0)
        if "logy" in config.keys() and config['logy']:
            self.canv.cd(1).SetLogy()
        self.leg = CMS.cmsLeg(0.7, 0.89 - 0.05 * 7, 0.99, 0.89, textSize=0.04, columns=1)

    def drawKinematicPad(self):
        self.canv.cd(1)
        CMS.GetcmsCanvasHist(self.canv.GetPad(1)).GetYaxis().SetMaxDigits(3)
        CMS.cmsDraw(self.ref, "PLE", fcolor=ROOT.kWhite, lcolor=ROOT.kBlack, lwidth=2, mcolor=ROOT.kBlack, msize=1.)
        self.leg.AddEntry(self.ref, "LEGACY", "PLE")
        for idx, (name, hist) in enumerate(self.hists.items()):
            CMS.cmsDraw(hist, "Hist", fcolor=ROOT.kWhite, lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed)
            CMS.cmsDraw(hist, "LE", lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed,
                                    fcolor=ROOT.kWhite, msize=0.)
            self.leg.AddEntry(hist, name, "LE")
        self.canv.cd(1).RedrawAxis()

    def drawRatioPad(self):
        self.canv.cd(2)
        CMS.GetcmsCanvasHist(self.canv.GetPad(2)).GetYaxis().SetTitleSize(0.08)
        CMS.GetcmsCanvasHist(self.canv.GetPad(2)).GetYaxis().SetTitleOffset(0.8)
        CMS.cmsDrawLine(self.ref_line, lcolor=ROOT.kBlack, lstyle=ROOT.kDotted)
        for idx, (name, hist) in enumerate(self.ratio_hists.items()):
            CMS.cmsDraw(hist, "Hist", fcolor=ROOT.kWhite, lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed)
            CMS.cmsDraw(hist, "PLE", lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed,
                                    fcolor=ROOT.kWhite, msize=0.)
        self.canv.cd(2).RedrawAxis()


class StackedCanvas():
    def __init__(self, data, hists, config):
        super().__init__()
        # store histogram and config
        self.data = data
        self.hists = hists
        self.stack = ROOT.THStack("stack", "stack")

        # Sumw2 should be called for error calculation
        self.data.Sumw2()
        for hist in self.hists.values():
            hist.Sumw2()

        if len(hists) > 6:
            self.PALLETE = PALLETE_LONG
        else:
            self.PALLETE = PALLETE

        # Set histogram binning
        if "rebin" in config.keys():
            self.data.Rebin(config["rebin"])
            for hist in self.hists.values():
                hist.Rebin(config["rebin"])
        elif len(config["xRange"]) > 2:
            # Create variable binning array from config
            bins = array('d', config["xRange"])
            # Rebin histograms with variable bin sizes
            self.data = self.data.Rebin(len(bins)-1, self.data.GetName()+"_rebin", bins)
            for name, hist in self.hists.items():
                self.hists[name] = hist.Rebin(len(bins)-1, hist.GetName()+"_rebin", bins)
        else:
            pass

        # make necessary duplicates
        self.systematics = None
        for hist in self.hists.values():
            if self.systematics is None: self.systematics = hist.Clone("syst")
            else: self.systematics.Add(hist)

        self.ratio = data.Clone("ratio")
        self.ratio.Divide(self.systematics)
        ymin = 0.
        ymax = self.systematics.GetMaximum()*2
        if "logy" in config.keys() and config['logy']:
            ymin = 1e-3
            ymax = self.systematics.GetMaximum()*1e3
  
        # Default settings
        CMS.SetEnergy(13.6)
        CMS.SetExtraText("Preliminary")
        self.canv = CMS.cmsDiCanvas("", config["xRange"][0], config["xRange"][-1], ymin, ymax, config["yRange"][0], config["yRange"][1], config["xTitle"], config["yTitle"], "Data / Pred", square=True, iPos=11, extraSpace=0)
        self.leg = CMS.cmsLeg(0.7, 0.89 - 0.05 * 7, 0.99, 0.89, textSize=0.04, columns=1)

    def drawPadUp(self):
        self.canv.cd(1)
        self.leg.AddEntry(self.data, "Data", "PE")
        CMS.cmsDrawStack(self.stack, self.leg, self.hists)
        CMS.cmsDraw(self.systematics, "E2", fcolor=ROOT.kBlack, fstyle=3004, msize=0.)
        CMS.cmsDraw(self.data, "PE", mcolor=ROOT.kBlack, msize=1.)
        self.leg.AddEntry(self.systematics, "Stat+Syst", "FE2")
        self.canv.cd(1).RedrawAxis()

    def drawPadDown(self):
        self.canv.cd(2)
        CMS.cmsDraw(self.ratio, "PE2", fcolor=ROOT.kBlack, fstyle=3004, msize=1.)
        self.canv.cd(2).RedrawAxis()