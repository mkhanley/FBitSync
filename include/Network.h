#ifndef FITBIT_NETWORK_H
#define FITBIT_NETWORK_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include "Tracker.h"
#include "Dongle.h"
#include "cpr/cpr.h"


std::string buildSyncMessage(std::string encodedData, Dongle &dongle, Tracker &tracker);

std::string sendSync(std::string xml);

string parseResponse(std::string xml);

#endif //FITBIT_NETWORK_H
