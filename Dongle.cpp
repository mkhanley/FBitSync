//
// Created by mark on 24/06/16.
//
#include <iostream>
#include <libusb-1.0/libusb.h>

using namespace std;

class Dongle{

private:
    uint16_t venID = 9863;
    uint16_t prodID = 64257;
    libusb_device_handle *handle;


    bool initUSB(){
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

    bool detachKernel(int interface){
        bool detached = false;
        if(libusb_kernel_driver_active(handle, interface) == 1) { //find out if kernel driver is attached
            cout<<"Kernel Driver Active"<<endl;
            if(libusb_detach_kernel_driver(handle, 0) == interface) { //detach it
                cout << "Kernel Driver Detached!" << endl;
                detached = true;
            }
        }
        else
            detached = true;
        return detached;
    }

    int claimInterface(int interface){
        int res = libusb_claim_interface(handle, interface);
        if(res < 0)
            cout << "Cannot Claim Interface" << interface << endl;
        else
            cout << "Interface" << interface <<"claimed"  <<endl;
    }

public:

    Dongle(){
        if(initUSB()){
            for (int i = 0; i < 2; i++) {
                detachKernel(i);
                claimInterface(i);
            }
        }
    }

    void print(){
        std::cout << "TEST" << std::endl;
    }

    int write(){

    }

    int read(){

    }
};
