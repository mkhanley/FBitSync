#ifndef FITBIT_NETWORK_H
#define FITBIT_NETWORK_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>


std::string buildSyncMessage(std::string encodedData);

#endif //FITBIT_NETWORK_H
