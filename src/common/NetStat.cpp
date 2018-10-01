#include "NetStat.h"
#include "Log.h"

#include <boost/process.hpp>
#include <boost/tokenizer.hpp>

namespace bp = boost::process;
typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

vector<NetStatConnection> NetStat::getConnections(int remotePortToMatch, bool onlyMatchEstablished)
{
    bp::ipstream stdout, stderr;
    bp::child c("/usr/bin/netstat", "-npt", bp::std_out > stdout, bp::std_err > stderr);

    vector<NetStatConnection> ret;
    string line;
    while (std::getline(stdout, line))
    {
        NetStatConnection connection = parseLine(line);
        if (connection.localAddr.empty()) continue;

        if (connection.remotePort == remotePortToMatch)
        {
            if (!onlyMatchEstablished || connection.state == "ESTABLISHED")
            {
                ret.push_back(connection);
            }
        }
    }
    while (std::getline(stderr, line))
    {
        BOOST_LOG_TRIVIAL(debug) << "ERROR: " << line;
    }
    c.wait();

    return ret;
}

void NetStat::verifyNoConnections()
{
    vector<NetStatConnection> conn = getConnections(9999, true);
    BOOST_LOG_TRIVIAL(debug) << "Existing connection count: " << conn.size();
    for (NetStatConnection c : conn)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Existing connection found, exiting program.  Process: " << c.process << endl;
        exit(-1);
    }
}

NetStatConnection NetStat::parseLine(string line)
{
    NetStatConnection connection;

    boost::char_separator<char> sep(" \t:");
    tokenizer st (line, sep);
    vector<string> tokens;
    for (const auto &t: st)
    {
        tokens.push_back(t);
    }
    if (tokens.size()<=8) { return connection; }

    connection.localAddr = tokens[3];
    connection.localPort = atoi(tokens[4].c_str());
    connection.remoteAddr = tokens[5];
    connection.remotePort = atoi(tokens[6].c_str());
    connection.state = tokens[7];
    connection.process = tokens[8];

    return connection;
}