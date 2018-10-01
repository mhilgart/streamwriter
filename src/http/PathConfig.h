#ifndef STREAMWRITER_PATHINFO_H
#define STREAMWRITER_PATHINFO_H

#include "src/common/Log.h"
#include <string>
#include <boost/thread.hpp>

using namespace std;

struct PathInfo
{
    string dir;
    string prefix;
    string username; // for file permissions
    int firstFrameLabel; // e.g. if 5, then series image #1 could have filename A1_000005.cbf
    int zmqReferenceIndex; // the ZMQ frame index corresponding to the first frame label
};

class PathConfig
{
public:
    static void init();
    static string getPathForFrame(int frameIndex);
    static PathInfo getCurrentInfo();
    static void setInfo(string dir, string prefix, string username, int firstFileNumber);
    static void resetZMQReferenceIndex();
    static void updateZMQReferenceForContinuingSeries();

private:
    static PathInfo current;
    static boost::mutex lock;
};

#endif //STREAMWRITER_PATHINFO_H
