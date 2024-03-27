#include "ExampleParameter.h"

ExampleParameter::ExampleParameter() {
    syst = SYST::Central;
}
ExampleParameter::~ExampleParameter() {}

void ExampleParameter::Clear() {
    AnalyzerParameter::Clear();
    AdditionalParameter = "";
    syst = SYST::Central;
}