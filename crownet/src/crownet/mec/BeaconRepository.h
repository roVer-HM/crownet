#pragma once

#include "packets/SimpleMECBeacon_m.h"

#include <map>
#include <vector>

typedef struct StoredBeacon
{
   SimpleMECBeacon beacon;
};

class BeaconRepository
{
private:
    std::map<int, StoredBeacon> storage;

public:
    void add(SimpleMECBeacon);

    std::vector<SimpleMECBeacon> getAll();

};
