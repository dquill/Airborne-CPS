#pragma once

#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include <cassert>
#include <cstring>
#include <math.h>

class Autopilot {
public:
    std::string usersMac;
    std::string intruderMac;
    XPLMDataRef position, override_planepath, iairspeed;
    XPLMDataRef theta, psi, phi;
    float q[4];
    float elv1[56], elv2[56];
    float elvPitch;
    float pitch;
    double data;
    Autopilot() {
        position = XPLMFindDataRef("sim/flightmodel/position/q");
        theta = XPLMFindDataRef("sim/flightmodel/position/theta");
        phi = XPLMFindDataRef("sim/flightmodel/position/phi");
        psi = XPLMFindDataRef("sim/flightmodel/position/psi");
        override_planepath = XPLMFindDataRef("sim/operation/override/override_planepath");
        iairspeed = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");
    }

    void getPosition() {
        std::string s = "Theta: ["+ std::to_string(XPLMGetDataf(theta)) + "] - Phi: [" + std::to_string(XPLMGetDataf(phi)) + "] Psi: [" + std::to_string(XPLMGetDataf(psi)) + "] \n";
        XPLMDebugString(s.c_str());
    }

    void getCurrentTheta() {
        std::string s = std::to_string(XPLMGetDatad(theta));
        s = s + "\n";
        XPLMDebugString(s.c_str());  
    }

    //Returns current position of plane in the form of a quaternion {x,y,z}
    //These are local coordinates, not latitude/longitude/altitude
    void getCurrentPosition() {
        int count = XPLMGetDatavf(position,q,0,4);
        std::string values = "Quaternion Coordinates: "+ std::to_string(q[0]) + " " + std::to_string(q[1]) + " " + std::to_string(q[2]) + " " + std::to_string(q[3]) +"\n";
        XPLMDebugString(values.c_str());
        
    }

    void setCurrentPosition() {
        int airspeedInterval = 40;
        float thetaInterval = .1;
        float phiInterval;
        int count = XPLMGetDatavf(position, q, 0, 4);
        if (XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/phi")) > 0) {
            phiInterval = -.1;
        }
        if (XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/phi")) < 0) {
            phiInterval = .1;
        }

        float roll = M_PI / 360 * (XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/phi")) + phiInterval);  
        float pitch = M_PI / 360 * (XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/theta")) + thetaInterval);
        float yaw = M_PI / 360 * XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/psi"));
        float o[4];
        o[0] = cos(yaw) * cos(pitch) * cos(roll) + sin(yaw) * sin(pitch) * sin(roll);
        o[1] = cos(yaw) * cos(pitch) * sin(roll) - sin(yaw) * sin(pitch) * cos(roll);
        o[2] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * cos(pitch) * sin(roll);
        o[3] = -cos(yaw) * sin(pitch) * sin(roll) + sin(yaw) * cos(pitch) * cos(roll);


        //flight path must be disabled for this to work
        //set flight model
        XPLMSetDataf(phi, XPLMGetDataf(phi) + phiInterval);
        XPLMSetDataf(theta, XPLMGetDataf(theta) + thetaInterval);
        XPLMSetDataf(iairspeed, XPLMGetDataf(iairspeed) + airspeedInterval);
        //set quaternion position
        XPLMSetDatavf(position, o, 0, 4);
    }
    
    void disableFlightPath() {
        int path[20];
        int count = XPLMGetDatavi(override_planepath, path, 0, 20);
        path[0] = 1;
        XPLMSetDatavi(override_planepath, path, 0, 20);
    }

    void enableFlightPath() {
        int path[20];
        int count = XPLMGetDatavi(override_planepath, path, 0, 20);
        path[0] = 0;
        XPLMSetDatavi(override_planepath, path, 0, 20);
    }

    void flightPathSwitch() {
        int path[20];
        int count = XPLMGetDatavi(override_planepath, path, 0, 20);
        if (path[0] == 1) {
            path[0] = 0;
        }
        else {
            path[0] = 1;
        }
        XPLMSetDatavi(override_planepath, path, 0, 20);
    }


   
};