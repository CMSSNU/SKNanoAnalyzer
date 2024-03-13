#ifndef TestAnalyzer_h
#define TestAnalyzer_h

#include "AnalyzerCore.h"

class TestAnalyzer : public AnalyzerCore {
public:
    TestAnalyzer();
    ~TestAnalyzer();
    
    void executeEvent();
};

#endif
