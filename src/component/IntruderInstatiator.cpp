#include "IntruderInstatiator.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMCamera.h"
#include "XPLMPlanes.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"

#include "Aircraft.h"


#include <gl\gl.h>
#include <gl\glu.h>

#define IGNOREDPARAMETER 1
#define NUMINTRUDERS 4

class IntruderInstatiator
{
private:
	static IntruderInstatiator* instance;

	XPLMDataRef gLatitude = NULL;
	XPLMDataRef gLongitude = NULL;
	XPLMDataRef gElevation = NULL;
	XPLMDataRef gPlaneX = NULL;
	XPLMDataRef gPlaneY = NULL;
	XPLMDataRef gPlaneZ = NULL;
	XPLMDataRef gPlaneTheta = NULL;
	XPLMDataRef gPlanePhi = NULL;
	XPLMDataRef gPlanePsi = NULL;

	XPLMMenuID gAcquireAircraftMenu;
	int gAcquireAircraftSubMenuItem;

	char* gpAircraft[4];
	char gAircraftPath[4][256];

	const double kFullPlaneDist = 5280.0 / 3.2 * 3.0;
	static inline float sqr(float a) { return a * a; }
	static inline float CalcDist3D(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1));
	}
	void AcquireAircraftMenuHandlerCallback(void* inMenuRef, void* inItemRef);
	float AcquireAircraftFlightLoopCB(float elapsedMe, float elapsedSim, int counter, void* refcon);
	void AcquireAircraftPlanesAvailableCallback(void* inRefcon);
	int AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase,
		int inIsBefore,
		void* inRefcon);
	void AcquireAircraft(void);

	typedef struct LLA {
		double latitude, longitude, altitude, x, y, z;
	}LLA;


	concurrency::concurrent_unordered_map<std::string, Aircraft*>* DrawnIntruders,

	// constructor
	IntruderInstatiator() {
		// these are from the example's XPluginStart()
		intruders[1] = { 47.523, 10.672, 3049.774, 0, 0, 0 };
		intruders[2] = { 47.525 , 10.672, 3049.345, 0, 0, 0 };
		intruders[3] = { 47.528 , 10.671, 3049.275, 0, 0, 0 };
		//intruders[4] = { 47.524, 10.672, 3150.442, 0, 0, 0 };


		strcpy(outName, "AcquireAircraft");
		strcpy(outSig, "xplanesdk.SandyBarbour.AcquireAircraft");
		strcpy(outDesc, "A plugin that controls aircraft.");

		gAcquireAircraftSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "AcquireAircraft", 0, IGNOREDPARAMETER);
		gAcquireAircraftMenu = XPLMCreateMenu("AcquireAircraft", XPLMFindPluginsMenu(), gAcquireAircraftSubMenuItem, AcquireAircraftMenuHandlerCallback, 0);
		XPLMAppendMenuItem(gAcquireAircraftMenu, "Acquire Planes", "Acquire Planes", IGNOREDPARAMETER);
		XPLMAppendMenuItem(gAcquireAircraftMenu, "Release Planes", "Release Planes", IGNOREDPARAMETER);
		XPLMAppendMenuItem(gAcquireAircraftMenu, "Load Aircraft", "Load Aircraft", IGNOREDPARAMETER);

		gLatitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
		gLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
		gElevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
		gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
		gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
		gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
		gPlaneTheta = XPLMFindDataRef("sim/flightmodel/position/theta");
		gPlanePhi = XPLMFindDataRef("sim/flightmodel/position/phi");
		gPlanePsi = XPLMFindDataRef("sim/flightmodel/position/psi");

		XPLMRegisterDrawCallback(AcquireAircraftDrawCallback, xplm_Phase_Airplanes, 0, NULL);
	}



public:
	static IntruderInstatiator *getIntruderInstatiator() {
		if (!instance) {
			instance = new IntruderInstatiator();
		}
		return instance;
	}

};



