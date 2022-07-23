#include "utils.h"
#include <fstream>

json loadJson(const std::string& filename)
{
    json j;

    while(true)
    {
        std::ifstream ifs(filename.c_str());
        if(!ifs.is_open()) {
            PFATAL("Failed to open json file %s.", filename.c_str())
            continue;
        }

#ifndef DEBUG
        j = json::parse(ifs, nullptr, false);
        if(j.is_discarded()) {
            ifs.close();
            PFATAL("Failed to parse json file %s\n", filename.c_str())
            continue;
        }
#else
        try
        {
            j = json::parse(ifs);
        }
        catch(json::parse_error& e)
        {
            PFATAL("Failed to parse json file %s\n%s\nAt Offset: %d\n", filename.c_str(), e.what(), e.byte)
            continue;
        }
#endif
        break;
    }
    return j;
}