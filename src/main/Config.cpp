#include "Config.h"
#include "../common/Log.h"

map<string,string> Config::values;

void Config::load()
{
    FILE *f = fopen("/etc/streamwriter.conf", "r");
    if (f==NULL) {
        BOOST_LOG_TRIVIAL(error) << "Error opening /etc/streamwriter.conf";
        exit(-1);
    }
    char buf[100], key[100], value[100];
    while (fgets(buf, 100, f)) {
        int ret = sscanf(buf, "%s %s", key, value);
        if (ret<2) break;
        BOOST_LOG_TRIVIAL(debug) << "Config: " << key << " = " << value;
        values[key]=value;
    }
    fclose(f);
}
