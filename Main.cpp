#include <iostream>
#include "Dongle.cpp"
#include "Base64.cpp"

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
        fbDongle.getDump();
        fbDongle.unlinkTracker();
    }
    cout << "Finished syncing "<< trackers.size() << " trackers" << endl;
    return 0;
}