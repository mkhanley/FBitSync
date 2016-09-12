#include "Dongle.h"

using namespace std;

enum{
    writeDir,
    readDir
};

Message::Message(uint8_t length, int instruction, uint8_t * payload){
    this->length = length;
    this->instruction = instruction;
    if(payload != nullptr) {
        this->payload = new uint8_t[length];
        copy(&payload[0], &payload[length], this->payload);
    }
    else
        this->payload = NULL;
    this->messageData = vector<uint8_t>();
    insToArr();
}

Message::Message(uint8_t length, uint8_t * payload){
    this->length = length;
    this->payload = new uint8_t[length];
    copy(&payload[0], &payload[length], this->payload);
    this->messageData = vector<uint8_t>();
}


Message::~Message(){
    delete[] payload;
    messageData.clear();
}

void Message::insToArr(){
    insArr = static_cast<uint8_t *>(static_cast<void*>(&instruction));
}

uint8_t* Message::buildMessage() {
    if(instruction > 0xFF){ // Message to tracker
        messageData.push_back(insArr[1]);
        messageData.push_back(insArr[0]);
        messageData.insert(messageData.end(), &payload[0], &payload[length - 2]);
        while(messageData.size() < 31)
            messageData.push_back(0);
        messageData.push_back(length);
    }
    else if(!instruction){
        messageData.insert(messageData.end(), &payload[0], &payload[length]);
        while(messageData.size() < 31)
            messageData.push_back(0);
        messageData.push_back(length);
    }
    else { //Message for dongle
        messageData.push_back(length);
        messageData.push_back(insArr[0]);
        messageData.insert(messageData.end(), &payload[0], &payload[length - 2]);
    }
    return messageData.data();
}

uint8_t Message::getLength(){
    return length;
}

int Message::getInstruction(){
    return instruction;
}

string Message::asString() {
    stringstream stream;
    stream << hex;
    if(instruction > 0xFF){
        stream << (int)(*insArr) << " ";
        stream << (int)(*(insArr + 1)) << " ";
    }
    else
        stream << (int)(*insArr) << " ";
    stream << "( ";
    for (int i = 0; i < length - 2; i++) {
        stream << (int)*(payload + i) << " ";
    }
    stream << ") ";
    stream << dec << (int)length;
    return stream.str();
}

Dongle::Dongle(){
    cout << "Initialising Dongle" << endl;
    if(initUSB()){
        int detached = 0;
        for (int i = 0; i < 2; i++) {
            if(detachKernel(i)) {
                if (claimInterface(i) == 0)
                    detached++;
                else
                    initDongleError();
            }
            else
                initDongleError();
        }
        if(detached == 2){
            readData = (uint8_t *)calloc(32, 1);
            writeData = (uint8_t *)calloc(32, 1);
            string uuidStr = "adab0000-6e7d-4601-bda2-bffaa68956ba";
            uuid = boost::lexical_cast<boost::uuids::uuid>(uuidStr);
        }
        else
            initDongleError();
        cout << "Emptying dongle buffer" << endl;
        exhaustControl(); //Empty buffer in dongle
        exhaustData();
        cout << "Buffer empty \n" << endl;
    }
    else
        initDongleError();
}

Dongle::~Dongle(){
    cout << "Closing Dongle" << endl;
    for (int i = 0; i < 2; ++i) {
        releaseInterface(i);
    }
    libusb_close(handle);
}

bool Dongle::disconnect(){
    cout << "Disconnecting from connected trackers" << endl;
    Message disconnectM = Message(2,2,NULL);
    controlWrite(disconnectM);
    string expected[] = {"CancelDiscovery", "TerminateLink"};
    for (int i = 0; i < 2; i++) {
        controlRead();
        if(!compareStatus(expected[i]))
            return false;
    }
    return true;
}

void Dongle::exhaustControl(){
    int readData = 1;
    while (readData > 0)
        readData = controlRead();
}

void Dongle::exhaustData() {
    int readData = 1;
    while (readData > 0)
        readData = dataRead();
}

bool Dongle::getDongleInfo(){
    cout << "Requesting Info" << endl;
    Message message = Message(2,1,NULL);
    controlWrite(message);
    controlRead();
    if(!expectedControlMessage(22, 8))
        return false;
    versionMajor = readData[2];
    versionMinor = readData[3];
    return true;
}

vector<Tracker> Dongle::discover(){
    vector<uint8_t> payload = vector<uint8_t>();
    payload.reserve(26);
    for (int i = 15; i > -1 ; i--) {
        payload.push_back(uuid.data[i]);
    }
    payload.insert(payload.end(), {0x00, 0xfb,
                                   0x01, 0xfb,
                                   0x02, 0xfb,
                                   0xa0, 0x0f});
    Message message = Message(26, 4, payload.data());
    cout << "Starting Discovery" << endl;
    controlWrite(message);
    controlRead();
    if(!compareStatus("StartDiscovery"))
        expectedStatusError("StartDiscovery");
    vector<Tracker> trackers = vector<Tracker>();
    int numOfTrackers = 0;
    bool endOfDiscovery = false;
    while(!endOfDiscovery){
        if(controlRead()){
            if(isStatus()){

            }
            else if(readData[0] == 3 && readData[1] == 2)
                endOfDiscovery = true;
            else if(expectedControlMessage(19, 3)) {
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
    controlWrite(cancel);
    controlRead();
    if(!compareStatus("CancelDiscovery"))
        expectedStatusError("CancelDiscovery");
    return trackers;
}

bool Dongle::linkTracker(Tracker &tracker){
    //Establish Airlink
    cout << "Linking tracker ";
    tracker.printID();
    Message estLink = buildTrackerLinkMessage(tracker);
    cout << "Establishing Airlink with tracker" << endl;
    controlWrite(estLink);
    string expectedStatus[] = {"EstablishLink called...", "GAP_LINK_ESTABLISHED_EVENT"};
    uint8_t expectedPayload [][2] = {{3,4},{2,7}};
    for (int i = 0; i < 4; i++) {
        controlRead();
        if(i % 2 == 0){
            if(!compareStatus(expectedStatus[i / 2])){

            }
        }
        else{
            uint8_t *payload = expectedPayload[i / 2];
            if(!expectedControlMessage(payload[0], payload[1])){

            }
        }
    }
    //Enable TX Pipe
    cout << "Enabling TX pipe with tracker" << endl;
    if(!toggleTXPipe(0x01))
        return false;
    return setLinkParams();
}

bool Dongle::unlinkTracker(){
    cout << "Disconnecting from tracker" << endl;
    if(!toggleTXPipe(0x00))
        return false;
    Message unlink = Message(2, 7, NULL);
    controlWrite(unlink);
    string expectedStrings[] = {"TerminateLink", "", "GAP_LINK_TERMINATED_EVENT"}; //[1] = NULL to make loop logic easier
    for (int i = 0; i < 3; i++) {
        controlRead();
        if(i == 1){
            if(!expectedControlMessage(3, 5))
                return false;
        }
        else if(!compareStatus(expectedStrings[i]))
            return false;
    }
    exhaustControl();
    return true;
}

bool Dongle::isStatus(){
    return readData[1] == 0x01;
}

int Dongle::controlRead(){
    int res = libusb_bulk_transfer(handle,readControlEndpoint, readData, 32, &readDataLen, 5000);
    if(res == 0 && readDataLen > 0){
        cout<<"Read Successful!"<<endl;
        if(isStatus())
            cout << &readData[2] << endl;
        else
            controlPrint(readData, readDir);
        return readDataLen;
    }
    else {
        cout << "Read Error " << libusb_error_name(res)  << endl;
        return -1;
    }
}

int Dongle::dataRead(){
    int res = libusb_bulk_transfer(handle,readDataEndpoint, readData, 32, &readDataLen, 2000);
    if(res == 0 && readDataLen > 0){
        cout<<"Read Successful!"<<endl;
        dataPrint(readData, readDir);
        return readDataLen;
    }
    else {
        cout << "Read Error " << libusb_error_name(res) << endl;
        return -1;
    }
}

void Dongle::controlWrite(Message &message) {
    int dataWritten = 0;
    uint8_t * data = message.buildMessage();
    int dataLen = message.getLength();
    controlPrint(data, writeDir);
    int res = libusb_bulk_transfer(handle,writeControlEndpoint, data, dataLen, &dataWritten, 2000);
    if(res == 0 && dataWritten == dataLen) //we wrote the bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        writeError(message, res);
}

void Dongle::dataWrite(Message &message){
    int dataWritten = 0;
    uint8_t * data = message.buildMessage();
    int dataLen = 32; //Always write 32 bytes to tracker
    dataPrint(data, writeDir);
    int res = libusb_bulk_transfer(handle,writeDataEndpoint, data, dataLen, &dataWritten, 2000);
    if(res == 0 && dataWritten == dataLen) //we wrote the bytes successfully
        cout<<"Writing Successful!"<<endl;
    else
        writeError(message, res);
}

void Dongle::controlPrint(uint8_t *data, int direction) {
    //0 for out, 1 for in
    if(direction)
        cout << "R <-- ";
    else
        cout << "W --> ";
    cout << "( ";
    for (int i = 1; i < data[0]; ++i) {
        cout << hex << setfill('0') << setw(2) << (int)data[i] << " " ;
    }
    cout << ") - " << dec << (int)data[0] << endl;
}

void Dongle::dataPrint(uint8_t *data, int direction) {
    //0 for out, 1 for in
    if(direction)
        cout << "R <== ";
    else
        cout << "W ==> ";
    cout << "[ ";
    for (int i = 0; i < data[31]; ++i) {
        cout << hex << setfill('0') << setw(2) <<(int)data[i] << " " ;
    }
    cout << "] - " << dec << (int)data[31] << endl;
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

bool Dongle::releaseInterface(int interface){
    int res = libusb_release_interface(handle, interface);
    if(res != 0){
        cout << "Cannot release interface " << interface << endl;
        cout << "Libusb error " << libusb_error_name(res) << endl;
        return false;
    }
    return true;
}

bool Dongle::detachKernel(int interface){
    bool detached = false;
    if(libusb_kernel_driver_active(handle, interface) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(handle, interface) == 0) { //detach it
            cout << "Kernel Driver Detached!" << endl;
            detached = true;
        }
        else{
            cout << "Unable to detach kernel driver. Are you running as root?" << endl;
        }
    }
    else
        detached = true;
    return detached;
}

int Dongle::claimInterface(int interface){
    int res = libusb_claim_interface(handle, interface);
    if(res < 0) {
        cout << "Cannot Claim Interface " << interface << endl;
        cout << "USB Error " << libusb_error_name(res) << endl;
    }
    else
        cout << "Interface " << interface <<" claimed"  <<endl;
    return res;
}

bool Dongle::trackerPresent(vector<Tracker> trackers, uint8_t* ID){
    for(vector<Tracker>::iterator itt = trackers.begin(); itt != trackers.end(); itt++){
        Tracker t = *itt;
        if(memcmp(t.getID(), ID, 6) == 0)
            return true;
    }
    return false;
}

void Dongle::initDongleError() {
    cout << "Dongle was not able to be initialised...\n"
            "Exiting..." << endl;
    exit(-1);
}

void Dongle::writeError(Message message, int writeRes) {
    cout << "Error writing " << message.asString() << endl;
    cout << "Libusb error " << libusb_error_name(writeRes);
    exit(-1);
}

bool Dongle::compareStatus(string expected) {
    if(strcmp((char*)&readData[2], expected.c_str()) == 0)
        return true;
    else
        return false;
}

bool Dongle::expectedControlMessage(uint8_t length, uint8_t instruction) {
    if(readData[0] != length) {
        cout << "Received length of " << (int)readData[0] << ". Expected " << (int)length << endl;
        return false;
    }
    if(readData[1] != instruction) {
        cout << "Received instruction " << (int)readData[1] << ". Expected " << (int)instruction << endl;
        return false;
    }
    return true;
}

void Dongle::expectedStatusError(string expected) {
    cout << "Received message " << (&readData[2]) << ". Expected " << expected << endl;
    exit(-1);
}

bool Dongle::expectedDataMessage(uint8_t length, uint8_t* instruction) {
    if(readData[31] != length) {
        cout << "Received length of " << (int)readData[31] << ". Expected " << (int)length << endl;
        return false;
    }
    if(memcmp(readData, instruction, 2) != 0){
        cout << "Received instruction of " << (int)readData[1] << " " << (int)readData[2]
        << ". Expected " << (int)instruction[0] << " " << (int)instruction[1] << endl;
        return false;
    }
    return true;
}

bool Dongle::setLinkParams() {
    vector<uint8_t> params = vector<uint8_t>(10);
    unsigned short initParams[5] = {0x0a, 0x06, 0x06, 0x00, 0xc8};
    for (int i = 0; i < 5; i++) { //Vector stores values contiguously so copy values and they are reversed
        memcpy(params.data() + (i * 2), &initParams[i], sizeof(short));
    }
    Message paramsMessage = Message(12, 0xC00A, params.data());
    dataWrite(paramsMessage);
    controlRead();
    if(!expectedControlMessage(8, 6))
        return false;
    dataRead();
    uint8_t insExpected[] = {0xc0, 0x14};
    if(!expectedDataMessage(14, insExpected))
        return false;
    return true;
}

bool Dongle::toggleTXPipe(uint8_t value) {
    uint8_t txValue[] = {value};
    Message tx = Message(3, 8, txValue);
    controlWrite(tx);
    dataRead();
    uint8_t expected[] = {0xc0, 0x0b};
    return expectedDataMessage(2, expected);
}

Message Dongle::buildTrackerLinkMessage(Tracker &tracker) {
    vector<uint8_t> trackerData = vector<uint8_t>();
    trackerData.reserve(12);
    uint8_t * trackerID = tracker.getID();
    trackerData.insert(trackerData.begin(), &trackerID[0], &trackerID[6]);
    trackerData.push_back(tracker.getAddressType());
    uint8_t * serviceUUID = tracker.getServiceUUID();
    trackerData.insert(trackerData.begin()+7, &serviceUUID[0], &serviceUUID[2]);
    Message linkMessage = Message(11, 6, trackerData.data());
    return linkMessage;
}

vector<uint8_t> Dongle::getDump() {
    uint8_t dumpType = 13;
    Message getDump = Message(3, 0xC010, &dumpType);
    uint8_t expectedMessages[][2] = {{0xC0, 0x41},
                                     {0xC0, 0x42}};
    int vectSize = 50; //Try to reduce realocations
    vector<vector<uint8_t >> dump;
    dump.reserve(vectSize);
    vector<int> slipIndex = vector<int>();
    vector<uint8_t> read = vector<uint8_t>(20);
    vector<uint8_t> footer;
    dataWrite(getDump);
    dataRead();
    if (!expectedDataMessage(3, expectedMessages[0]))
        return vector<uint8_t>();
    readData[0] = 0; //Clear first byte so we enter while loop
    int bytesRead = 0;
    int count = 0;
    while (readData[0] != 0xC0) {
        dataRead();
        int length = readData[31];
        if(readData[0] != 0xC0) {
            if(length != 20)
                read.resize(length);
            copy(&readData[0], &readData[length], read.begin());
            dump.insert(dump.begin()+count, read);
            bytesRead += length;
            if(readData[0] == 0xDB)
                slipIndex.push_back(count);
            count++;
        }
        else{
            footer = vector<uint8_t>(length);
            copy(&readData[0], &readData[length], footer.begin());
        }
    }
    cout << bytesRead << endl;
    if (!expectedDataMessage(9, expectedMessages[1])){
        cout << "Unexpected end of payload" << endl;
        return vector<uint8_t>();
    }
    unslip(dump, slipIndex);
    unsigned short calculatedCRC = getCRC(dump);
    //TODO compare crc to value in footer
    vector<uint8_t> dataDump = vector<uint8_t>();
    dataDump.reserve(bytesRead);
    for(vector<vector<uint8_t>>::iterator i = (dump.begin()); i != (dump.end()); i++){
        copy((*i).begin(), (*i).end(), back_inserter(dataDump));
    }
    return dataDump;
}

void Dongle::unslip(vector<vector<uint8_t>> &dump, vector<int> &slipIndex) {
    /*
     * Removes SLIP encoding from dump
     * 0xDB DC = 0xC0
     * 0xDB DD = 0xDB
     */
    for(vector<int>::iterator i = slipIndex.begin(); i != slipIndex.end(); i++) {
        vector<uint8_t> bytes = dump.at(*i);
        if(bytes[1] == 0xDC){
            bytes.erase(bytes.begin(), bytes.begin()+2);
            bytes.insert(bytes.begin(), 0xC0);
        }
        if(bytes[1] == 0xDD){
            bytes.erase(bytes.begin(), bytes.begin()+2);
            bytes.insert(bytes.begin(), 0xDB);
        }
        dump.at(*i) = bytes;
        cout << "Unsliped " << *i << endl;
    }
}

void Dongle::slip(vector<uint8_t> &response, int index){
    response[index] = 0xDB;
    vector<uint8_t>::iterator replace = response.begin() + index;
    if(response[index] == 0xC0)
        response.insert(replace, 0xDC);
    else
        response.insert(replace, 0xDD);
}

unsigned short Dongle::getCRC(vector<vector<uint8_t>> &dump) {
    boost::crc_optimal<16, 0x1021, 0, 0x0, false, false>  crc;
    for(vector<vector<uint8_t>>::iterator i = (dump.begin()); i != (dump.end()); i++){
        crc = for_each((*i).begin(), (*i).end(), crc);
    }
    return static_cast<unsigned short>(crc());
}

uint8_t Dongle::getVersionMajor() {
    return versionMajor;
}

uint8_t Dongle::getVersionMinor() {
    return versionMinor;
}

boost::uuids::uuid Dongle::getUUID() {
    return uuid;
}

bool Dongle::startResponse(unsigned short responseLength) {
    vector<uint8_t> startBytes = vector<uint8_t>();
    startBytes.reserve(7);
    uint8_t dumpType = 4;

    uint8_t lengthArr[2];
    lengthArr[0] = static_cast<uint8_t>(responseLength & 0x00FFu);
    lengthArr[1] = static_cast<uint8_t>((responseLength & 0xFF00u) >> 8);

    //copy(&lengthArr[0], &lengthArr[1], startBytes.begin());
    startBytes.insert(startBytes.begin(), dumpType);
    startBytes.push_back(lengthArr[0]);
    startBytes.push_back(lengthArr[1]);
    startBytes.push_back(0);
    startBytes.push_back(0);
    startBytes.push_back(0);
    Message startTransfer = Message(9, 0xC024, startBytes.data());
    dataWrite(startTransfer);
    dataRead();
    uint8_t expected[] = {0xc0, 0x12};
    if(!expectedDataMessage(5, expected))
        return false;
    return true;
}

void Dongle::sendResponse(vector<uint8_t> &response){
    int length = static_cast<int>(response.size());
    cout << length << endl;
    int currentByte;
    int bytesWritten = 0;
    int writeLength = 20;
    while(bytesWritten != length){
        if((bytesWritten + writeLength) > length)
            writeLength = length - bytesWritten;
        if(response[bytesWritten] == 0xC0 || response[bytesWritten] == 0xDB)
            slip(response, bytesWritten);
        uint8_t * byteptr = (response.data()) + bytesWritten;
        Message data = Message(writeLength, byteptr);
        dataWrite(data);
        dataRead();
        bytesWritten += writeLength;
    }
}





