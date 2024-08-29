#ifndef ODETECT_H
#define ODETECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOURCC_STR(fourcc) \
    ((char[5]){(char)(fourcc & 0xFF), (char)((fourcc >> 8) & 0xFF), \
               (char)((fourcc >> 16) & 0xFF), (char)((fourcc >> 24) & 0xFF), 0})

typedef uint32_t OdPixelFmt;
typedef uint8_t* OdBuf;

typedef struct ODCaps_ {
    uint16_t width;
    uint16_t height;
    OdPixelFmt pformat; //V4L2_PIX_FMT_YUV420 etc
    uint8_t channels;

} ODCaps;

const char* PixelFormatToString(OdPixelFmt fourcc);

uint8_t GetChannelsByPixelFormat(OdPixelFmt type);

uint8_t GetOdCapsFromVideoDev(const char* video_dev_path, ODCaps* caps);

#ifdef __cplusplus
}
#endif

#endif // ODETECT_H
