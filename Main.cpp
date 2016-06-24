#include <iostream>
#include <libusb-1.0/libusb.h>
#include <zconf.h>
#include "Dongle.cpp"

using namespace std;

uint16_t venID = 9863;
uint16_t prodID = 64257;

int main() {
    Dongle f;
    uint8_t wdata[] = {0x02, 0x02};
    f.write(wdata, 2);
    f.read();
    /*libusb_init(NULL);

    // discover devices
    libusb_device **list;
    ssize_t count = libusb_get_device_list(NULL, &list);
    ssize_t i = 0;
    int res = 0;
    if (count < 0) {
        cout << "No USB devices found";
        exit(-1);
    }
    libusb_device_handle *handle;
    handle = libusb_open_device_with_vid_pid(NULL, venID, prodID);
    if(handle == NULL)
        cout<<"Cannot open device"<<endl;
    else
        cout<<"Device Opened"<<endl;
    libusb_free_device_list(list, 1);
    if(libusb_kernel_driver_active(handle, 0) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(handle, 0) == 0) //detach it
            cout<<"Kernel Driver Detached!"<<endl;
    }
    if(libusb_kernel_driver_active(handle, 1) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(handle, 1) == 0) //detach it
            cout<<"Kernel Driver Detached!"<<endl;
    }
    else
        cout << "Kernel Driver not active" << endl;
    res = libusb_claim_interface(handle, 1);
    if(res < 0 ){
        cout << "Cannot claim interface" <<endl;
        return 1;
    }
    else
        cout << "Interface claimed" <<  endl;
    uint8_t wdata[2], *rdata;
    wdata[0] = 0x02;
    wdata[1] = 0x02;
    rdata = (uint8_t *)calloc(32, 1);
    int actual;
    res = libusb_bulk_transfer(handle,0x02, wdata, 2, &actual, 2000);
    if(res == 0 && actual == 2) //we wrote the 4 bytes successfully
    cout<<"Writing Successful!"<<endl;
    else
    cout<<"Write Error"<<endl;
    sleep(1);
    res = libusb_bulk_transfer(handle, 0x82u, rdata, 32, &actual,2000);
    printf("%s",libusb_error_name(res));

    if(res == 0 && actual > 0)
        cout<< rdata <<endl;
    sleep(2);
    wdata[0] = 0x01;
    wdata[1] = 0x01;
    //res = libusb_bulk_transfer(handle,0x02, wdata, 3, &actual, 2000);
    if(res == 0 && actual == 3) //we wrote the 4 bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        cout<<"Write Error"<<endl;
    res = libusb_bulk_transfer(handle, 0x82u, rdata, 32, &actual,2000);
    cout<< rdata <<endl;
    sleep(2);
    wdata[0] = 0x02;
    wdata[1] = 0x01;
    res = libusb_bulk_transfer(handle,0x02, wdata, 3, &actual, 2000);
    if(res == 0 && actual == 3) //we wrote the 4 bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        cout<<"Write Error"<<endl;
    res = libusb_bulk_transfer(handle, 0x82u, rdata, 32, &actual,2000);
    cout<< rdata <<endl;
    Dongle f;
    f.print();
    res = libusb_release_interface(handle, 0);
    res = libusb_release_interface(handle, 1);
    libusb_close(handle);*/
    return 0;
}