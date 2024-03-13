#ifndef Muon_h
#define Muon_h

#include "TString.h"
#include "Lepton.h"

// Need update
// - TuneP object
// - momentum scale
// - rochester correction
// - chi2
// - truth matching info
// - Analysis dependent IDs

class Muon: public Lepton {
public:
    Muon();
    ~Muon();

    // Boolean IDs
    enum class BooleanID {NONE, LOOSE, MEDIUM, MEDIUMPROMPT, TIGHT, SOFT, SOFTMVA, TRIGGERLOOSE};

    void SetBIDBit(BooleanID id, bool value);
    inline bool isPOGTightId() const {return j_tightId;}
    inline bool isPOGMediumId() const {return j_mediumId;}
    inline bool isPOGMediumPromptId() const {return j_mediumPromptId;}
    inline bool isPOGLooseId() const {return j_looseId;}
    inline bool isPOGSoftId() const {return j_softId;}
    inline bool isPOGSoftMvaId() const {return j_softMvaId;}
    inline bool isPOGTriggerIdLoose() const {return j_triggerIdLoose;}

    // Unsigned char IDs
    enum class WorkingPointID {NONE, HIGHPT, MINIISO, MULTIISO, MVA, MVALOWPT, PFISO, PUPPIISO, TKISO};
    enum class WorkingPoint {None, VLOOSE, LOOSE, MEDIUM, TIGHT, VTIGHT, VVTIGHT};

    void SetWIDBit(WorkingPointID id, unsigned char value);
    inline WorkingPoint HighPtId() const {return (WorkingPoint)j_highPtId;}
    inline WorkingPoint MiniIsoId() const {return (WorkingPoint)j_miniIsoId;}
    inline WorkingPoint MultiIsoId() const {return (WorkingPoint)j_multiIsoId;}
    inline WorkingPoint MvaId() const {return (WorkingPoint)j_mvaId;}
    inline WorkingPoint MvaLowPtId() const {return (WorkingPoint)j_mvaLowPtId;}
    inline WorkingPoint PfIsoId() const {return (WorkingPoint)j_pfIsoId;}
    inline WorkingPoint PuppiIsoId() const {return (WorkingPoint)j_puppiIsoId;}
    inline WorkingPoint TkIsoId() const {return (WorkingPoint)j_tkIsoId;}

    // MVA ID scores
    enum class MVAID {NONE, SOFTMVA, MVALOWPT, MVATTH};

    void SetMVAID(MVAID id, float score);
    inline float SoftMva() const {return j_softMva;}
    inline float MvaLowPt() const {return j_mvaLowPt;}
    inline float MvaTTH() const {return j_mvaTTH;}


private:
    bool j_isTracker, j_isStandalone, j_isGlobal;
    bool j_looseId, j_mediumId, j_mediumPromptId, j_tightId, j_softId, j_softMvaId, j_triggerIdLoose;
    unsigned char j_highPtId, j_miniIsoId, j_multiIsoId, j_mvaId, j_mvaLowPtId, j_pfIsoId, j_puppiIsoId, j_tkIsoId;
    float j_softMva, j_mvaLowPt, j_mvaTTH;

    ClassDef(Muon, 1);
};

#endif
