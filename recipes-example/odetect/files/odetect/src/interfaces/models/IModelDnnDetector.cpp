#include "interfaces/models/IModelDnnDetector.hpp"

#include <stdexcept>
#include <linux/videodev2.h>

IModelDnnDetector::IModelDnnDetector(const ODCaps& inCaps)
    : inCaps(inCaps)
{
    if (inCaps.pformat == V4L2_PIX_FMT_BGR24) {
        colorConvertId = COLOR_CVT_NONE;
    } else if (inCaps.pformat == V4L2_PIX_FMT_YUYV) {
        colorConvertId = cv::COLOR_YUV2BGR_YUY2;
    } else {
        throw std::runtime_error("The specified input pixel type are not supported");
    }
}

void IModelDnnDetector::InputPreProcess(const OdBuf inBuf, cv::Mat& outFrame) const {
    cv::Mat inFrame(inCaps.height, inCaps.width, CV_MAKETYPE(CV_8U, inCaps.channels), inBuf);
    if (colorConvertId == COLOR_CVT_NONE) {
        outFrame = inFrame;
    } else {
        cv::cvtColor(inFrame, outFrame, colorConvertId);
    }
}
