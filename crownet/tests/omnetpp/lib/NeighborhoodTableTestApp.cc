#include "crownet/common/NeighborhoodTable.h"
#include "crownet/common/NeighborhoodTableEntry.h"
#include <stdio.h>

namespace crownet{
    class NeighborhoodTableTestApp : public NeighborhoodTable {
        void printTable() {
            for(auto it = this->_table.cbegin(); it != _table.cend(); ++it)
            {
                std::cout << "nTable[" << it->first << "]" <<
                        " -- nodeId: " << it->second.getNodeId() <<
                        " -- timeSend: " << it->second.getTimeSend() << "\n";
            }
        }

        void initialize(int stage) override {
            NeighborhoodTable::initialize(stage);
            if (stage == INITSTAGE_LOCAL){
              std::cout << "maxAge: " << this->maxAge << std::endl;
            }
        }

        void finish() override {
        std::cout << "finished: simtime: " << simTime() << std::endl;
        this->printTable();
        }

    };
Define_Module(NeighborhoodTableTestApp);
}
