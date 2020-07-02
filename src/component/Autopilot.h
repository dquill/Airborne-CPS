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
    XPLMDataRef lat, local_y;
    double data;
    Autopilot() {
        lat = XPLMFindDataRef("sim/flightmodel/position/latitude");
        local_y = XPLMFindDataRef("sim/flightmodel/position/local_y");
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

};