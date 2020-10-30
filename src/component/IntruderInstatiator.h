#pragma once
#ifndef WINVER
// Define windows xp version
#define WINVER 0x0501
#endif

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMCamera.h"
#include "XPLMPlanes.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"

#pragma comment(lib, "IPHLPAPI.lib")
#include <iphlpapi.h>

#include <vector>                  // we probably don't need to include all of thse
#include <sys/types.h>             // but i was trying to get concurrency::concurrent_unordered_map to resolve
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <mutex>
//#include <ctime>
#include <condition_variable>
#include <math.h>
#include <concrt.h>                   // why did i have to explicityly include these
#include <concurrent_unordered_map.h> // when transponder.h doesn't have to?



#include "data/Aircraft.h"
#include "data\Location.h"
#include <gl\gl.h>
#include <gl\glu.h>

//using namespace Concurrency;

#define IGNOREDPARAMETER 1
#define NUMINTRUDERS 4

class IntruderInstantiator
{
private:
	static IntruderInstantiator* instance;

	XPLMDataRef gLatitude;
	XPLMDataRef gLongitude;
	XPLMDataRef gElevation;
	XPLMDataRef gPlaneX;
	XPLMDataRef gPlaneY;
	XPLMDataRef gPlaneZ;
	XPLMDataRef gPlaneTheta;
	XPLMDataRef gPlanePhi;
	XPLMDataRef gPlanePsi;

	XPLMMenuID gAcquireAircraftMenu;
	int gAcquireAircraftSubMenuItem;

	char* gpAircraft[4];
	char gAircraftPath[4][256];



	concurrency::concurrent_unordered_map<std::string, Aircraft*>* drawnIntrudersMap;
	concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap;
	IntruderInstantiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>*);

	static inline float sqr(float a);
	static inline float CalcDist3D(float x1, float y1, float z1, float x2, float y2, float z2);


public:
	static IntruderInstantiator* IntruderInstantiator::getIntruderInstatiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap);
	static IntruderInstantiator* IntruderInstantiator::getIntruderInstatiator();
	int IntruderInstantiator::DrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon);
	void IntruderInstantiator::AcquireAircraftMenuHandlerCallback(void* inMenuRef, void* inItemRef);
	float IntruderInstantiator::AcquireAircraftFlightLoopCB(float elapsedMe, float elapsedSim, int counter, void* refcon);
	void IntruderInstantiator::AcquireAircraftPlanesAvailableCallback(void* inRefcon);
	void IntruderInstantiator::AcquireAircraft(void);
	void IntruderInstantiator::updateDrawnIntruders();
	void IntruderInstantiator::addDrawnIntruder(Aircraft*);
	void IntruderInstantiator::removeDrawnIntruder(Aircraft*);
	void IntruderInstantiator::drawIntruder(Aircraft*);
};

