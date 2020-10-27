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



	LLA intruders[NUMINTRUDERS];

	IntruderInstatiator() {

	}



public:
	static IntruderInstatiator *getIntruderInstatiator() {
		if (!instance) {
			instance = new IntruderInstatiator();
		}
		return instance;
	}

};



