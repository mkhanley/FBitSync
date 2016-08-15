#include "Network.h"

std::string buildSyncMessage(std::string encodedData, Dongle &dongle, Tracker &tracker) {
    using boost::property_tree::ptree;
    ptree tree;

    string postID = "6de4df71-17f9-43ea-9854-67f842021e05";

    ptree galileo;
    ptree info;
    ptree id;
    ptree clientVersion;
    ptree mode;
    ptree dongleVersion;
    ptree trackerTree;
    ptree data;

    galileo.put("<xmlattr>.version", "2.0");
    id.put("", postID);
    clientVersion.put("", "0.4.4");
    mode.put("","sync");
    dongleVersion.put("<xmlattr>.major", dongle.getVersionMajor());
    dongleVersion.put("<xmlattr>.minor", dongle.getVersionMinor());
    trackerTree.put("<xmlattr>.tracker-id", tracker.getIDasString());
    data.put("", encodedData);

    info.add_child("client-id", id);
    info.add_child("client-version", clientVersion);
    info.add_child("client-mode", mode);
    info.add_child("dongle-version", dongleVersion);

    trackerTree.add_child("data", data);

    galileo.add_child("client-info", info);
    galileo.add_child("tracker", trackerTree);

    tree.add_child("galileo-client", galileo);
    std::stringstream stream;
    boost::property_tree::write_xml(stream, tree,
            boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    return stream.str();
}

string sendSync(std::string xml) {
    auto r = cpr::Post(cpr::Url{"http://api.fitbit.com/tracker/client/message"},
                       cpr::Body{xml},
                       cpr::Header{{"Content-Type", "text/xml"}});
    return r.text;
}

vector<uint8_t> parseResponse(std::string xml) {
    using boost::property_tree::ptree;
    ptree pt;
    istringstream stream(xml);
    boost::property_tree::xml_parser::read_xml(stream, pt, boost::property_tree::xml_parser::trim_whitespace);
    cout << pt.get<std::string>("galileo-server.tracker.data") << endl;
    return std::vector<uint8_t>();
}




