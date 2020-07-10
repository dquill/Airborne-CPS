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
    XPLMDataRef lat, local_y, local_x, theta, position, r1, r2, elv_trim;
    float q[4];
    float elv1[56], elv2[56];
    float elvPitch;
    double data;
    Autopilot() {
        lat = XPLMFindDataRef("sim/flightmodel/position/latitude");
        local_y = XPLMFindDataRef("sim/flightmodel/position/local_y");
       // local_x = XPLMFindDataRef("sim/flightmodel/position/local_x");
       // theta = XPLMFindDataRef("sim/flightmodel/position/theta");
        position = XPLMFindDataRef("sim/flightmodel/position/q");
        r1 = XPLMFindDataRef("sim/flightmodel/controls/elv1_def");
        r2 = XPLMFindDataRef("sim/flightmodel/controls/elv2_def");
        elv_trim = XPLMFindDataRef("sim/flightmodel/controls/elv_trim");
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

    void getRudderPosition() {
        int count = XPLMGetDatavf(r1,elv1,0,56);
        int count2 = XPLMGetDatavf(r2, elv2, 0, 56);

        for (int i = 0; i < (sizeof(elv1) / sizeof(elv1[0])); i++) {
            std::string value = "Float position at [" + std::to_string(i) + "]: " + std::to_string(elv1[i]) + "\n";
            XPLMDebugString(value.c_str());
        }
      //  std::string values = "Rudder 1 data: " + std::to_string(elv1[0]) + " " + std::to_string(elv1[1]) + " " + std::to_string(elv1[2]) + " " + std::to_string(elv1[3]) + " " + std::to_string(elv1[4]) + "\n";
    }

    void changeRudderPosition() {
        int count = XPLMGetDatavf(r1, elv1, 0, 56);
        int count2 = XPLMGetDatavf(r2, elv2, 0, 56);
        for (int i = 0; i < (sizeof(elv1) / sizeof(elv1[0])); i++) {
            elv1[i] = elv1[i] + 0.05f;
        }
        for (int i = 0; i < (sizeof(elv2) / sizeof(elv2[0])); i++) {
            elv2[i] = elv2[i] + 0.05f;
        }
        XPLMSetDatavf(r1, elv1, 0, 56);
        XPLMSetDatavf(r2, elv2, 0, 56);
        
    }

    void getElevatorPitch() {
        std::string value = "Elevator Pitch:  "+ std::to_string(XPLMGetDataf(elv_trim)) + "\n";
        XPLMDebugString(value.c_str());
    }

    void setElevatorPitch() {
        XPLMSetDataf(elv_trim, (1.0f));
    }

};