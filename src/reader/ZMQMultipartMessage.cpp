#include "ZMQMultipartMessage.h"
#include "../common/Util.h"

ZMQMultipartMessage::ZMQMultipartMessage()
{
    messageType = UNKNOWN;
}

void ZMQMultipartMessage::addMessage(zmq::message_t *msg)
{
    messageParts.push_back(msg);

    // parse the first part in each message to determine the message type
    if (messageParts.size()==1)
    {
        string header = Util::getZMQString(msg);

        // TODO: use JSON parsing
        if (NULL!=strstr(header.c_str(),"dheader-1.0"))        messageType=MessageType::HEADER;
        if (NULL!=strstr(header.c_str(),"dimage-1.0"))         messageType=MessageType::IMAGE;
        if (NULL!=strstr(header.c_str(),"dseries_end-1.0"))    messageType=MessageType::END;
    }
}

ZMQMultipartMessage::~ZMQMultipartMessage()
{
    for (int i=0;i<messageParts.size();i++)
    {
        delete messageParts[i];
    }
}

zmq::message_t *ZMQMultipartMessage::getPart(int partNumber)
{
    return messageParts[partNumber-1];
}
