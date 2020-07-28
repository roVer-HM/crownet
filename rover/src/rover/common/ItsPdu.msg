//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.applications.base.ApplicationPacket;
import inet.common.packet.chunk.Chunk;
import inet.common.geometry.Geometry;
import rover.common.util.rover_util;

cplusplus{{
#include "rover/common/util/rover_util.h"
}}

namespace rover;

// StationType ETSI TS 102 894-2 V1.2.1 (2014-09)
enum ItsStationType {
	UNKNOWN = 0;
	PEDSTRIAN = 1;
	CYCLIST = 2;
	MOPED = 3;
	MOTORCYCLE = 4;
	PASSENGER_CAR = 5;
	BUS = 6;
	LIGHT_TRUCK = 7;
	HEAVY_TRUCK = 8;
	TRAILER = 9;
	SPECIAL_VEHICLES = 10;
	TRAM = 11;
	ROAD_SIDE_UNIT = 15;	
}

// DataType_94 VehicleRole ETSI TS 102 894-2 V1.2.1 (2014-09)
enum ItsVehicleRole {
	DEFAULT = 0; //default vehicle role as indicated by the vehicle type
	PUBLIC_TRANSPORT = 1;
	SPECIAL_TRANSPORT = 2;
	DANGEROUS_GOODS = 3;
	ROAD_WORK = 4;
	RESCUE = 5;
	EMERGENCY = 6;
	SAFTEY_CAR = 7;
	AGRICULTURE = 8;
	COMMERCIAL = 9;
	MILITARY = 10;
	ROEAD_OPERATOR = 11;
	TAXI = 12;
	RESERVERD1 = 13;
	RESERVERD2 = 14;
	RESERVERD3 = 15;
}

// VRU_Profile ETSI TS 103 300-2 V2.1.1 (2020-05)
enum ItsVruProfile {
	PEDESTRIAN = 1;
	BICYCLIST = 2;
	MOTORCYCLIST = 3;
	ANIMALS = 4;
}

enum ItsVruType {
	ADULT = 1;
	CHILD = 2;
	INFANT = 3;
	ANIMAL = 4;
}

enum ItsVruDeviceType {
 	VRU_TX = 1; // ETSI TS 103 300-2 V2.1.1 (2020-05)
	VRU_RX = 2; // ETSI TS 103 300-2 V2.1.1 (2020-05)
	VRU_ST = 3; // ETSI TS 103 300-2 V2.1.1 (2020-05)   
}

enum ItsMessageId {
	DENM = 1; //Decentralized Environmental Notification Message
	CAM = 2;  //Cooperative Awareness Message
	POI = 3;  //Point of Interest message as specified
	SPAT = 4; //Signal Phase And Timing
	MAP = 5; // Map Data message
	IVI = 6;  //  In Vehicle Information
	EV_RSR = 7; // Electric vehicle recharging spot reservation message
	CPM = 8; // Cooperative Perception Message
	MCM = 9; // Manoeuvre Coordination Message
	VAM = 10; // VRU Awareness Message	   
}

class ItsBase extends cObject {

}

// DataType_124 (using cartesian coordinates here but standard uses lat/lon/alt)
class ReferencePosition extends ItsBase {
    inet::Coord referencePosition;
    double semiMajorConf; // part of DataType_119 PosConfidenceEllipse
    double semiMinorConf; // part of DataType_119 PosConfidenceEllipse
    inet::Coord headingValue; // part of DataType_119 PosConfidenceEllipse
}


class ItsContainer extends ItsBase
{
}

// The basic container includes basic information related to the originating ITS-S
class ItsBasicContainer extends ItsContainer
{
    ItsStationType stationType;
	ReferencePosition referencePosition;
	ItsVehicleRole role;
	PathPoint pathHistory[40];
}

class ItsBasicVehicleLfContainer extends ItsBasicContainer {
    short exterioLights; // 0-7 bitmask
    PathPoint pathHistory[40];
}


class ItsVamBasicContainer extends ItsBasicContainer{
    ItsVruProfile vruProfile;
    ItsVruType vruType;
    ItsVruDeviceType vruDeviceType;
}

// The high frequency container contains highly dynamic information of the originating ITS-S
class ItsHfContainer extends ItsContainer
{

}

class ItsBasicVehicleHfContainer extends ItsHfContainer {
  inet::Coord heading; //normalized
  double speed; // in m/s
  //driveDirection DriveDirection,
  double vehicleLength; // in m
  double vehicleWidth; // in m
  //longitudinalAcceleration LongitudinalAcceleration,
  //curvature Curvature,
  //curvatureCalculationMode CurvatureCalculationMode,
  //yawRate YawRate,
  //accelerationControl AccelerationControl OPTIONAL,
  //lanePosition LanePosition OPTIONAL,
  //steeringWheelAngle SteeringWheelAngle OPTIONAL,
  //lateralAcceleration LateralAcceleration OPTIONAL,
  //verticalAcceleration VerticalAcceleration OPTIONAL,
  //performanceClass PerformanceClass OPTIONAL,
  //cenDsrcTollingZone CenDsrcTollingZone OPTIONAL
}

class ItsVamHfContainer extends ItsHfContainer {
   double speed;
   inet::Coord direction;   // heading, normalized
   inet::Coord orientation;   
   PathPoint predictedTrajectory[10]; 
   PathPoint predictedVelocity[10];  
}

//The low frequency container contains static and not highly dynamic information of the originating ITS-S.
class ItsLfContainer extends ItsContainer
{
    
}

class ItsSpecialVehicleContainer extends ItsContainer
{
    
}

class ItsVamClusterContainer extends ItsSpecialVehicleContainer {
    int clusterIdentifier;
    inet::Coord clusterPosition;
    inet::Coord clusterDimension;
    int clusterSize;
}

class ItsPublicTransportContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsSpecialTransportContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsDangerousGoodsContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsRoadWorksContainerBasic extends ItsSpecialVehicleContainer
{
    
}

class ItsRescueContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsEmergencyContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsSafetyCarContainer extends ItsSpecialVehicleContainer
{
    
}

class ItsPduHeader extends ItsBase{
	int protocolVersion;
	ItsMessageId messageId;
	int stationId; // using omnet node id?
}

class ItsCam extends inet::ApplicationPacket
{
   ItsPduHeader itsHeader; 
   simtime_t generationDeltaTime; // Time corresponding to the time of the reference position
   //CoopAwareness Parameters 
   ItsBasicContainer basicContainer; // type, position
   ItsHfContainer hfContainer; 
   ItsLfContainer lfContainer; // optional
   ItsSpecialVehicleContainer specialContainer; // optional
}

class ItsVam extends inet::ApplicationPacket
{
    ItsPduHeader itsHeader; // ITS-S Id/ Packet Type (CAM, VAM, ...)
    simtime_t generationDeltaTime; // Time corresponding to the time of the reference position
	ItsVamBasicContainer basicContainer; // Profile, Type, Device, referencePosition, pathHistory
	ItsVamHfContainer hfContainer; // speed, Prediction for Speed, Trajectory
	ItsLfContainer lfContainer; // optional
	ItsVamClusterContainer clusterContainer; // Cluster Paramters
}

