//
// Created by mark on 01/07/16.
//

#ifndef FITBIT_TRACKER_H
#define FITBIT_TRACKER_H

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


    void printInfo();
    void printID();

};

Tracker::Tracker(){

}

Tracker::Tracker(uint8_t* initPayload){
    //Payload received from discovery
    std::copy(&initPayload[2], &initPayload[8], trackerID);
    addressType = initPayload[8];
    rrsi = initPayload[9];
    std::copy(&initPayload[11], &initPayload[13], attributes);
    if(attributes[1] == 4)
        recentlySynced = true;
    else if(attributes[1] == 6)
        recentlySynced = false;
    std::copy(&initPayload[17], &initPayload[19], serviceUUID);
}

Tracker::~Tracker(){

}

uint8_t * Tracker::getID(){
    return trackerID;
}

uint8_t Tracker::getAddressType(){
    return addressType;
}

uint8_t * Tracker::getAttributes(){
    return attributes;
}

uint8_t * Tracker::getServiceUUID(){
    return serviceUUID;
}
void Tracker::printID(){
    std::cout << "Tracker: ";
    for (int i = 0; i < 6; i++){
        //std::cout << std::hex << (int)(this->trackerID[i]) << " ";
        std::cout << std::hex << std::setfill('0') << std::setw(2)<< (int)(this->trackerID[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

void Tracker::printInfo(){
    std::cout << "Tracker ID " ;
    printID();
    std::cout << "Tracker Signal " << (int)rrsi << std::endl;
    std::cout << "Attributes " << std::hex << (int)attributes[0] << " " <<(int)attributes[1] << std::dec << std::endl;
    std::cout << "Recently Synced " << recentlySynced << std::endl;
}
#endif //FITBIT_TRACKER_H
