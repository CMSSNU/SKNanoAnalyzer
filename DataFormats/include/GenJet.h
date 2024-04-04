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
    j_partonFlavour = short(pf);
    j_hadronFlavour = short(hf);
  };
  inline short GetPartonFlavour() { return j_partonFlavour; };
  inline short GetHadronFlavour() { return j_hadronFlavour; };


private:
  // Flavour
  short j_hadronFlavour;
  short j_partonFlavour;
  ClassDef(GenJet, 1)
};

#endif