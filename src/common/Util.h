#ifndef STREAMWRITER_UTIL_H
#define STREAMWRITER_UTIL_H

#include <string>
#include <vector>
#include <zmq.hpp>

using namespace std;

class Util
{
public:
    static bool fileExists(const char *path);
    static std::vector<char> readBytes(const char *path);
    static string getZMQString(zmq::message_t *zmq_msg);
    static string getHomeDir(string username);
    static void setEffectiveIDs(string basic_string);
    static int getBeamlineIndex();
    static vector<string> getExistingConnections();
    static vector<string> readFile(string path);
    static long getCurrentMilliseconds();
    static void printBacktrace();
};


#endif //STREAMWRITER_UTIL_H
