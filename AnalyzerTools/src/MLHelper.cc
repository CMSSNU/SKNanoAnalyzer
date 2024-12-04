#include "MLHelper.h"

MLHelper::MLHelper(std::string modelPath, ModelType modelType)
{
    if(modelType == MLHelper::ModelType::TORCHSCRIPT)
        Load_TorchScript_Model(modelPath);
    else if(modelType == MLHelper::ModelType::ONNX)
        Load_ONNX_Model(modelPath);
}

MLHelper::~MLHelper()
{
    //if(modelType == MLHelper::ModelType::ONNX) delete session;
    //if(modelType == MLHelper::ModelType::TORCHSCRIPT) delete module;
}

void MLHelper::Load_TorchScript_Model(std::string modelPath)
{
    module = torch::jit::load(modelPath);
}

void MLHelper::Load_ONNX_Model(std::string modelPath)
{
    std::cout << "Not implemented yet" << std::endl;
    exit(1);
    /*Ort::SessionOptions session_options;
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    session = std::make_shared<Ort::Session>(env, modelPath.c_str(), session_options);
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    size_t num_input_nodes = session->GetInputCount();
    size_t num_output_nodes = session->GetOutputCount();
    inputNodeNames.resize(num_input_nodes);
    outputNodeNames.resize(num_output_nodes);
    inputShapes.clear();
    outputShapes.clear();
    for (size_t i = 0; i < num_input_nodes; i++)
    {
        char* input_name = session->GetInputName(i, allocator);
        inputNodeNames[i] = input_name;
        Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        std::vector<int> input_shape = tensor_info.GetShape();
        inputShapes[input_name] = input_shape;
    }
    for (size_t i = 0; i < num_output_nodes; i++)
    {
        char* output_name = session->GetOutputName(i, allocator);
        outputNodeNames[i] = output_name;
        Ort::TypeInfo type_info = session->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        std::vector<int> output_shape = tensor_info.GetShape();
        outputShapes[output_name] = output_shape;
    }*/
}