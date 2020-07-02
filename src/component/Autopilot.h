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
    XPLMDataRef lat, local_x;
    double data;
    Autopilot() {
        lat = XPLMFindDataRef("sim/flightmodel/position/latitude");
        local_x = XPLMFindDataRef("sim/flightmodel/position/local_x");
    }
    void getInformation() {
        data = XPLMGetDatad(lat);
        std::string info = "AutoPilot Latitude: " + std::to_string(data) + "\n";
        XPLMDebugString(info.c_str());
    }

    void writeToRef(double i) {
        XPLMSetDatad(local_x, i);
    }

    //local_x is a double
    //need a method that writes to a double

};