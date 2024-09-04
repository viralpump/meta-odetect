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

#include "odetect.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>

uint8_t GetChannelsByPixelFormat(OdPixelFmt fmt) {
    switch(fmt) {
        case V4L2_PIX_FMT_YUYV:
            return 2;
        case V4L2_PIX_FMT_BGR24:
            return 3;
        default:
            printf("Unsupported pixel format %u\n", fmt);
            return 0;
    }

    return 0;
}

const char* PixelFormatToString(OdPixelFmt fourcc) {
    static char fourcc_str[5];
    fourcc_str[0] = (char)(fourcc & 0xFF);
    fourcc_str[1] = (char)((fourcc >> 8) & 0xFF);
    fourcc_str[2] = (char)((fourcc >> 16) & 0xFF);
    fourcc_str[3] = (char)((fourcc >> 24) & 0xFF);
    fourcc_str[4] = '\0';
    return fourcc_str;    
}

uint8_t GetOdCapsFromVideoDev(const char* video_dev_path, ODCaps* caps) {
    int fd = open(video_dev_path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open video device");
        return 1;
    }

    if (!caps) {
        perror("Invalid ptr for caps");
        return 1;
    }

    struct v4l2_fmtdesc fmt_desc;
    memset(&fmt_desc, 0, sizeof(fmt_desc));
    fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt_desc.index = 0;

    int found = 0;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt_desc) == 0) {
        if (!(fmt_desc.flags & V4L2_FMT_FLAG_COMPRESSED)) {

            struct v4l2_frmsizeenum frmsize;
            memset(&frmsize, 0, sizeof(frmsize));
            frmsize.pixel_format = fmt_desc.pixelformat;
            frmsize.index = 0;

            if (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
                caps->width = frmsize.discrete.width;
                caps->height = frmsize.discrete.height;
                caps->pformat = fmt_desc.pixelformat;
                caps->channels = GetChannelsByPixelFormat(caps->pformat);
                found = 1;
                printf("Found RAW format: %s, %ux%u\n", PixelFormatToString(fmt_desc.pixelformat), caps->width, caps->height);
                break;
            } else {
                perror("Failed to get frame sizes");
                close(fd);
                return 1;
            }
        }
        fmt_desc.index++;
    }

    if (!found) {
        fprintf(stderr, "No suitable RAW format found\n");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}