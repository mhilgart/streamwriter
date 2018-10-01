#ifndef STREAMWRITER_IMAGE_H
#define STREAMWRITER_IMAGE_H

#include <json.h>
#include <boost/thread/pthread/shared_mutex.hpp>
#include "ZMQMultipartMessage.h"

class Image
{
public:
    Image(ZMQMultipartMessage *msg);
    ~Image();

    void decompress();
    void applyPixelMask(void *pixelMask, size_t maskSize);

    int xPixels, yPixels;
    int frameIndex, seriesNumber;
    int bitDepth;
    void *uncompressedData, *compressedData;
    int uncompressedDataSize, compressedDataSize;

    bool pendingHDFWrite=false, pendingCBFWrite=false;
    bool hasPendingWrites() { return pendingHDFWrite || pendingCBFWrite; }

private:
    ZMQMultipartMessage *zmq_msg;
    Json::Value header1Root, header2Root;
    boost::mutex lock;

};

#endif //STREAMWRITER_IMAGE_H
