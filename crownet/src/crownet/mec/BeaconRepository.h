#pragma once

#include "packets/SimpleMECBeacon_m.h"

#include <map>
#include <vector>

struct StoredBeacon
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
