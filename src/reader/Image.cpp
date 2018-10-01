#include <src/http/Statistics.h>
#include <src/common/Log.h>
#include "Image.h"
#include "ZMQMultipartMessage.h"
#include "../common/Util.h"
#include <bitshuffle.h>
#include <lz4.h>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <string>

using namespace std;

Image::Image(ZMQMultipartMessage *msg)
{
    zmq_msg = msg;

    pendingHDFWrite = true;
    pendingCBFWrite = true;

    Json::Reader reader;
    reader.parse(Util::getZMQString(msg->getPart(1)), header1Root, false);
    reader.parse(Util::getZMQString(msg->getPart(2)), header2Root, false);

    xPixels             = header2Root["shape"][0].asInt();
    yPixels             = header2Root["shape"][1].asInt();
    frameIndex          = header1Root["frame"].asInt();
    seriesNumber        = header1Root["series"].asInt();
    compressedData      = msg->getPart(3)->data();
    compressedDataSize  = msg->getPart(3)->size();
    uncompressedData    = NULL;

    string elementType = header2Root["type"].asString();
    if (elementType == "uint32")
    {
        bitDepth = 32;
    }
    else if (elementType == "uint16")
    {
        bitDepth = 16;
    }
    else throw std::exception();
}

Image::~Image()
{
    delete zmq_msg;
    if (NULL != uncompressedData) free(uncompressedData);
}

void Image::decompress()
{
    // allocate output buffer
    int uncompressedElements = xPixels * yPixels;
    int outputSize = uncompressedElements*(bitDepth/8);
    uncompressedData = calloc(uncompressedElements, bitDepth/8);
    uncompressedDataSize = uncompressedElements*sizeof(int);

    // read bslz4 header
    char *c_bslz4Data = (char *)compressedData;
    c_bslz4Data += 8;
    const uint32_t *uint32buf = (const uint32_t *)c_bslz4Data;
    int blockSize = (uint32_t) (ntohl(*uint32buf));
    if (blockSize<0) {
        BOOST_LOG_TRIVIAL(error) << "Invalid block size " << blockSize;
        free(uncompressedData);
        uncompressedData=NULL;
        return;
    }
    c_bslz4Data += 4;

    // decompress
    int64_t ret = bshuf_decompress_lz4
    (
        (void *)c_bslz4Data,
        uncompressedData,
        uncompressedElements,
        bitDepth/8,
        blockSize/(bitDepth/8)
    );
    if (ret<0)
    {
        BOOST_LOG_TRIVIAL(error) << "Error from bshuf_decompress_lz4: " << ret;
        free(uncompressedData);
        uncompressedData=NULL;
        return;
    }

    Statistics::notifyCompressedImageMB(compressedDataSize/((double)(1<<20)));
    Statistics::notifyUncompressedImageMB(uncompressedDataSize/((double)(1<<20)));
}

void Image::applyPixelMask(void *pixelMask, size_t maskSize)
{
    int *pixelMask32 = (int *) pixelMask;
    if (bitDepth == 32)
    {
        int *uncompressed32 = (int *) uncompressedData;
        int error_val = 0x7FFFFFFE;

        for (int i = 0; i < xPixels * yPixels; i++)
        {
            if (pixelMask32[i] == 1)
            {
                uncompressed32[i] = -1;
            } else if (pixelMask32[i] > 1)  // the pixel mask is 2, 4, 8, 16
            {
                uncompressed32[i] = -2;
            }
        }
    }
    else if (bitDepth == 16)
    {
        short *uncompressed16 = (short *) uncompressedData;
        int error_val = 0x7FFE;

        for (int i = 0; i < xPixels * yPixels; i++)
        {
            if (pixelMask32[i] == 1)
            {
                uncompressed16[i] = -1;
            } else if (pixelMask32[i] > 1)  // the pixel mask is 2, 4, 8, 16
            {
                uncompressed16[i] = -2;
            }
        }
    }
    else throw std::exception();
}
