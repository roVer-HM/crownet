package crownet.artery.traci;

import artery.networking.IPositionProvider;

simple PedestrianPositionProvider like IPositionProvider
{
    parameters:
        @class(crownet::PedestrianPositionProvider);
        @signal[PositionFix](PositionFixObject);
        string mobilityModule;
        string runtimeModule;
}
