/*

Copyright (c) 2014-2024 Pavel Batsekin pavelbats@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

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
