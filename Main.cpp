#include <iostream>
#include "Dongle.h"

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
        fbDongle.unlinkTracker(t);
    }
    cout << "Finished syncing "<< trackers.size() << " trackers" << endl;
    return 0;
}