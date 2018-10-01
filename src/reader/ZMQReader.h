#ifndef STREAMWRITER_ZMQREADER_H
#define STREAMWRITER_ZMQREADER_H

#include <string>
#include <pthread.h>
#include <atomic>
#include "SeriesHeader.h"
#include "Image.h"

using namespace std;

class ZMQReader
{
public:
    static void start(string _serverString);
    static void loop();

    static void handleImageHeader(SeriesHeader *imageHeader);
    static void handleImageData(boost::shared_ptr<Image> pImage);
    static void handleEndMarker();

    static std::atomic<int> lastFrameIndex;

    // these are set or cleared via HTTP requests
    static std::atomic<double> overrideOmegaStart, overrideOmegaDelta, overrideEnergy_keV;

private:
    static bool receiveMultipart();

    static zmq::context_t *ctx;
    static zmq::socket_t *pull;
    static SeriesHeader* lastHeader;
    static std::string serverString;
    static int lastFrameSeries;
};


#endif //STREAMWRITER_ZMQREADER_H
