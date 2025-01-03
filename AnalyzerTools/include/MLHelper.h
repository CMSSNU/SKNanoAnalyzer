// MLHelper.h
#ifndef MLHELPER_H
#define MLHELPER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
// Forward declaration of the implementation class
class MLHelperImpl;

// Type aliases for clarity
using FloatArray = std::vector<float>;
using FloatArrays = std::vector<FloatArray>;
using IntArray = std::vector<int>;
using BoolArray = std::vector<uint8_t>;
using VariousArray = std::variant<FloatArray, IntArray, BoolArray>;

class MLHelper
{
public:
    // Enumeration for model types
    enum class ModelType
    {
        ONNX,
        TORCHSCRIPT
    };

    // Constructor and Destructor
    MLHelper(const std::string &modelPath, ModelType modelType);
    ~MLHelper();

    // Delete copy constructor and copy assignment operator
    MLHelper(const MLHelper &) = delete;
    MLHelper &operator=(const MLHelper &) = delete;

    // Implement move semantics
    MLHelper(MLHelper &&) noexcept;
    MLHelper &operator=(MLHelper &&) noexcept;

    // Load model methods
    // void Load_TorchScript_Model(const std::string &modelPath);
    void Load_ONNX_Model(const std::string &modelPath);
    // Run model methods
    std::unordered_map<std::string, FloatArray> Run_ONNX_Model(const std::unordered_map<std::string, VariousArray> &inputDataMap, const std::unordered_map<std::string, IntArray> &inputDataShapeMap);
    // FloatArrays Run_TorchScript_Model(const FloatArray &inputData);

    // Getter for the model type
    ModelType GetModelType() const;

private:
    // Pointer to the implementation
    std::unique_ptr<MLHelperImpl> pImpl;
};

#endif // MLHELPER_H
