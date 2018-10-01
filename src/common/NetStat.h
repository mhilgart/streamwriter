// Used to make sure no other Eiger ZeroMQ clients are running on this machine

#ifndef STREAMWRITER_NETSTAT_H
#define STREAMWRITER_NETSTAT_H

#include <string>
#include <vector>
using namespace std;

struct NetStatConnection
{
    string localAddr;
    int localPort;
    string remoteAddr;
    int remotePort;
    string state;
    string process;
};

class NetStat
{
public:
    static vector<NetStatConnection> getConnections(int remotePortToMatch, bool onlyMatchEstablished);
    static void verifyNoConnections();

private:
    static NetStatConnection parseLine(string line);
};

#endif //STREAMWRITER_NETSTAT_H
