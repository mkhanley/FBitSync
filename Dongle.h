#include <iostream>
#include <iomanip>
#include <libusb-1.0/libusb.h>
#include <boost/uuid/uuid.hpp>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Tracker.h"

#ifndef FITBIT_DONGLE_H
#define FITBIT_DONGLE_H

using namespace std;

enum{
    writeDir,
    readDir
};

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

Message::Message(uint8_t length, int instruction, uint8_t * payload){
    this->length = length;
    this->instruction = instruction;
    this->payload = payload;
    this->messageData = vector<uint8_t>();
    insToArr();
    //buildMessage();
}

Message::~Message(){
    messageData.clear();
}

void Message::insToArr(){
    insArr = static_cast<uint8_t *>(static_cast<void*>(&instruction));
}

uint8_t* Message::buildMessage() {
    messageData.push_back(length);
    if(instruction > 0xFF){
        messageData.push_back(insArr[1]);
        messageData.push_back(insArr[0]);
    }
    else
        messageData.push_back(insArr[0]);
    messageData.insert(messageData.end(), &payload[0], &payload[length - 2]);
    return messageData.data();
}

uint8_t Message::getLength(){
    return length;
}

int Message::getInstruction(){
    return instruction;
}

string Message::asString() {
    string message;
    return 0;
}

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

    void writeError(Message message, int writeRes);

    bool compareStatus(string expected);

    bool expectedMessage(uint8_t length, uint8_t instruction);

    void expectedStatusError(string expected);

public:

    Dongle();

    ~Dongle();

    bool disconnect();

    void exhaust();

    bool getDongleInfo();

    vector<Tracker> discover();

    bool linkTracker(Tracker);

    void controlWrite(Message message);

    void dataWrite(Message message);

    bool isStatus();

    int controlRead();

    int dataRead();

    void controlPrint(uint8_t *data, int direction);

    void dataPrint(uint8_t *data, int direction);

    bool unlinkTracker(Tracker tracker);
};

Dongle::Dongle(){
    cout << "Initialising Dongle" << endl;
    if(initUSB()){
        int detached = 0;
        for (int i = 0; i < 2; i++) {
            if(detachKernel(i)) {
                if (claimInterface(i) == 0)
                    detached++;
                else
                    initDongleError();
            }
            else
                initDongleError();
        }
        if(detached == 2){
            readData = (uint8_t *)calloc(32, 1);
            writeData = (uint8_t *)calloc(32, 1);
            string uuidStr = "adab0000-6e7d-4601-bda2-bffaa68956ba";
            uuid = boost::lexical_cast<boost::uuids::uuid>(uuidStr);
        }
        else
            initDongleError();
        cout << "Emptying dongle buffer" << endl;
        exhaust(); //Empty buffer in dongle
        cout << "Buffer empty \n" << endl;
    }
    else
        initDongleError();
}

Dongle::~Dongle(){
    cout << "Closing Dongle" << endl;
    for (int i = 0; i < 2; ++i) {
        releaseInterface(i);
    }
    libusb_close(handle);
}

bool Dongle::disconnect(){
    cout << "Disconnecting from connected trackers" << endl;
    Message disconnectM = Message(2,2,NULL);
    controlWrite(disconnectM);
    string expected[] = {"CancelDiscovery", "TerminateLink"};
    for (int i = 0; i < 2; i++) {
        controlRead();
        if(!compareStatus(expected[i]))
            return false;
    }
    return true;
}

void Dongle::exhaust(){
    int readData = 1;
    while (readData > 0)
        readData = controlRead();
}

bool Dongle::getDongleInfo(){
    cout << "Requesting Info" << endl;
    Message message = Message(2,1,NULL);
    controlWrite(message);
    controlRead();
    return expectedMessage(22, 8);
}

vector<Tracker> Dongle::discover(){
    vector<uint8_t> payload = vector<uint8_t>();
    payload.reserve(26);
    for (int i = 15; i > -1 ; i--) {
        payload.push_back(uuid.data[i]);
    }
    payload.insert(payload.end(), {0x00, 0xfb,
                                   0x01, 0xfb,
                                   0x02, 0xfb,
                                   0xa0, 0x0f});
    Message message = Message(26, 4, payload.data());
    cout << "Starting Discovery" << endl;
    controlWrite(message);
    controlRead();
    if(!compareStatus("StartDiscovery"))
        expectedStatusError("StartDiscovery");
    vector<Tracker> trackers = vector<Tracker>();
    int numOfTrackers = 0;
    bool endOfDiscovery = false;
    while(!endOfDiscovery){
        if(controlRead()){
            if(isStatus()){

            }
            else if(readData[0] == 3 && readData[1] == 2)
                endOfDiscovery = true;
            else if(expectedMessage(19, 3)) {
                Tracker discoveredTracker = Tracker(readData);
                if(!trackerPresent(trackers, &readData[2])) {
                    trackers.push_back(discoveredTracker);
                    discoveredTracker.printInfo();
                    numOfTrackers++;
                }
            }
        }
    }
    if(numOfTrackers == 0){
        cout << "No trackers found" << endl;
    }
    if(numOfTrackers != trackers.size()){
        //Check array size
        //exit(-1);
        cout << trackers.size() << endl;
    }
    //Cancel Discovery
    Message cancel = Message(2, 5, NULL);
    controlWrite(cancel);
    controlRead();
    if(!compareStatus("CancelDiscovery"))
        expectedStatusError("CancelDiscovery");
    return trackers;
}

bool Dongle::linkTracker(Tracker tracker){
    //Establish Airlink
    cout << "Linking tracker ";
    tracker.printID();
    vector<uint8_t> trackerData = vector<uint8_t>();
    trackerData.reserve(12);
    uint8_t * trackerID = tracker.getID();
    trackerData.insert(trackerData.begin(), &trackerID[0], &trackerID[6]);
    trackerData.push_back(tracker.getAddressType());
    uint8_t * serviceUUID = tracker.getServiceUUID();
    trackerData.insert(trackerData.begin()+7, &serviceUUID[0], &serviceUUID[2]);
    Message estLink = Message(11, 6, trackerData.data());
    uint8_t * estLinkData = estLink.buildMessage();
    controlPrint(estLinkData, writeDir);
    controlWrite(estLink);
    controlRead();
    controlRead();
    controlRead();
    controlRead();
    //Enable TX Pipe
    uint8_t enableTX[] = {0x01};
    Message tx = Message(3, 8, enableTX);
    controlWrite(tx);
    dataRead();
    return true;
}

bool Dongle::unlinkTracker(Tracker tracker){
    cout << "Disconnecting from tracker" << endl;
    uint8_t disableTX[] = {0x00};
    Message tx = Message(3, 8, disableTX);
    controlWrite(tx);
    dataRead();
    Message unlink = Message(2, 7, NULL);
    controlWrite(unlink);
    controlRead();
    exhaust();
    return true;
}

bool Dongle::isStatus(){
    return readData[1] == 0x01;
}

int Dongle::controlRead(){
    int res = libusb_bulk_transfer(handle,readControlEndpoint, readData, 32, &readDataLen, 5000);
    if(res == 0 && readDataLen > 0){
        cout<<"Read Successful!"<<endl;
        if(isStatus())
            cout << &readData[2] << endl;
        else
            controlPrint(readData, readDir);
        return readDataLen;
    }
    else {
        cout << "Read Error " << libusb_error_name(res)  << endl;
        return -1;
    }
}

int Dongle::dataRead(){
    int res = libusb_bulk_transfer(handle,readDataEndpoint, readData, 32, &readDataLen, 2000);
    if(res == 0 && readDataLen > 0){
        cout<<"Read Successful!"<<endl;
        if(isStatus())
            cout << &readData[2] << endl;
        else
            dataPrint(readData, readDir);
        return readDataLen;
    }
    else {
        cout << "Read Error " << libusb_error_name(res) << endl;
        return -1;
    }
}

void Dongle::controlWrite(Message message) {
    int dataWritten = 0;
    uint8_t * data = message.buildMessage();
    int dataLen = message.getLength();
    controlPrint(data, writeDir);
    int res = libusb_bulk_transfer(handle,writeControlEndpoint, data, dataLen, &dataWritten, 2000);
    if(res == 0 && dataWritten == dataLen) //we wrote the bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        writeError(message, res);
}

void Dongle::dataWrite(Message message){
    int dataWritten = 0;
    uint8_t * data = message.buildMessage();
    int dataLen = message.getLength();
    dataPrint(data, writeDir);
    int res = libusb_bulk_transfer(handle,writeDataEndpoint, data, dataLen, &dataWritten, 2000);
    if(res == 0 && dataWritten == dataLen) //we wrote the bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        writeError(message, res);
}

void Dongle::controlPrint(uint8_t *data, int direction) {
    //0 for out, 1 for in
    if(direction)
        cout << "<-- ";
    else
        cout << "--> ";
    cout << "( ";
    for (int i = 1; i < data[0]; ++i) {
        cout << hex << (int)data[i] << " " ;
    }
    cout << ") - " << dec << (int)data[0] << endl;
}

void Dongle::dataPrint(uint8_t *data, int direction) {
    //0 for out, 1 for in
    if(direction)
        cout << "<== ";
    else
        cout << "==> ";
    cout << "[ ";
    for (int i = 0; i < data[31]; ++i) {
        cout << hex << (int)data[i] << " " ;
    }
    cout << "] - " << dec << (int)data[31] << endl;
}

//Private

bool Dongle::initUSB(){
    libusb_init(NULL);
    handle = libusb_open_device_with_vid_pid(NULL, venID, prodID);
    if(handle == NULL) {
        cout << "Cannot open device" << endl;
        return false;
    }
    else {
        cout << "Device Opened" << endl;
        return true;
    }
}

bool Dongle::releaseInterface(int interface){
    int res = libusb_release_interface(handle, interface);
    if(res != 0){
        cout << "Cannot release interface " << interface << endl;
        cout << "Libusb error " << libusb_error_name(res) << endl;
        return false;
    }
    return true;
}

bool Dongle::detachKernel(int interface){
    bool detached = false;
    if(libusb_kernel_driver_active(handle, interface) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(handle, interface) == 0) { //detach it
            cout << "Kernel Driver Detached!" << endl;
            detached = true;
        }
        else{
            cout << "Unable to detach kernel driver. Are you running as root?" << endl;
        }
    }
    else
        detached = true;
    return detached;
}

int Dongle::claimInterface(int interface){
    int res = libusb_claim_interface(handle, interface);
    if(res < 0) {
        cout << "Cannot Claim Interface " << interface << endl;
        cout << "USB Error " << libusb_error_name(res) << endl;
    }
    else
        cout << "Interface " << interface <<" claimed"  <<endl;
    return res;
}

bool Dongle::trackerPresent(vector<Tracker> trackers, uint8_t* ID){
    for(vector<Tracker>::iterator itt = trackers.begin(); itt != trackers.end(); itt++){
        Tracker t = *itt;
        if(memcmp(t.getID(), ID, 6) == 0)
            return true;
    }
    return false;
}

void Dongle::initDongleError() {
    cout << "Dongle was not able to be initialised...\n"
                    "Exiting..." << endl;
    exit(-1);
}

void Dongle::writeError(Message message, int writeRes) {
    cout << "Error writing " << message.asString() << endl;
    cout << "Libusb error " << libusb_error_name(writeRes);
    exit(-1);
}

bool Dongle::compareStatus(string expected) {
    if(strcmp((char*)&readData[2], expected.c_str()) == 0)
        return true;
    else
        return false;
}

bool Dongle::expectedMessage(uint8_t length, uint8_t instruction) {
    if(readData[0] != length) {
        cout << "Received length of " << (int)readData[0] << ". Expected " << (int)length << endl;
        return false;
    }
    if(readData[1] != instruction) {
        cout << "Received instruction " << (int)readData[1] << ". Expected " << (int)instruction << endl;
        return false;
    }
    return true;
}

void Dongle::expectedStatusError(string expected) {
    cout << "Received message " << (&readData[2]) << ". Expected " << expected << endl;
    exit(-1);
}


#endif //FITBIT_DONGLE_H
