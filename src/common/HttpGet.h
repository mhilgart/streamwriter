#ifndef STREAMWRITER_HTTPGET_H
#define STREAMWRITER_HTTPGET_H

#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;
using boost::asio::ip::tcp;

class HttpGet
{
public:
    static string get(string host, string path);

private:
    static boost::asio::io_service io_service;
};


#endif //STREAMWRITER_HTTPGET_H
