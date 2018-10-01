#ifndef STREAMWRITER_COUNTER_H
#define STREAMWRITER_COUNTER_H

#include <vector>
#include <thread>
#include <boost/thread.hpp>
#include <json.h>
using namespace std;

struct CounterInfo
{
    int latestCounter, completeCounter;
    bool streamInProgress;
};

class Counter
{
public:
    static void init();
    static void startSeries();
    static void clearWritten();
    static void endSeries();
    static void setImageWritten(int seriesFrameIndex);
    static void setCounterStartValue(int i);

    static CounterInfo getCounterInfo();
    static bool isSeriesInProgress() { return seriesInProgress; }
    static int getSeriesStartCountValue() { return seriesStartCounterValue; }

private:

    static boost::mutex lock;
    static int latestCounter, completeCounter, counterStartValue;
    static int seriesStartCounterValue, detectorIndexStart;
    static vector<bool> writtenFrames;
    static bool seriesInProgress;
};

#endif //STREAMWRITER_COUNTER_H
