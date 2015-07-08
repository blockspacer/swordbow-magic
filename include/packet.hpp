#ifndef PACKET_HPP
#define PACKET_HPP

#include <functional>
#include <string>
#include <type_traits>

constexpr int stringhash(const char* str, int h = 0) {
    return *str ? stringhash(str+1, (h << 1 ^ *str+1)) : h;
}

static unsigned char nextSequenceNum = 0;

//Representing objects as unsigned int is probably the easiest way
//but template is here to allow user to pass any type in the packet
//caution: objects with pointers wont work as data, obviously,
//since memory pointed to by pointers aren't the same across
//machines...
template<class Data = unsigned int>
class Packet {
private:
    unsigned int protocol;
    unsigned char sequence;
    unsigned int datasize;
    unsigned char type;

    //Dont move data any further up - it will cause invalid read of
    //protocol, sequence, type and datasize before the packet was
    //typecasted to the correct type
    Data data;
public:

    constexpr Packet(
        const unsigned int protocol,
        unsigned char type,
        const Data& data,
        const unsigned int datasize) :
        protocol(protocol),
        sequence(nextSequenceNum++),
        data(data),
        datasize(datasize),
        type(type) {
            static_assert(
                !std::is_pointer<Data>::value,
                "Sending pointers over network can't work. Try to copy the data pointed to by the pointer instead"
            );
        }

    constexpr unsigned int getProtocol() const { return protocol; }
    constexpr unsigned char getSequence() const { return sequence; }
    constexpr unsigned char getType() const { return type; }
    constexpr const Data& getData() const { return data; }
    constexpr Data& getData() { return data; }
    constexpr unsigned int getDataSize() { return datasize; }
};

#endif //PACKET_HPP