#include <iostream>
#include <libusb-1.0/libusb.h>

using namespace std;

uint16_t venID = 9863;
uint16_t prodID = 64257;

int main() {
    libusb_init(NULL);

    // discover devices
    libusb_device **list;
    ssize_t count = libusb_get_device_list(NULL, &list);
    ssize_t i = 0;
    int err = 0;
    if (count < 0) {
        cout << "None found";
        exit(-1);
    }
    printf("\n%d\n", (int)count);
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

    return 0;
}