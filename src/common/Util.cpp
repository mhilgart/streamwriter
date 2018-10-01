#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include "Util.h"
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "Log.h"
#include <string>
#include <sstream>
#include <sys/time.h>
#include <execinfo.h>

using namespace std;

bool Util::fileExists(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer)==0);
}

std::vector<char> Util::readBytes(const char *path)
{
    ifstream ifs(path, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    std::vector<char> data(pos);
    ifs.seekg(0, ios::beg);
    ifs.read(&data[0], pos);

    return data;
}

// TODO: reduce copying
string Util::getZMQString(zmq::message_t *msg)
{
    int size = msg->size();

    char *buf2 = new char[size+1];
    memcpy(buf2, msg->data(), size);
    buf2[size] = 0;
    string s(buf2);
    delete [] buf2;

    return s;
}

void Util::setEffectiveIDs(string username)
{
    struct passwd *pwd = getpwnam(username.c_str()); // don't free the returned pointer

    ::seteuid(0); ::setegid(0);

    if (NULL!=pwd)
    {
        ostringstream os;
        os << "/run/user/" << pwd->pw_uid;
        ::setegid(pwd->pw_gid);
        ::seteuid(pwd->pw_uid);
        setenv("CBF_TMP_DIR",os.str().c_str(),1);
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "ERROR: Couldn't look up user IDs for \"" << username << "\"" ;
    }
}

string Util::getHomeDir(string username)
{
    struct passwd *pwd = getpwnam(username.c_str()); // don't free the returned pointer
    if (NULL!=pwd)
    {
        return pwd->pw_dir;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "ERROR: Couldn't look up home directory for \"" << username << "\"" ;
        return NULL;
    }
}

vector<string> Util::readFile(string path)
{
    FILE *f;
    vector<string> ret;

    if ( (f=fopen(path.c_str(), "r")) == NULL)
    {
        BOOST_LOG_TRIVIAL(error) << "Couldn't open file " << path;
        return ret;
    }

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        string s(line);
        ret.push_back(s);
    }

    fclose(f);
    return ret;
}

long Util::getCurrentMilliseconds()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

void Util::printBacktrace()
{
    void *array[50];
    size_t size;
    size = backtrace(array, 50);
    BOOST_LOG_TRIVIAL(debug) << "Backtrace:";
    char **backtraceOutput = backtrace_symbols(array, size);
    for (int i=0;i<size;i++) {
        BOOST_LOG_TRIVIAL(debug) << backtraceOutput[i];
    }
    free(backtraceOutput);
}