// This statistics class is not currently used

#ifndef STREAMWRITER_STATISTICS_H
#define STREAMWRITER_STATISTICS_H

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>

// Stats are saved per-dataset and in a circular buffer
// They are not yet available via HTTP
//
// ZeroMQ read buffer size
// incoming compression ratio: bslz4 size to uncompressed
// messages received: not parts, just real-time message count
// messages discarded: due to parsing error or missing header
// MB received
// Write threads active

class Statistics
{
public:
    static void init();
    static void loop();
    static void send(); // to be called each second

    // These stats are accumulated here and then reported each second
    // (read buffer size and write thread count are queried at each send() call)
    static void notifyAcquisitionStart();
    static void notifyImageReceived();
    static void notifyUncompressedImageMB(double mb);
    static void notifyCompressedImageMB(double mb);
    static void notifyMessageReceived();
    static void notifyMessageDiscarded();

private:
    // notifications are sent from a different thread than
    // send() is called from
    static boost::mutex lock;

    // the lock protects all of these variables
    static int imageCount, messagesReceived, messagesDiscarded;
    static double uncompressedMB, compressedMB;

    // these aren't protected because they're only accessed by send()
    static boost::circular_buffer<double> compressionRatioHistory, mbReceivedHistory;
    static boost::circular_buffer<int>
            messagesReceivedHistory, messagesDiscardedHistory, writeThreadsHistory;

    static long startMs;
};


#endif //STREAMWRITER_STATISTICS_H
