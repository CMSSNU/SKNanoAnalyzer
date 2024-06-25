#ifndef TestAnalyzer2_h
#define TestAnalyzer2_h

#include "AnalyzerCore.h"

class TestAnalyzer2 : public AnalyzerCore {
public:
    TestAnalyzer2();
    ~TestAnalyzer2();
    
    void executeEvent();
};

#endif
