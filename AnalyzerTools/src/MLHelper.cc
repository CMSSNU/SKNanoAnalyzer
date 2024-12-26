#include "MLHelper.h"


// #include <torch/script.h>
#include <onnxruntime_cxx_api.h>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

// Implementation class definition
class MLHelperImpl
{
public:
    // Constructor
    MLHelperImpl(const std::string &modelPath, MLHelper::ModelType modelType)
        : modelPath_(modelPath), modelType_(modelType), env_(ORT_LOGGING_LEVEL_WARNING, "MLHelper")
    {
        if (modelType_ == MLHelper::ModelType::ONNX)
        {
            Load_ONNX_Model(modelPath_);
        }
        else if (modelType_ == MLHelper::ModelType::TORCHSCRIPT)
        {
            Load_TorchScript_Model(modelPath_);
        }
    }

    ~MLHelperImpl()
    {
        // Resources are managed by smart pointers 
    }

    void Load_TorchScript_Model(const std::string &modelPath)
    {
        throw std::runtime_error("[MLHelperImpl::Load_TorchScript_Model] TorchScript model loading is not implemented yet.");
    }

    // Load ONNX model
    void Load_ONNX_Model(const std::string &modelPath)
    {
        if (torchScriptModelLoaded_)
        {
            throw std::runtime_error("[MLHelperImpl::Load_ONNX_Model] TorchScript model is already loaded.");
        }

        try
        {
            Ort::SessionOptions sessionOptions;
            sessionOptions.SetIntraOpNumThreads(1);
            sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), sessionOptions);

            Ort::AllocatorWithDefaultOptions allocator;

            // Get input node names and shapes
            size_t numInputNodes = session_->GetInputCount();
            inputNodeNames_.reserve(numInputNodes);
            inputNodeNamesChar_.reserve(numInputNodes);
            inputShapes_.reserve(numInputNodes);

            for (size_t i = 0; i < numInputNodes; i++)
            {
                Ort::AllocatedStringPtr inputNameAllocated = session_->GetInputNameAllocated(i, allocator);
                const char *inputName = inputNameAllocated.get();
                inputNodeNames_.emplace_back(inputName);
                inputNodeNamesChar_.emplace_back(inputNodeNames_.back().c_str());

                Ort::TypeInfo typeInfo = session_->GetInputTypeInfo(i);
                auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
                inputShapes_[inputNodeNames_.back()] = tensorInfo.GetShape();
            }

            // Get output node names and shapes
            size_t numOutputNodes = session_->GetOutputCount();
            outputNodeNames_.reserve(numOutputNodes);
            outputNodeNamesChar_.reserve(numOutputNodes);
            outputShapes_.reserve(numOutputNodes);

            for (size_t i = 0; i < numOutputNodes; i++)
            {
                Ort::AllocatedStringPtr outputNameAllocated = session_->GetOutputNameAllocated(i, allocator);
                const char *outputName = outputNameAllocated.get();
                outputNodeNames_.emplace_back(outputName);
                outputNodeNamesChar_.emplace_back(outputNodeNames_.back().c_str());

                Ort::TypeInfo typeInfo = session_->GetOutputTypeInfo(i);
                auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
                outputShapes_[outputNodeNames_.back()] = tensorInfo.GetShape();
            }

            onnxModelLoaded_ = true;
            std::cout << "[MLHelperImpl::Load_ONNX_Model] ONNX model loaded successfully from " << modelPath << std::endl;
            // print model info
            std::cout << "Number of input nodes: " << numInputNodes << std::endl;
            for (size_t i = 0; i < numInputNodes; i++)
            {
                std::cout << "Input node name: " << inputNodeNamesChar_[i] << std::endl;
                std::cout << "Input node shape: ";
                for (const auto &dim : inputShapes_[inputNodeNames_[i]])
                {
                    std::cout << dim << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "Number of output nodes: " << numOutputNodes << std::endl;
            for (size_t i = 0; i < numOutputNodes; i++)
            {
                std::cout << "Output node name: " << outputNodeNamesChar_[i] << std::endl;
                std::cout << "Output node shape: ";
                for (const auto &dim : outputShapes_[outputNodeNames_[i]])
                {
                    std::cout << dim << " ";
                }
                std::cout << std::endl;
            }
        }
        catch (const Ort::Exception &e)
        {
            throw std::runtime_error("[MLHelperImpl::Load_ONNX_Model] Failed to load ONNX model: " + std::string(e.what()));
        }
    }

    // Run ONNX model
    // currrently supports float, int, bool(should be get as uint8_t)
    std::unordered_map<std::string, FloatArray> Run_ONNX_Model(
        const std::unordered_map<std::string, VariousArray> &inputDataMap,
        const std::unordered_map<std::string, IntArray> &inputDataShapeMap) const
    {
        if (!onnxModelLoaded_ || !session_)
        {
            throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] ONNX model is not loaded.");
        }

        try
        {
            if (inputNodeNames_.empty())
            {
                throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] No input nodes found in the ONNX model.");
            }

            for (const auto &inputName : inputNodeNames_)
            {
                if (inputDataMap.find(inputName) == inputDataMap.end())
                {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Missing input data for node: " + inputName);
                }

                if (inputDataShapeMap.find(inputName) == inputDataShapeMap.end())
                {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Missing input shape data for node: " + inputName);
                }
            }

            std::vector<Ort::Value> inputTensors;
            inputTensors.reserve(inputNodeNames_.size());

            for (const auto &inputName : inputNodeNames_)
            {
                const IntArray &shapeData = inputDataShapeMap.at(inputName);

                std::vector<int64_t> actualShape(shapeData.size());
                for (size_t i = 0; i < shapeData.size(); ++i)
                {
                    int64_t dim = static_cast<int64_t>(shapeData[i]);
                    if (dim <= 0)
                    {
                        throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Invalid dimension (<=0) provided for input: " + inputName);
                    }
                    actualShape[i] = dim;
                }

                size_t inputTensorSize = 1;
                for (auto dim : actualShape)
                {
                    inputTensorSize *= static_cast<size_t>(dim);
                }

                const auto &inputData = inputDataMap.at(inputName);

                std::visit([&](auto &&data){
                using T = std::decay_t<decltype(data)>;
                
                // Validate size
                if (data.size() != inputTensorSize) {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Input data size for node '" + inputName +
                                            "' does not match the user-provided shape. Expected: " +
                                            std::to_string(inputTensorSize) + ", Got: " + std::to_string(data.size()));
                }

                // Create input tensor
                Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

                if constexpr (std::is_same_v<T, FloatArray>) {
                    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
                        memoryInfo,
                        const_cast<float*>(data.data()),
                        data.size(),
                        actualShape.data(),
                        actualShape.size());
                    inputTensors.emplace_back(std::move(inputTensor));
                } else if constexpr (std::is_same_v<T, IntArray>) {
                    Ort::Value inputTensor = Ort::Value::CreateTensor<int>(
                        memoryInfo,
                        const_cast<int*>(data.data()),
                        data.size(),
                        actualShape.data(),
                        actualShape.size());
                    inputTensors.emplace_back(std::move(inputTensor));
                } else if constexpr (std::is_same_v<T, BoolArray>) {
                    Ort::Value inputTensor = Ort::Value::CreateTensor<bool>(
                        memoryInfo,
                        reinterpret_cast<bool *>(const_cast<unsigned char *>(data.data())),
                        data.size(),
                        actualShape.data(),
                        actualShape.size());
                    //check tensor content
                    const bool *boolArray = inputTensor.GetTensorData<bool>();
                    inputTensors.emplace_back(std::move(inputTensor));
                } else {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Unsupported input data type for node '" + inputName + "'.");
                } }, inputData);
            }

            // Run the model
            Ort::RunOptions runOptions;
            runOptions.SetRunLogVerbosityLevel(4); // Optional: set verbosity level
            // Execute the model
            std::vector<Ort::Value> outputTensors = session_->Run(
                runOptions,
                inputNodeNamesChar_.data(),
                inputTensors.data(),
                inputTensors.size(),
                outputNodeNamesChar_.data(),
                outputNodeNamesChar_.size());

            // Process outputs
            FloatArrays results;
            results.reserve(outputTensors.size());

            for (size_t i = 0; i < outputTensors.size(); ++i)
            {
                // Get output tensor information
                Ort::TensorTypeAndShapeInfo typeInfo = outputTensors[i].GetTensorTypeAndShapeInfo();
                size_t numElements = typeInfo.GetElementCount();

                // Get pointer to output data
                const float *floatArray = outputTensors[i].GetTensorData<float>();

                // Copy data to FloatArray
                FloatArray outputData(floatArray, floatArray + numElements);
                results.emplace_back(std::move(outputData));
            }

            std::unordered_map<std::string, FloatArray> outputDataMap;

            for(size_t i = 0; i < outputNodeNames_.size(); i++)
            {
                outputDataMap[outputNodeNames_[i]] = results[i];
            }
            return outputDataMap;
        }
        catch (const Ort::Exception &e)
        {
            throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Failed to run ONNX model: " + std::string(e.what()));
        }

        
    }

    // Getter for model type
    MLHelper::ModelType GetModelType() const
    {
        return modelType_;
    }

private:
    // Member variables
    std::string modelPath_;
    MLHelper::ModelType modelType_;

    // ONNX-related members
    bool onnxModelLoaded_ = false;
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::vector<std::string> inputNodeNames_;
    std::vector<std::string> outputNodeNames_;
    std::unordered_map<std::string, std::vector<int64_t>> inputShapes_;
    std::unordered_map<std::string, std::vector<int64_t>> outputShapes_;
    std::vector<const char *> inputNodeNamesChar_;
    std::vector<const char *> outputNodeNamesChar_;

    // TorchScript-related members
    bool torchScriptModelLoaded_ = false;
    // torch::jit::script::Module module_;
};

// MLHelper Class Method Implementations

// Constructor
MLHelper::MLHelper(const std::string &modelPath, ModelType modelType)
    : pImpl(std::make_unique<MLHelperImpl>(modelPath, modelType))
{
}

// Destructor
MLHelper::~MLHelper() = default;

// Move Constructor
MLHelper::MLHelper(MLHelper &&) noexcept = default;

// Move Assignment Operator
MLHelper &MLHelper::operator=(MLHelper &&) noexcept = default;

// Load ONNX Model
void MLHelper::Load_ONNX_Model(const std::string &modelPath)
{
    pImpl->Load_ONNX_Model(modelPath);
}

// Run ONNX Model
std::unordered_map<std::string, FloatArray> MLHelper::Run_ONNX_Model(const std::unordered_map<std::string, VariousArray> &inputDataMap, const std::unordered_map<std::string, IntArray> &inputDataShapeMap)
{
    return pImpl->Run_ONNX_Model(inputDataMap, inputDataShapeMap);
}

    // Get Model Type
MLHelper::ModelType MLHelper::GetModelType() const
{
    return pImpl->GetModelType();
}
