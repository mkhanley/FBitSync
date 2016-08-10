#ifndef FITBIT_TRACKER_H
#define FITBIT_TRACKER_H

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>

class Tracker{
private:
    uint8_t trackerID[6];
    uint8_t addressType;
    int8_t rrsi; //Signal level
    uint8_t attributes[2];
    uint8_t serviceUUID[2];
    bool recentlySynced;
public:
    Tracker();

    Tracker(uint8_t *);

    ~Tracker();

    uint8_t * getID();
    uint8_t getAddressType();
    uint8_t * getAttributes();
    uint8_t * getServiceUUID();
    std::string getIDasString();


    void printInfo();
    void printID();

};

#endif //FITBIT_TRACKER_H
