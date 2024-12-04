#include "SystematicHelper.h"
#include <yaml-cpp/yaml.h>

SystematicHelper::SystematicHelper(std::string yaml_path)
{
    YAML::Node config = YAML::LoadFile(yaml_path);
    for (const auto &node : config["systematics"])
    {
        std::string syst_name = node["syst"].as<std::string>();
        std::string variation_target = node["variation_target"].as<std::string>();
        SYST syst;
        syst.syst = syst_name;
        syst.variation_target = variation_target;
        if (node["source"].IsDefined())
        {
            syst.source = node["source"].as<std::string>();
        }
        if (node["evtLoopAgain"].IsDefined())
        {
            syst.evtLoopAgain = node["evtLoopAgain"].as<bool>();
        }
        if (node["oneSided"].IsDefined())
        {
            syst.oneSided = node["oneSided"].as<bool>();
        }
        if (node["hasDedicatedSample"].IsDefined())
        {
            syst.hasDedicatedSample = node["hasDedicatedSample"].as<bool>();
        }
        if (node["dedicatedSampleKey_up"].IsDefined())
        {
            syst.dedicatedSampleKey_up = node["dedicatedSampleKey_up"].as<std::string>();
        }
        if (node["dedicatedSampleKey_down"].IsDefined())
        {
            syst.dedicatedSampleKey_down = node["dedicatedSampleKey_down"].as<std::string>();
        }
        systematics.push_back(syst);
    }

    for (const auto &node : config["correlations"])
    {
        CORRELATION correlation;
        std::string name = node["correlation"].as<std::string>();
        correlation.type = name;
        std::string rep_name;
        for (unsigned int i = 0; i < node["sources"].size(); i++)
        {
            if (i == 0)
            {
                rep_name = node["sources"][i].as<std::string>();
                correlation.rep_name = rep_name;
            }
            else
            {
                correlation.child_syst_names.insert(node["sources"][i].as<std::string>());
            }
            std::string source = node["sources"][i].as<std::string>();
            correlation.sources.push_back(findSystematic(source));
        }
        correlations[name] = correlation;
    }

    checkBadSystematics();
}

SystematicHelper::~SystematicHelper() {}

SYST *SystematicHelper::findSystematic(std::string syst_name)
{
    for (auto &syst : systematics)
    {
        if (syst.syst == syst_name)
        {
            return &syst;
        }
    }
    return nullptr;
}

void SystematicHelper::checkBadSystematics()
{
    bool badSysts = false;
    for (const auto &syst : systematics)
    {
        // check 1. if hasDedicatedSample is true, then dedicatedSampleKey_up and dedicatedSampleKey_down must be defined
        if (syst.hasDedicatedSample)
        {
            if (syst.dedicatedSampleKey_up == "" || syst.dedicatedSampleKey_down == "")
            {
                std::cerr << "Systematic " << syst.syst << " has hasDedicatedSample set to true, but dedicatedSampleKey_up and dedicatedSampleKey_down are not defined" << std::endl;
                badSysts = true;
            }
        }

        // check 2. variation_target must be defined
        if (syst.variation_target == "")
        {
            std::cerr << "Systematic " << syst.syst << " has no variation_target defined" << std::endl;
            badSysts = true;
        }

        // check 3. if evtLoopAgain is true, hasDedicatedSample must be false
        if (syst.evtLoopAgain && syst.hasDedicatedSample)
        {
            std::cerr << "Systematic " << syst.syst << " has evtLoopAgain set to true, but hasDedicatedSample is also true" << std::endl;
            badSysts = true;
        }
    }

    for (const auto &correlation : correlations)
    {
        for (const auto &source : correlation.second.sources)
        {
            if (source == nullptr)
            {
                std::cerr << "Correlation " << correlation.first << " has a source that is not defined" << std::endl;
                badSysts = true;
            }
        }
    }

    if (badSysts)
    {
        throw std::runtime_error("Bad systematics found");
        exit(1);
    }
    make_Iter_obj_EvtLoopAgain();
    make_Iter_obj_weight();

}

void SystematicHelper::make_Iter_obj_EvtLoopAgain()
{
    unordered_set<std::string> added_systematics;
    for (const auto &syst : systematics)
    {
        if(syst.syst == "Central"){
            Iter_obj obj;
            obj.name = syst.syst;
            systematics_evtLoopAgain.push_back(obj);
        }
        if (syst.evtLoopAgain)
        {
            // check if this systematic is in correlation table. If this is in correlation table and but not first one, then skip
            for (const auto &correlation : correlations)
            {
                if (correlation.second.child_syst_names.find(syst.syst) != correlation.second.child_syst_names.end())
                    continue;
            }

            Iter_obj obj_up;
            Iter_obj obj_down;
            obj_up.name = syst.syst + "_Up";
            obj_down.name = syst.syst + "_Down";
            RVec<SYST> this_systs;
            this_systs.push_back(syst);
            if (correlations.find(syst.syst) != correlations.end())
            {
                for (const auto &source : correlations[syst.syst].sources)
                {
                    this_systs.push_back(*source);
                }
            }
            for (const auto &this_syst : this_systs)
            {
                obj_up.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::up);
                if (this_syst.oneSided)
                    obj_down.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::nom);
                else
                    obj_down.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::down);
            }
            systematics_evtLoopAgain.push_back(obj_up);
            systematics_evtLoopAgain.push_back(obj_down);
        }
    }
}

void SystematicHelper::make_Iter_obj_weight()
{
    unordered_set<std::string> added_systematics;
    for (const auto &syst : systematics)
    {
        if (!syst.evtLoopAgain && !syst.hasDedicatedSample)
        {
            // check if this systematic is in correlation table. If this is in correlation table and but not first one, then skip
            for (const auto &correlation : correlations)
            {
                if (correlation.second.child_syst_names.find(syst.syst) != correlation.second.child_syst_names.end())
                    continue;
            }

            Iter_obj obj_up;
            Iter_obj obj_down;
            obj_up.name = syst.syst + "_Up";
            obj_down.name = syst.syst + "_Down";
            RVec<SYST> this_systs;
            this_systs.push_back(syst);
            if (correlations.find(syst.syst) != correlations.end())
            {
                for (const auto &source : correlations[syst.syst].sources)
                {
                    this_systs.push_back(*source);
                }
            }
            for (const auto &this_syst : this_systs)
            {
                obj_up.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::up);
                if (this_syst.oneSided)
                    obj_down.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::nom);
                else
                    obj_down.variation_target_sources_variations[this_syst.variation_target] = std::make_pair(this_syst.source, Correction::variation::down);
            }
            systematics_weight.push_back(obj_up);
            systematics_weight.push_back(obj_down);
        }
    }
}

bool SystematicHelper::IsDedicatedSample(TString sample)
{
    for (const auto &syst : systematics)
    {
        if (syst.hasDedicatedSample)
        {
            if (sample.Contains(syst.dedicatedSampleKey_up) || sample.Contains(syst.dedicatedSampleKey_down))
            {
                systematics_weight.clear();
                systematics_evtLoopAgain.clear();
                Iter_obj obj;
                obj.name = "Central";
                systematics_weight.push_back(obj);
                systematics_evtLoopAgain.push_back(obj);
                return true;
            }
        }
    }
    return false;
}