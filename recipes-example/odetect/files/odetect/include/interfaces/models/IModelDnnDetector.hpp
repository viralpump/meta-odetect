#ifndef IMODELDNNDETECTOR_HPP
#define IMODELDNNDETECTOR_HPP

#include "odetect.h"

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

#define COLOR_CVT_NONE -1

using ColorCvtId = uint32_t;

class IModelDnnDetector {
protected:
    const ODCaps inCaps;
    ColorCvtId colorConvertId;

    IModelDnnDetector(const ODCaps& inCaps);

    void InputPreProcess(const OdBuf inBuf, cv::Mat& outFrame) const;

public:
    virtual bool Detect(const OdBuf inBuf, OdBuf outBuf) const = 0;

    virtual ~IModelDnnDetector() = default;
};

#endif // IMODELDNNDETECTOR_HPP