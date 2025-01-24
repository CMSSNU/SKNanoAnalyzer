#ifndef GeneratorStudy_h
#define GeneratorStudy_h

#include "AnalyzerCore.h"

class GeneratorStudy : public AnalyzerCore {
public:
    void initializeAnalyzer();
    void executeEvent();

    RVec<Gen> AllGens;
    RVec<LHE> AllLHEs;

    GeneratorStudy();
    ~GeneratorStudy();
};

#endif
