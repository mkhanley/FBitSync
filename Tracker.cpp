#include "Tracker.h"
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