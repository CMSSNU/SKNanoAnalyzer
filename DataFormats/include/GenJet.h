#ifndef GenJet_h
#define GenJet_h

#include "Particle.h"

class GenJet : public Particle
{
public:
  GenJet();
  ~GenJet();
  // setting functions
  inline void SetGenFlavours(short pf, unsigned char hf)
  {
    j_partonFlavour = pf;
    j_hadronFlavour = hf;
  };
  //Parton flavour follows the typical PDG IDs.
  //For more info. please refer https://indico.cern.ch/event/271460/contributions/1610529/attachments/487999/
  inline short partonFlavour() const { return j_partonFlavour; };
  inline int hadronFlavour() const { return int(j_hadronFlavour); };


private:
  // Flavour
  unsigned char j_hadronFlavour;
  short j_partonFlavour;
  ClassDef(GenJet, 1)
};

#endif
