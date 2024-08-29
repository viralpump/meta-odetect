#include "models/ResNet10SSDFaceDetector.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>

#include <cstdlib>
#include <cstring>

ResNet10SSDFaceDetector::ResNet10SSDFaceDetector(const std::string& modelDir, const ODCaps inCaps, const void* modelData) 
    : IModelDnnDetector(inCaps)
{
    std::string modelConfiguration = modelDir + "/deploy.prototxt";
    std::string modelWeights = modelDir + "/res10_300x300_ssd_iter_140000_fp16.caffemodel";
    net = cv::dnn::readNetFromCaffe(modelConfiguration, modelWeights);

    bufferSize = inCaps.width * inCaps.height * inCaps.channels;
    buffer = new uint8_t[bufferSize];

    float conf = *static_cast<const float*>(modelData);
    modelThreshold = conf > 0 && conf <= 1 ? conf : modelThDefault;
}

std::unique_ptr<IModelDnnDetector> ResNet10SSDFaceDetector::Construct(const std::string& modelDir, const ODCaps inCaps, const void* modelData) {
    return std::make_unique<ResNet10SSDFaceDetector>(modelDir, inCaps, modelData);
}


// TODO: probably pass to Interface
ResNet10SSDFaceDetector::~ResNet10SSDFaceDetector() {
    delete[] buffer;
}

bool ResNet10SSDFaceDetector::Detect(const OdBuf inBuf, OdBuf outBuf) const {
    memcpy(buffer, inBuf, bufferSize);

    cv::Mat bgrFrame;
    InputPreProcess(buffer, bgrFrame);

    cv::Mat input_blob = cv::dnn::blobFromImage(bgrFrame, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0), false, false);
    net.setInput(input_blob);
    cv::Mat detection = net.forward();
    cv::Mat detectionMat = cv::Mat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > modelThreshold) {
            int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * bgrFrame.cols);
            int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * bgrFrame.rows);
            int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * bgrFrame.cols);
            int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * bgrFrame.rows);

            cv::rectangle(bgrFrame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 3, 8);
        }
    }

    memcpy(outBuf, bgrFrame.data, inCaps.width * inCaps.height * 3);

    return true;
}