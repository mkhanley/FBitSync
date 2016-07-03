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

class Dongle{

private:
    uint16_t venID = 9863;
    uint16_t prodID = 64257;
    libusb_device_handle *handle;
    uint8_t *readData;
    uint8_t *writeData;
    uint8_t readEndpoint = 0x82;
    uint8_t writeEndpoint = 0x02;
    int readDataLen = 0;
    boost::uuids::uuid uuid;

    bool initUSB();

    int releaseInterface(int interface);

    bool detachKernel(int interface);

    int claimInterface(int interface);

    void reverseArray(uint8_t * array, int size);

    bool trackerPresent(vector<Tracker>, uint8_t*);

public:

    Dongle();

    ~Dongle();

    bool disconnect();

    void exhaust();

    bool getDongleInfo();

    vector<Tracker> discover();

    bool linkTracker(Tracker);

    int write(uint8_t * data, int dataLen);

    bool isStatus();

    int read();

    void print(uint8_t * data);
};

Dongle::Dongle(){
    cout << "Initialising Dongle" << endl;
    if(initUSB()){
        int detached = 0;
        for (int i = 0; i < 2; i++) {
            if(detachKernel(i))
            if(claimInterface(i) == 0)
                detached++;
        }
        if(detached == 2){
            readData = (uint8_t *)calloc(32, 1);
            writeData = (uint8_t *)calloc(32, 1);
            string uuidStr = "adab0000-6e7d-4601-bda2-bffaa68956ba";
            uuid = boost::lexical_cast<boost::uuids::uuid>(uuidStr);
        }
        else{
            cout << "Error Initialising dongle" << endl;
        }
    }
    else{
        exit(-1);
    }
}

Dongle::~Dongle(){
    cout << "Closing Dongle" << endl;
    for (int i = 0; i < 2; ++i) {
        releaseInterface(i);
    }
    libusb_close(handle);
}

bool Dongle::disconnect(){
    cout << "Attempting to disconnect" << endl;
    Message disconnectM = Message(2,2,NULL);
    uint8_t * wdata = disconnectM.buildMessage();
    //uint8_t wdata[] = {0x02, 0x02};
    write(wdata, 2);
    read();
    exhaust();
    return true;
}

void Dongle::exhaust(){
    int readData = 1;
    while (readData > 0)
        readData = read();
}

bool Dongle::getDongleInfo(){
    cout << "Requesting Info" << endl;
    Message message = Message(2,1,NULL);
    uint8_t * wdata = message.buildMessage();
    //uint8_t wdata[] = {0x02, 0x01};
    write(wdata, 2);
    read();
    return true;
}

vector<Tracker> Dongle::discover(){
    vector<uint8_t> payload = vector<uint8_t>();
    payload.reserve(26);
    for (int i = 15; i > -1 ; i--) {
        payload.push_back(uuid.data[i]);
    }
    payload.push_back(0x00);
    payload.push_back(0xfb);
    payload.push_back(0x01);
    payload.push_back(0xfb);
    payload.push_back(0x02);
    payload.push_back(0xfb);
    payload.push_back(0xa0);
    payload.push_back(0x0f);
    Message message = Message(26, 4, payload.data());
    uint8_t * messageData = message.buildMessage();
    cout << "Starting Discovery" << endl;
    write(messageData, 26);
    vector<Tracker> trackers = vector<Tracker>();
    int numOfTrackers = 0;
    while(readData[1] != 0x02){
        if(read()){
            if(isStatus()){

            }
            else if(readData[1] == 0x03) {
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
    uint8_t * cancelData = cancel.buildMessage();
    write(cancelData, 2);
    read();
    return trackers;
}

bool Dongle::linkTracker(Tracker tracker){
    vector<uint8_t> trackerData = vector<uint8_t>();
    trackerData.reserve(12);
    uint8_t * trackerID = tracker.getID();
    trackerData.insert(trackerData.begin(), &trackerID[0], &trackerID[6]);
    trackerData.push_back(tracker.getAddressType());
    uint8_t * serviceUUID = tracker.getServiceUUID();
    trackerData.insert(trackerData.begin()+7, &serviceUUID[0], &serviceUUID[2]);
    Message estLink = Message(11, 6, trackerData.data());
    uint8_t * estLinkData = estLink.buildMessage();
    print(estLinkData);
    write(estLinkData, 11);
    read();
    exhaust();
    return true;
}

int Dongle::write(uint8_t * data, int dataLen){
    int dataWritten = 0;
    int res = libusb_bulk_transfer(handle,writeEndpoint, data, dataLen, &dataWritten, 2000);
    if(res == 0 && dataWritten == dataLen) //we wrote the bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        cout<<"Write Error"<<endl;
    return res;
}

bool Dongle::isStatus(){
    return readData[1] == 0x01;
}

int Dongle::read(){
    int res = libusb_bulk_transfer(handle,readEndpoint, readData, 32, &readDataLen, 2000);
    if(res == 0 && readDataLen > 0){
        cout<<"Read Successful!"<<endl;
        if(isStatus())
            cout << &readData[2] << endl;
        else
            print(readData);
        return readDataLen;
    }
    else {
        cout << "Read Error" << endl;
        return -1;
    }
}

void Dongle::print(uint8_t * data){
    cout << "Received " << (int)data[0] << " bytes" << endl;
    for (int i = 1; i < data[0]; ++i) {
        cout << hex << (int)data[i] << " " ;
    }
    cout << endl;
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

int Dongle::releaseInterface(int interface){
    int res = libusb_release_interface(handle, interface);
    return res;
}

bool Dongle::detachKernel(int interface){
    bool detached = false;
    if(libusb_kernel_driver_active(handle, interface) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(handle, interface) == 0) { //detach it
            cout << "Kernel Driver Detached!" << endl;
            detached = true;
        }
    }
    else
        detached = true;
    return detached;
}

int Dongle::claimInterface(int interface){
    int res = libusb_claim_interface(handle, interface);
    if(res < 0)
        cout << "Cannot Claim Interface" << interface << endl;
    else
        cout << "Interface " << interface <<" claimed"  <<endl;
    return res;
}

void Dongle::reverseArray(uint8_t * array, int size){
    if(size > 1){
        int end = size - 1;
        for (int i = 0; i < size/2; i++) {
            uint8_t a = array[i];
            uint8_t temp = array[end - i];
            array[size - i] = a;
            array[i] = temp;
        }
    }
}

bool Dongle::trackerPresent(vector<Tracker> trackers, uint8_t* ID){
    for(vector<Tracker>::iterator itt = trackers.begin(); itt != trackers.end(); itt++){
        Tracker t = *itt;
        if(memcmp(t.getID(), ID, 6) == 0)
            return true;
    }
    return false;
}
#endif //FITBIT_DONGLE_H
