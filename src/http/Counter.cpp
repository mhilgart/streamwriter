#include <chrono>
#include "Counter.h"
#include "src/common/Log.h"
#include "PathConfig.h"
#include <iostream>
#include <boost/asio.hpp>
#include <src/reader/ZMQReader.h>

using namespace std;
typedef boost::mutex::scoped_lock ScopedLock;

boost::mutex Counter::lock;
int Counter::latestCounter=0, Counter::completeCounter=0;
int Counter::counterStartValue=0;
vector<bool> Counter::writtenFrames;
bool Counter::seriesInProgress = false;

void Counter::init(){}

void Counter::setCounterStartValue(int i)
{
    ScopedLock sl(lock);

    counterStartValue=i;
    latestCounter=i;
    completeCounter=i;
}

void Counter::startSeries()
{
    ScopedLock sl(lock);
    writtenFrames.clear();
    seriesInProgress = true;
}

void Counter::endSeries()
{
    ScopedLock sl(lock);
    seriesInProgress = false;
}

void Counter::clearWritten()
{
    ScopedLock sl(lock);
    writtenFrames.clear();
}

void Counter::setImageWritten(int seriesFrameIndex)
{
    ScopedLock sl(lock);

    seriesFrameIndex -= PathConfig::getCurrentInfo().zmqReferenceIndex;
    if (seriesFrameIndex<0) {
        BOOST_LOG_TRIVIAL(error) << "seriesFrameIndex < 0 : " << seriesFrameIndex;
        return;
    }

    int minArraySize = seriesFrameIndex+1;
    while (writtenFrames.size() < minArraySize) {
        writtenFrames.push_back(false);
    }
    writtenFrames[seriesFrameIndex]=true;

    int counterValForThisFrame = counterStartValue + seriesFrameIndex + 1;
    if (counterValForThisFrame > latestCounter) latestCounter = counterValForThisFrame;

    for (int i=0;i<writtenFrames.size();i++)
    {
        int counterValForIthFrame = counterStartValue + i + 1;
        bool frameWritten = writtenFrames[i];
        if (!frameWritten) break;
        if (counterValForIthFrame > completeCounter) {
            completeCounter = counterValForIthFrame;
        }
    }
}

CounterInfo Counter::getCounterInfo()
{
    ScopedLock sl(lock);

    CounterInfo counterInfo;
    counterInfo.latestCounter = latestCounter;
    counterInfo.completeCounter = completeCounter;
    counterInfo.streamInProgress = seriesInProgress;
    return counterInfo;
}
