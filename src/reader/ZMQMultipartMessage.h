#ifndef STREAMWRITER_ZMQMULTIPARTMESSAGE_H
#define STREAMWRITER_ZMQMULTIPARTMESSAGE_H

#include <vector>
#include <zmq.hpp>

using namespace std;

class ZMQMultipartMessage
{
public:
    ZMQMultipartMessage();
    void addMessage(zmq::message_t *msg);
    ~ZMQMultipartMessage();

    enum MessageType { HEADER, IMAGE, END, UNKNOWN };
    MessageType messageType = UNKNOWN;

    zmq::message_t *getPart(int partNumber);
    int partCount() { return messageParts.size(); }

private:
    vector<zmq::message_t *> messageParts;
};


#endif //STREAMWRITER_ZMQMULTIPARTMESSAGE_H
