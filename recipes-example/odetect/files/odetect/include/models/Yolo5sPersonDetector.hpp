#ifndef YOLOV5SFACEDETECTOR_HPP
#define YOLOV5SFACEDETECTOR_HPP

#include "interfaces/models/IModelDnnDetector.hpp"
#include "factories/ModelFactory.hpp"

#include <string>
#include <opencv2/dnn.hpp>

class Yolo5sPersonDetector : public IModelDnnDetector {
private:
    const static std::string modelName;

    mutable cv::dnn::Net net;

    OdBuf buffer;
    uint32_t bufferSize;
    const float modelThDefault = 0.3;
    float modelThreshold;
    const uint16_t m_width = 640;
    const uint16_t m_height = 640;

    float confThreshold, objThreshold;
    const float nmsThreshold = 0.5;

	const float anchors[3][6] = { {4,5,  8,10,  13,16}, {23,29,  43,55,  73,105},{146,217,  231,300,  335,433} };
	const float stride[3] = { 8.0, 16.0, 32.0 };

    void Sigmoid(cv::Mat* out, int length);
    void drawPred(float conf, int left, int top, int right, int bottom, cv::Mat& frame, std::vector<int> landmark) const;

    static std::unique_ptr<IModelDnnDetector> Construct(const std::string& modelDir, const ODCaps inCaps, const void* modelData);
    friend struct ModelFactory;

public:
    Yolo5sPersonDetector(const std::string& modelDir, const ODCaps inCaps, const void* modelData);
    ~Yolo5sPersonDetector();

    bool Detect(const OdBuf inBuf, OdBuf outBuf) const override;
};

#endif // YOLOV5SFACEDETECTOR_HPP