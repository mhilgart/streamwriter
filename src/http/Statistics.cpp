#include "Statistics.h"
#include "src/writers/CBFWriter.h"
#include <thread>
#include "src/common/Util.h"

typedef boost::mutex::scoped_lock ScopedLock;

boost::mutex Statistics::lock;
int Statistics::imageCount;
int Statistics::messagesReceived, Statistics::messagesDiscarded;
double Statistics::uncompressedMB, Statistics::compressedMB;
long Statistics::startMs=0;
boost::circular_buffer<double> Statistics::compressionRatioHistory(60);
boost::circular_buffer<double> Statistics::mbReceivedHistory(60);
boost::circular_buffer<int> Statistics::messagesReceivedHistory(60);
boost::circular_buffer<int> Statistics::messagesDiscardedHistory(60);
boost::circular_buffer<int> Statistics::writeThreadsHistory(60);

void Statistics::init()
{
    startMs = Util::getCurrentMilliseconds();
    for (int i = 0; i < 60; i++)
    {
        compressionRatioHistory.push_back(0);
        mbReceivedHistory.push_back(0);
        messagesReceivedHistory.push_back(0);
        messagesDiscardedHistory.push_back(0);
        writeThreadsHistory.push_back(0);
    }
    boost::thread t{loop};
}

void Statistics::loop()
{
    long startMs = Util::getCurrentMilliseconds();
    long updateCount = 0;
    while (true)
    {
        long nowMs = Util::getCurrentMilliseconds();
        long sinceStartMs = nowMs-startMs;
        if (sinceStartMs > updateCount*1000 + 1000)
        {
            Statistics::send();
            updateCount = sinceStartMs/1000;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void Statistics::send()
{
}

void Statistics::notifyAcquisitionStart()
{
    ScopedLock sl(lock);

    imageCount=0;
    messagesReceived=0;
    messagesDiscarded=0;
    uncompressedMB=0;
    compressedMB=0;
}

void Statistics::notifyImageReceived()                 { ScopedLock sl(lock); imageCount++; }
void Statistics::notifyUncompressedImageMB(double mb)  { ScopedLock sl(lock); uncompressedMB += mb; }
void Statistics::notifyCompressedImageMB(double mb)    { ScopedLock sl(lock); compressedMB += mb; }
void Statistics::notifyMessageReceived()               { ScopedLock sl(lock); messagesReceived++; }
void Statistics::notifyMessageDiscarded()              { ScopedLock sl(lock); messagesDiscarded++; }
