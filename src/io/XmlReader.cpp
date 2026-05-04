#include "pugixml.hpp"
#include <vector>
#include <iostream>
#include "XmlReader.h"

void tsunami_lab::io::loadXmlConfig(const std::string& file,
                std::vector<StationConfig>& stations,
                OutputConfig& output) {

    pugi::xml_document doc;

    // try loading file
    if (!doc.load_file(file.c_str())) {
        throw std::runtime_error("Cannot load XML file");
    }

    auto root = doc.child("tsunamilab");

    // go through every station that is inside the xml file
    for (auto s : root.child("stations").children("station")) {
        StationConfig sc;

        sc.name = s.attribute("name").as_string();
        sc.x = s.attribute("x").as_int();
        sc.y = s.attribute("y").as_int();

        stations.push_back(sc);
    }

    auto out = root.child("output");

    output.interval = out.child("record_intervall").text().as_double(0.5);
    output.path = out.child("path").text().as_string();
}