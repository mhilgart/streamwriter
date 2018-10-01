#ifndef STREAMWRITER_CONFIG_H
#define STREAMWRITER_CONFIG_H

#include <map>
using namespace std;

class Config
{
public:
    static void load();
    static map<string,string> values;
};

#endif //STREAMWRITER_CONFIG_H
