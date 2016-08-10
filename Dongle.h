#ifndef FITBIT_DONGLE_H
#define FITBIT_DONGLE_H

#include "Tracker.h"
#include <iostream>
#include <iomanip>
#include <libusb-1.0/libusb.h>
#include <boost/uuid/uuid.hpp>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/crc.hpp>

using namespace std;

class Message{
private:
    uint8_t length;
    int instruction;
    uint8_t *insArr;
    uint8_t * payload;
    vector<uint8_t> messageData;

    void insToArr();

public:
    Message(uint8_t length, int instruction, uint8_t * payload);

    ~Message();

    uint8_t* buildMessage();

    uint8_t getLength();
    int getInstruction();

    string asString();
};

class Dongle{

private:
    uint16_t venID = 9863;
    uint16_t prodID = 64257;
    libusb_device_handle *handle;
    uint8_t *readData;
    uint8_t *writeData;
    uint8_t readControlEndpoint = 0x82;
    uint8_t readDataEndpoint = 0x81;
    uint8_t writeControlEndpoint = 0x02;
    uint8_t writeDataEndpoint = 0x01;
    int readDataLen = 0;
    boost::uuids::uuid uuid;

    bool initUSB();

    bool releaseInterface(int interface);

    bool detachKernel(int interface);

    int claimInterface(int interface);

    bool trackerPresent(vector<Tracker>, uint8_t*);

    void initDongleError();

    Message buildTrackerLinkMessage(Tracker &tracker);

    bool setLinkParams();

    bool toggleTXPipe(uint8_t value);

    void unslip(vector<vector<uint8_t>> &dump, vector<int> &slipIndex);

    unsigned short getCRC(vector<vector<uint8_t>> &dump);

    void writeError(Message message, int writeRes);

    bool compareStatus(string expected);

    bool expectedControlMessage(uint8_t length, uint8_t instruction);

    bool expectedDataMessage(uint8_t length, uint8_t* instruction);

    void expectedStatusError(string expected);

public:

    Dongle();

    ~Dongle();

    bool disconnect();

    void exhaustControl();

    void exhaustData();

    bool getDongleInfo();

    vector<Tracker> discover();

    bool linkTracker(Tracker &tracker);

    vector<uint8_t> getDump();

    void controlWrite(Message &message);

    void dataWrite(Message &message);

    bool isStatus();

    int controlRead();

    int dataRead();

    void controlPrint(uint8_t *data, int direction);

    void dataPrint(uint8_t *data, int direction);

    bool unlinkTracker();
};
#endif //FITBIT_DONGLE_H
