#include <thread>
#include "src/http/Statistics.h"
#include <src/common/Log.h>
#include <iostream>
#include <zmq.hpp>
#include <sstream>
#include <iomanip>
#include "SeriesHeader.h"
#include "ZMQReader.h"
#include "Image.h"
#include "../writers/CBFWriter.h"
#include "src/http/PathConfig.h"
#include "../common/Util.h"
#include "src/http/Counter.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

using namespace std;

zmq::context_t *ZMQReader::ctx;
zmq::socket_t *ZMQReader::pull;
SeriesHeader *ZMQReader::lastHeader = NULL;
string ZMQReader::serverString;
int imageCount = 0;
int ZMQReader::lastFrameSeries=-1;
atomic<int> ZMQReader::lastFrameIndex;
std::atomic<double> ZMQReader::overrideOmegaStart, ZMQReader::overrideOmegaDelta, ZMQReader::overrideEnergy_keV;

void ZMQReader::start(std::string _serverString)
{
    overrideOmegaStart = std::numeric_limits<double>::quiet_NaN();
    overrideOmegaDelta = std::numeric_limits<double>::quiet_NaN();
    overrideEnergy_keV = std::numeric_limits<double>::quiet_NaN();

    serverString = _serverString;
    lastFrameIndex = -1;
    boost::thread t{loop};
}

void ZMQReader::loop()
{
    Statistics::init();
    Counter::init();
    PathConfig::init();

    ctx = new zmq::context_t(1);
    pull = new zmq::socket_t(*ctx, ZMQ_PULL);

    // connection may not be successful immediately, but ZeroMQ will keep
    // trying to connect until the server is available
    BOOST_LOG_TRIVIAL(debug) << "Connecting to " << serverString;
    pull->connect(serverString);

    long startMs = Util::getCurrentMilliseconds();
    long updateCount = 0;
    while (true)
    {
        receiveMultipart();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    //delete pull;
    //delete ctx;
}

bool ZMQReader::receiveMultipart()
{
    ZMQMultipartMessage *multipartMessage = new ZMQMultipartMessage();

    while (true)
    {
        zmq::message_t *msg = new zmq::message_t();
        try {
            pull->recv(msg, 0);
        } catch (zmq::error_t error) {
            BOOST_LOG_TRIVIAL(debug) << "ZeroMQ error: " << error.what();
            continue;
        }
        multipartMessage->addMessage(msg);
        if (!msg->more()) break;
    }

    BOOST_LOG_TRIVIAL(debug) << "Received message with " << multipartMessage->partCount() << " parts";
    for (int i=0;i<multipartMessage->partCount();i++)
    {
        string msg = Util::getZMQString(multipartMessage->getPart(i+1));
        int fullSize = multipartMessage->getPart(i+1)->size();
        if (msg.length()>200) msg = msg.substr(0,200);
        BOOST_LOG_TRIVIAL(debug) << "Message part " << (i+1) << " (" << fullSize << "B): " << msg;
    }

    switch (multipartMessage->messageType)
    {
        case ZMQMultipartMessage::HEADER:
        {
            handleImageHeader(new SeriesHeader(multipartMessage));
            break;
        }
        case ZMQMultipartMessage::IMAGE:
            handleImageData(boost::make_shared<Image>(multipartMessage));
            break;
        case ZMQMultipartMessage::END:
            handleEndMarker();
            break;
        case ZMQMultipartMessage::UNKNOWN:
            //throw std::invalid_argument("Unknown message type");
            BOOST_LOG_TRIVIAL(debug) << "Invalid message type";
            break;
    }
}

void ZMQReader::handleImageHeader(SeriesHeader *imageHeader)
{
    BOOST_LOG_TRIVIAL(debug) << "Read ZeroMQ header";
    if (NULL != lastHeader)
    {
        Counter::endSeries();
        delete lastHeader; // safe as long as all images are written before the next series header is read
        lastHeader = NULL;
    }
    lastHeader = imageHeader;

    Statistics::notifyAcquisitionStart();
    Counter::startSeries();
    PathConfig::resetZMQReferenceIndex();

    string currentUsername = PathConfig::getCurrentInfo().username;
    if (currentUsername.length() > 0)
    {
        Util::setEffectiveIDs(currentUsername);
    }
}

void ZMQReader::handleImageData(boost::shared_ptr<Image> pImage)
{
    BOOST_LOG_TRIVIAL(debug) << "Read ZeroMQ image";

    if (NULL==lastHeader) { BOOST_LOG_TRIVIAL(error) << "ERROR: Missing image header"; return ;}

    if (pImage->seriesNumber==lastFrameSeries &&
        pImage->frameIndex!=lastFrameIndex+1)
    {
        BOOST_LOG_TRIVIAL(error) << "ERROR: Missing frames.  Series " << pImage->seriesNumber << " received frame index " << pImage->frameIndex << " after frame index " << lastFrameIndex;
    }
    lastFrameIndex=pImage->frameIndex;
    lastFrameSeries=pImage->seriesNumber;

    Statistics::notifyImageReceived();

    if (!Counter::isSeriesInProgress())
    {
        // if we get more images than the image count in the header, this is a
        // multiple trigger data collection and we will expect the same amount
        // of images for this subsequent batch
        Statistics::notifyAcquisitionStart();
        Counter::setCounterStartValue(pImage->frameIndex);
        Counter::startSeries();
    }

    string filepath = PathConfig::getPathForFrame(pImage->frameIndex);
    HeaderOverride override;
    override.omegaStart = overrideOmegaStart;
    override.omegaDelta = overrideOmegaDelta;
    override.wavelength_A = 12.398 / overrideEnergy_keV;
    CBFWriter::submitTask(filepath.c_str(), lastHeader, override, pImage);
}

void ZMQReader::handleEndMarker()
{
    BOOST_LOG_TRIVIAL(debug) << "Read ZeroMQ end marker";
    Counter::endSeries();
    //HDFWriter::submitTask("", NULL, NULL);
}
