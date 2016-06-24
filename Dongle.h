#include <iostream>

#ifndef FITBIT_DONGLE_H
#define FITBIT_DONGLE_H


class Dongle {
private:
    bool initUSB();
    int releaseInterface(int interface);
public:
    Dongle();

    int read();

    int write(uint8_t * data, int dataLen);
};


#endif //FITBIT_DONGLE_H
