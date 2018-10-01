#include "../common/Log.h"
#include "HttpGet.h"

boost::asio::io_service HttpGet::io_service;

// adapted from Boost examples:
//
// http://www.boost.org/doc/libs/1_49_0/doc/html/boost_asio/example/http/client/sync_client.cpp
//
string HttpGet::get(string host, string path)
{
    ostringstream ret;

    try
    {
        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host.c_str(), "http");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << path << " HTTP/1.0\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid response\n";
            return "";
        }
        if (status_code != 200)
        {
            BOOST_LOG_TRIVIAL(error) << "Response returned with status code " << status_code << "\n";
            return "";
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {}
            BOOST_LOG_TRIVIAL(debug) << header;
        BOOST_LOG_TRIVIAL(debug) << "";

        // Write whatever content we already have to output.
        if (response.size() > 0)
        {
            ret << &response;
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
            BOOST_LOG_TRIVIAL(error) << &response;
        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what() << "\n";
    }

    return ret.str();
}