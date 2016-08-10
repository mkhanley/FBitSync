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
        string xml = buildSyncMessage(encodedDump);
        cout << xml << endl;
        fbDongle.unlinkTracker();
    }
    cout << "Finished syncing "<< trackers.size() << " trackers" << endl;
    return 0;
}