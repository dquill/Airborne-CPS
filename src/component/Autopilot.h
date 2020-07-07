#pragma once

#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include <cassert>
#include <cstring>

class Autopilot {
public:
    std::string usersMac;
    std::string intruderMac;
    XPLMDataRef lat, local_y, local_x, theta, position;
    float q[4];
    double data;
    Autopilot() {
        lat = XPLMFindDataRef("sim/flightmodel/position/latitude");
        local_y = XPLMFindDataRef("sim/flightmodel/position/local_y");
       // local_x = XPLMFindDataRef("sim/flightmodel/position/local_x");
       // theta = XPLMFindDataRef("sim/flightmodel/position/theta");
        position = XPLMFindDataRef("sim/flightmodel/position/q");
    }
    void getInformation() {
        data = XPLMGetDatad(lat);
        std::string info = "AutoPilot Latitude: " + std::to_string(data) + "\n";
        XPLMDebugString(info.c_str());
    }

    void writeToRef(double i) {
        double currentPosition = XPLMGetDatad(local_y);
        double updatedPosition = currentPosition + i;
        XPLMSetDatad(local_y, updatedPosition);
    }
    double getCurrentY() {
        return XPLMGetDatad(local_y);
    }
    double getCurrentX() {
        return XPLMGetDatad(local_x);
    }
    void getCurrentTheta() {
        std::string s = std::to_string(XPLMGetDatad(theta));
        s = s + "\n";
        XPLMDebugString(s.c_str());
        
    }

    //Rseturns current position of plane in the form of a quaternion {x,y,z}
    //These are local coordinates, not latitude/longitude/altitude
    std::string getCurrentPosition() {
        int count = XPLMGetDatavf(position,q,0,4);
        std::string values = "Quaternion Coordinates: "+ std::to_string(q[0]) + " " + std::to_string(q[1]) + " " + std::to_string(q[2]) + " " + std::to_string(q[3]) +"\n";
        return values;
    }

};