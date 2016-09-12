#include <iostream>
#include "Dongle.h"
#include "Base64.h"
#include "Network.h"

using namespace std;

int main() {
    Dongle fbDongle;
    fbDongle.disconnect();
    fbDongle.getDongleInfo();
    vector<Tracker> trackers;
    trackers = fbDongle.discover();
    for(vector<Tracker>::iterator i = trackers.begin(); i != trackers.end(); i++){
        Tracker t = *i;
        fbDongle.linkTracker(t);
        vector<uint8_t> dump = fbDongle.getDump();
        string encodedDump = base64_encode(dump.data(), dump.size());
        //cout << encodedDump << endl;
        string xml = buildSyncMessage(encodedDump, fbDongle, t);
        cout << xml << endl;
        string serverResponse = sendSync(xml);
        string serverData = parseResponse(serverResponse);
        string decodedData = base64_decode(serverData);
        vector<uint8_t> responseData = vector<uint8_t>(decodedData.begin(), decodedData.end());
        fbDongle.startResponse((responseData.size()));
        fbDongle.sendResponse(responseData);
        fbDongle.unlinkTracker();
    }
    cout << "Finished syncing "<< trackers.size() << " trackers" << endl;
    return 0;
}