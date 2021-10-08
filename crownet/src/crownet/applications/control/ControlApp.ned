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

// ControlApp is used for information dissemination based
// on control input provided by crowd-guidance (TraCI) using the
// configIn gate to trigger the sending of messages.
//
// These apps are based on ~BaseApp.ned and ~BaseBroadcast.ned with minor adation
// in de state machine. The lifecycle is still manged with the `startTime` and
// `stopTime`, however the main loop (actively sending messages) is not
// automatically started.  To start sending data a inet:Message Tagged with
// SimpleControlCfg Tag must be send to the `configIn` gate of the application
// containing the control model name and model metadata. With this Message the
// configured `appMain*` settings will take affect and new Application packets
// will be generated.
//
// Examples:
//
// 1. Single control information source, sending meassage 10 times with and interval of
//    5 seconds  without rebroadcast.
//
//  startTime = 0.0s        // lifecycle start (app is active but does not send packets)
//  stopTime = 0.0s         // lifecycle stop (run forever)
//  appMainInterval = 5.0s
//  appMainLimit = 10
//  hopcount = 1            // deactivate reboradcast
//
//
// 2. Single control informaion source, sending 5 messages with a 20 second interval
//    30 seconds after the control command was received. Additionally each message
//    should be reboradcasted 3 times.
//
//
//  startTime = 0.0s        // lifecycle start (app is active but does not send packets)
//  stopTime = 0.0s         // lifecycle stop (run forever)
//  appMainInveralOffset = 30.0s
//  appMainInterval = 20.0s
//  appMainLimit = 3
//  hopcount = 3
//
// These setting can be set for all nodes because appMain* settings will only take
// effect if a node is triggerd through the `configIn` gate. Passively receiving nodes
// must only ensure that there lifecycle settings (startTime, stopTime) are set correctly.
// Ensure to set isControlled only on nodes  actually controlled bei Vadere (TraCi)

package crownet.applications.control;

import crownet.applications.common.BaseBroadcast;

simple BroadcastControlApp extends BaseBroadcast 
{
parameters:
    @class(crownet::BroadcastControlApp);
    @display("i=block/app");
	// equiped node is controlled by Vadere. Send received message to Vadere over TraCI
	bool isControlled = default(true);
    
}