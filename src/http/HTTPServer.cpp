#include "HTTPServer.h"
#include "PathConfig.h"
#include "../common/Log.h"
#include <json.h>
#include <src/reader/ZMQReader.h>
#include "../http/Counter.h"
#include "../main/Config.h"

onion_connection_status post_data(void *_, onion_request *req, onion_response *res)
{
	if (onion_request_get_flags(req)&OR_HEAD)
	{
		onion_response_write_headers(res);
		return OCS_PROCESSED;
	}
	const char *user_data=onion_request_get_post(req,"text");
	onion_response_printf(res, "The user wrote: %s", user_data);
	return OCS_PROCESSED;
}

void HTTPServer::init()
{
    boost::thread t{server_thread};
}

void HTTPServer::server_thread()
{
    Onion::Onion server(O_POOL);
    Onion::Url root(&server);

    root.add("set_config", [](Onion::Request &req, Onion::Response &res)
    {
        try
        {
            Onion::Dict dict = req.post();
            string dir = dict["dir"];
            string prefix = dict["prefix"];

            string username = "";
            int firstFrameLabel = 1;
            if (dict.has("username")) username = dict["username"];
            if (dict.has("first_frame_label")) firstFrameLabel = stoi(dict["first_frame_label"]);
            PathConfig::setInfo(dir, prefix, username, firstFrameLabel);

            if (dict.has("counter_start_value"))
            {
                string counterStartValue = dict["counter_start_value"];
                Counter::setCounterStartValue(stoi(counterStartValue));
            }

            bool continueSeries = false;
            if (dict.has("continue_series")) continueSeries=(stoi(dict["continue_series"])==1);
            if (continueSeries) {
                PathConfig::updateZMQReferenceForContinuingSeries();
                Counter::clearWritten();
            }

            if (dict.has("omega_start")) {
                ZMQReader::overrideOmegaStart = stod(dict["omega_start"]);
            } else {
                ZMQReader::overrideOmegaStart = std::numeric_limits<double>::quiet_NaN();
            }
            if (dict.has("omega_delta")) {
                ZMQReader::overrideOmegaDelta = stod(dict["omega_delta"]);
            } else {
                ZMQReader::overrideOmegaDelta = std::numeric_limits<double>::quiet_NaN();
            }
            if (dict.has("energy_keV")) {
                ZMQReader::overrideEnergy_keV = stod(dict["energy_keV"]);
            } else {
                ZMQReader::overrideEnergy_keV = std::numeric_limits<double>::quiet_NaN();
            }

        } catch (const exception &t) {
            BOOST_LOG_TRIVIAL(debug) << "Failed updating path info: " << t.what();
        }
        return OCS_PROCESSED;
    });
    root.add("counter", [](Onion::Request &req, Onion::Response &res)
    {
        // TODO: add rate-limiting to ~100Hz, send server-busy otherwise?
        // TODO: or, cache counter at 10Hz?
        
        Json::Value root, counter, path;
        
        // counter
        CounterInfo counterInfo = Counter::getCounterInfo();
        counter["latestCounter"] = counterInfo.latestCounter;
        counter["completeCounter"] = counterInfo.completeCounter;
        counter["streamInProgress"] = counterInfo.streamInProgress;
        root["counter"] = counter;

        // path info
        PathInfo pathInfo = PathConfig::getCurrentInfo();
        path["firstFramePath"]=PathConfig::getPathForFrame(0);
        path["prefix"]=pathInfo.prefix;
        path["dir"]=pathInfo.dir;
        path["username"]=pathInfo.username;
        path["firstFrameLabel"]=pathInfo.firstFrameLabel;
        path["zmqReferenceIndex"]=pathInfo.zmqReferenceIndex;
        root["path"]=path;
        
        // write to string
        Json::StyledWriter writer;
        res << writer.write(root);

        return OCS_PROCESSED;
    });

    string host = Config::values["http_listen_ip"];
    int port = atoi(Config::values["http_listen_port"].c_str());
    BOOST_LOG_TRIVIAL(debug) << "Listening for HTTP on " << host << " port " << port;

    server.setHostname(host);
    server.setPort(port);
    server.listen();
}

void HTTPServer::shutdown(int _)
{
}
