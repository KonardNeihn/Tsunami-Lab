#ifndef TSUNAMI_LAB_IO_XML_READER
#define TSUNAMI_LAB_IO_XML_READER

#include <vector>
#include <string>

namespace tsunami_lab {
  namespace io {
    struct StationConfig {
    std::string name;
    int x;
    int y;
    };

    struct OutputConfig {
        double interval;
        std::string path;
    };

    void loadXmlConfig(const std::string& file,
                    std::vector<StationConfig>& stations,
                    OutputConfig& output);
  }
}

#endif