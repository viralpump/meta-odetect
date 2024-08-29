#ifndef RESNET10SSDFACEDETECTOR_HPP
#define RESNET10SSDFACEDETECTOR_HPP

#include "interfaces/models/IModelDnnDetector.hpp"
#include "factories/ModelFactory.hpp"

#include <opencv2/dnn.hpp>

class ResNet10SSDFaceDetector : public IModelDnnDetector {
private:
    mutable cv::dnn::Net net;
    mutable OdBuf buffer;
    uint32_t bufferSize;
    const float modelThDefault = 0.6;
    float modelThreshold;

    static std::unique_ptr<IModelDnnDetector> Construct(const std::string& modelDir, const ODCaps inCaps, const void* modelData);
    friend struct ModelFactory;
public:
    ResNet10SSDFaceDetector(const std::string& modelDir, const ODCaps inCaps, const void* modelData);
    ~ResNet10SSDFaceDetector();

    bool Detect(const OdBuf inBuf, OdBuf outBuf) const override;
};

#endif // RESNET10SSDFACEDETECTOR_HPP