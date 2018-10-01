#include <iostream>
#include <thread>
#include <src/common/Util.h>
#include <src/common/NetStat.h>
#include <src/http/HTTPServer.h>
#include "../reader/ZMQReader.h"
#include "src/http/Counter.h"
#include "src/http/PathConfig.h"
#include "../writers/CBFWriter.h"
#include "StreamWriter.h"
#include "../common/Log.h"
#include "Config.h"
#include "src/http/Statistics.h"

using namespace std;

bool Main::inShutdown = false;

void signalHandler(int signal)
{
    BOOST_LOG_TRIVIAL(debug) << "received signal " << signal;
    exit(0);
}

void registerSignalHandler()
{
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
}

int main(int argc, char **argv)
{
    registerSignalHandler();

    Log::init(true);
    Config::load();
    NetStat::verifyNoConnections();
    CBFWriter::init(); // start writer threads
    HTTPServer::init();

    // connect to eiger
    ZMQReader reader;
    ostringstream connectionString;
    connectionString << "tcp://" << Config::values["eiger_ip"] << ":" << Config::values["eiger_port"];
    reader.start(connectionString.str());

    while (!Main::inShutdown)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    HTTPServer::shutdown(0);

    return 0;
}
