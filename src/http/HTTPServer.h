#ifndef STREAMWRITER_HTTPSERVER_H
#define STREAMWRITER_HTTPSERVER_H

#include <signal.h>
#include <string>
#include <bindings/cpp/onion.hpp>
#include <bindings/cpp/response.hpp>
#include <bindings/cpp/dict.hpp>
#include <bindings/cpp/request.hpp>
#include <bindings/cpp/url.hpp>
using namespace std;

class HTTPServer
{
public:
    static void init();
    static void server_thread();
    static void shutdown(int _);
};

#endif //STREAMWRITER_HTTPSERVER_H
