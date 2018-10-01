#include <sstream>
#include <iomanip>
#include <boost/regex.hpp>
#include <src/reader/ZMQReader.h>
#include "PathConfig.h"
#include "src/common/Util.h"
#include "src/common/Log.h"
#include "Counter.h"

PathInfo PathConfig::current;

typedef boost::mutex::scoped_lock ScopedLock;
boost::mutex PathConfig::lock;

void PathConfig::setInfo(string dir, string prefix, string username, int firstFrameLabel)
{
    ScopedLock sl(lock);

    current.dir=dir;
    current.prefix=prefix;
    current.username=username;
    current.firstFrameLabel = firstFrameLabel;
}

void PathConfig::resetZMQReferenceIndex()
{
    ScopedLock sl(lock);
    current.zmqReferenceIndex = 0;
}

void PathConfig::updateZMQReferenceForContinuingSeries()
{
    ScopedLock sl(lock);
    current.zmqReferenceIndex = ZMQReader::lastFrameIndex+1;
}

PathInfo PathConfig::getCurrentInfo()
{
    ScopedLock sl(lock);

    PathInfo ret = current;
    return ret;
}

string PathConfig::getPathForFrame(int frameIndex)
{
    ScopedLock sl(lock);

    int frameNumber = current.firstFrameLabel + frameIndex - current.zmqReferenceIndex;

    stringstream ss;
    ss << current.dir << "/" << current.prefix;
    ss << setw(6) << setfill('0') << frameNumber << ".cbf";

    return ss.str();
}

void PathConfig::init()
{
    // set defaults
    current.dir=".";
    current.prefix="streamwriter_test_";
    current.username="";
    current.firstFrameLabel = 1;
    current.zmqReferenceIndex = 0;
}
