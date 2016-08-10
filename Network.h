#ifndef FITBIT_NETWORK_H
#define FITBIT_NETWORK_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include "Tracker.h"
#include "Dongle.h"

std::string buildSyncMessage(std::string encodedData, Dongle &dongle, Tracker &tracker);

#endif //FITBIT_NETWORK_H
