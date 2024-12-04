#ifndef MLHELPER_H
#define MLHELPER_H

#ifdef ClassDef
#undef ClassDef
#endif
#include <torch/script.h> // PyTorch header
#include <unordered_map>
#include <vector>
#include "onnxruntime_cxx_api.h"


class MLHelper{
    public:
        enum class ModelType
        {
            ONNX,
            TORCHSCRIPT
        };

        MLHelper(std::string modelPath, ModelType modelType);
        ~MLHelper();
        void Load_TorchScript_Model(std::string modelPath);
        void Load_ONNX_Model(std::string modelPath);

        std::string modelPath;
        ModelType modelType;



    private:
        // ONNX
        Ort::Env env;
        const std::shared_ptr<Ort::Session> session;
        const std::vector<std::string> inputNodeNames;
        const std::vector<std::string> outputNodeNames;
        const std::unordered_map<std::string, std::vector<int>> inputShapes;
        const std::unordered_map<std::string, std::vector<int>> outputShapes;
        // TorchScript
        torch::jit::script::Module module;
    };


#endif
