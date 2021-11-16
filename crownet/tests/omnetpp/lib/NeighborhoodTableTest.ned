package crownet.test.omnetpp.lib;

import crownet.neighbourhood.NeighborhoodTable;
import inet.mobility.static.StationaryMobility;

simple NeighborhoodTableTestApp extends NeighborhoodTable{
    
    parameters:
        @class(::crownet::NeighborhoodTableTestApp);
        maxAge @unit(s) = default (3s);
}

module Node {
    parameters:
        mobility.initFromDisplayString = false;
        mobility.initialX = 5m;
		mobility.initialY = 2m;
		mobility.initialZ = 1.3m;
        
 	submodules:
 	    table: NeighborhoodTableTestApp{}
 	    mobility: StationaryMobility{} 
}

network NeighborhoodTableTestNetwork {
  submodules:
    foo: Node{}
}