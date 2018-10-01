#ifndef STREAMWRITER_CBFWRITER_H
#define STREAMWRITER_CBFWRITER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include "../reader/SeriesHeader.h"
#include "../reader/Image.h"

class CBFWriter
{
public:
    static string generateHeader(SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage);
    static void writeCBF(std::string path, SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage);
    static void submitTask(std::string path, SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage);
    static void init();
    static int getPendingWrites() { boost::mutex::scoped_lock(lock); return pendingWrites; }

private:
    static boost::asio::io_service *io_service;
    static boost::asio::io_service::work *work;
    static boost::thread_group *threads;
    static const int THREAD_MAX;

    static boost::mutex lock;
    static int pendingWrites;
};

#endif //STREAMWRITER_CBFWRITER_H
