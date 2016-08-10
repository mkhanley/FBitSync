#include "Network.h"

std::string buildSyncMessage(std::string encodedData){
    using boost::property_tree::ptree;
    ptree tree;

    ptree galileo;
    ptree info;
    ptree id;
    ptree clientVersion;
    ptree mode;
    ptree dongleVersion;
    ptree tracker;
    ptree data;

    galileo.put("<xmlattr>.version", "2.0");
    id.put("","TEST");
    clientVersion.put("", "0.4.4");
    mode.put("","sync");
    dongleVersion.put("<xmlattr>.major", "2");
    dongleVersion.put("<xmlattr>.minor", "5");
    tracker.put("<xmlattr>.tracker-id", "00TEST00");
    data.put("","TEST");

    info.add_child("client-id", id);
    info.add_child("client-version", clientVersion);
    info.add_child("client-mode", mode);
    info.add_child("dongle-version", dongleVersion);

    tracker.add_child("data", data);

    galileo.add_child("client-info", info);
    galileo.add_child("tracker", tracker);

    tree.add_child("galileo-client", galileo);
    std::stringstream stream;
    boost::property_tree::write_xml(stream, tree,
            boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    return stream.str();
}
