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
public:
    Tracker();

    Tracker(uint8_t *);

    ~Tracker();

    uint8_t * getID();

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
}

Tracker::~Tracker(){

}

uint8_t * Tracker::getID(){
    return trackerID;
}

void Tracker::printID(){
    std::cout << "Tracker: ";
    for (int i = 0; i < 6; i++) {
        //std::cout << std::hex << (int)(this->trackerID[i]) << " ";
        std::cout << std::hex << std::setfill('0') << std::setw(2)<< (int)(this->trackerID[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

/*void Tracker::printInfo(){
    std::cout << "Tracker ID " << printID() endl;
}*/
#endif //FITBIT_TRACKER_H
