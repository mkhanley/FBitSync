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
    uint8_t *readData;
    uint8_t *writeData;
    uint8_t readEndpoint = 0x82;
    uint8_t writeEndpoint = 0x02;



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

    int releaseInterface(int interface){
        int res = libusb_release_interface(handle, interface);
        return res;
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
            cout << "Interface " << interface <<" claimed"  <<endl;
        return res;
    }

public:

    Dongle(){
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
            }
            else{
                cout << "Error Initialising dongle" << endl;
            }
        }
    }

    ~Dongle(){
        cout << "Closing Dongle" << endl;
        for (int i = 0; i < 2; ++i) {
            releaseInterface(i);
        }
        libusb_close(handle);
    }

    int write(uint8_t * data, int dataLen){
        int dataWritten = 0;
        int res = libusb_bulk_transfer(handle,writeEndpoint, data, dataLen, &dataWritten, 2000);
        if(res == 0 && dataWritten == 2) //we wrote the 4 bytes successfully
            cout<<"Writing Successful!"<<endl;
        else
            cout<<"Write Error"<<endl;
        return res;
    }

    int read(){
        int dataRead = 0;
        int res = libusb_bulk_transfer(handle,readEndpoint, readData, 32, &dataRead, 2000);
        if(res == 0 && dataRead == 32) //we wrote the 4 bytes successfully
            cout<<"Read Successful!"<<endl;
        else
            cout<<"Read Error"<<endl;
        return res;
    }
};
