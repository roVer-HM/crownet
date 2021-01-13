package rover.artery.traci;

import artery.networking.IPositionProvider;

simple PedestrianPositionProvider like IPositionProvider
{
    parameters:
        @class(rover::PedestrianPositionProvider);
        @signal[PositionFix](PositionFixObject);
        string mobilityModule;
        string runtimeModule;
}
