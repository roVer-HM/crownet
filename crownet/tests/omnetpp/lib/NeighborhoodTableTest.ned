package crownet.test.omnetpp.lib;

import crownet.neighbourhood.NeighborhoodTable;

simple NeighborhoodTableTestApp{
    
    parameters:
        @class(::crownet::NeighborhoodTableTestApp);
        double maxAge @unit(s) = default (3s);
}

network NeighborhoodTableTestNetwork {
  submodules:
    foo: NeighborhoodTableTestApp{}
}