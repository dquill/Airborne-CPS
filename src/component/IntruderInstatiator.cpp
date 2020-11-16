#include "IntruderInstatiator.h"

char* gpIIAircraft[4];
char gIIAircraftPath[256];

//user selection for which threat classification to start drawing intruders at
Aircraft::ThreatClassification threatChoice;



const double kFullPlaneDist = 5280.0 / 3.2 * 3.0;
static inline float sqr(float a) { return a * a; }
static inline float CalcDist3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1));
}


int AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon);
IntruderInstantiator* IntruderInstantiator::instance = NULL;

// constructor
IntruderInstantiator::IntruderInstantiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>* imap) {

	// we will have diff menu item names, and these calls should go in AirbornCPS.cpp
	//gAcquireAircraftSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "AcquireAircraft", 0, IGNOREDPARAMETER);
	//gAcquireAircraftMenu = XPLMCreateMenu("AcquireAircraft", XPLMFindPluginsMenu(), gAcquireAircraftSubMenuItem, AcquireAircraftMenuHandlerCallback, 0);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Acquire Planes", "Acquire Planes", IGNOREDPARAMETER);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Release Planes", "Release Planes", IGNOREDPARAMETER);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Load Aircraft", "Load Aircraft", IGNOREDPARAMETER);

	intrudersMap = imap;

	threatChoice = Aircraft::ThreatClassification::PROXIMITY_INTRUDER_TRAFFIC;

	//DrawingEnabled = false;

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


// we need to pass a reference to the intrudersmap to the class so that we know where it is. 
IntruderInstantiator* IntruderInstantiator::getIntruderInstatiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap) {
	if (instance == NULL) { // why is it angry about this?
		instance = new IntruderInstantiator(intrudersMap);
	}
	return instance;
}
// for when we just want to call on our instance to do things, but don't have easy access to a pointer to the intruders map.
// this function returns a pointer to the IntruderInstantiator intance if it has already be constructed.
// Returns NULL if there is no instance yet.
IntruderInstantiator* IntruderInstantiator::getIntruderInstatiator() {
	if (instance == NULL) {
		return NULL;
	}
	return instance;
}



// inner method for draw callback that has access to class variables
int IntruderInstantiator::DrawCallback (XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon)
{

	updateDrawnIntruders();
	int planeCount;
	//double x, y, z, x1, y1, z1;

	float distMeters;
	float Heading, Pitch, Roll;

	int drawFullPlane;
	int Index;

	XPLMCountAircraft(&planeCount, 0, 0);

	if (planeCount <= 1)
		return 0;

	XPLMCameraPosition_t cameraPos;

	XPLMReadCameraPosition(&cameraPos);

	Pitch = XPLMGetDataf(gPlaneTheta);
	Roll = XPLMGetDataf(gPlanePhi);


	// iterate through at most planeCount of drawninterudersmap
	int counter = 0;
	int idx = 1;
	for (auto const& iter : this->drawnIntrudersMap) {

		Aircraft* intruder = iter.second;
		intruder->lock.lock(); // must lock this before writing to it

		//convert the aircrafts LLA into OpenGL Local X, Y and Z coordinates
		XPLMWorldToLocal(intruder->positionCurrent.latitude.toDegrees(),
						 intruder->positionCurrent.longitude.toDegrees(),
						 intruder->positionCurrent.altitude.toMeters(), 
						&intruder->openGL_localx,
						&intruder->openGL_localy,
						&intruder->openGL_localz);

		intruder->lock.unlock();  // unlock the Aircraft object

		distMeters = CalcDist3D(iter.second->openGL_localx, iter.second->openGL_localy, iter.second->openGL_localz, cameraPos.x, cameraPos.y, cameraPos.z);
		if (cameraPos.zoom != 0.0)
			distMeters /= cameraPos.zoom;

		drawFullPlane = distMeters < kFullPlaneDist;

		float backAzimuth = 0;
		//if (Heading >= 180) {
		//	backAzimuth = Heading - 180;
		//}
		//else {
		//	backAzimuth = Heading + 180;
		//}


		XPLMPlaneDrawState_t DrawState;
		DrawState.structSize = sizeof(XPLMPlaneDrawState_t);
		DrawState.gearPosition = 1;
		DrawState.flapRatio = 1.0;
		DrawState.spoilerRatio = 0;
		DrawState.speedBrakeRatio = 0;
		DrawState.slatRatio = 0;
		DrawState.wingSweep = 0;
		DrawState.thrust = 0;
		DrawState.yokePitch = 0;
		DrawState.yokeHeading = 0;
		DrawState.yokeRoll = 0;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(iter.second->openGL_localx, iter.second->openGL_localy, iter.second->openGL_localz);
		glRotatef(backAzimuth, 0.0, -1.0, 0.0);
		glRotatef(Pitch, 1.0, 0.0, 0.0);
		glRotatef(Roll, 0.0, 0.0, -1.0);


		// we need to keep track of an airplane index, not sure if this is the right way but lets try it
		XPLMDrawAircraft(idx, (float)intruder->openGL_localx, (float)intruder->openGL_localy, (float)intruder->openGL_localz, Pitch, Roll, intruder->heading.toDegrees(), drawFullPlane ? 1 : 0, &DrawState);
		idx++;
		if (idx > (planeCount - 1))
			idx = 1;
		glPopMatrix();
	}
	return 1;
}

int AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon)
{
	IntruderInstantiator* ii_inst = IntruderInstantiator::getIntruderInstatiator();
	if (ii_inst) {
		return ii_inst->DrawCallback(inPhase, inIsBefore, inRefcon);
	}
	else {
		return 1;
	} 
	
}

void updateDrawnIntrudersCallback() {
	IntruderInstantiator* ii_inst = IntruderInstantiator::getIntruderInstatiator();
	if (ii_inst) {
		return ii_inst->updateDrawnIntruders();
	}
	else {
		return;
	}
}

//method that gets the chosen threat classification
void IntruderInstantiator::getThreatChoice(int threatClass)
{
	threatChoice = (Aircraft::ThreatClassification)threatClass;
}

void IntruderInstantiator::updateDrawnIntruders()
{
	
	//iterate through intrudersMap
	for (auto iter : *this->intrudersMap)
	{
		 
		//if threat classification of aircraft >= threat classification we're drawing at
		if (iter.second->threatClassification >= threatChoice)
		{
			//look for the aircraft in drawnIntrudersMap
			auto foundAircraft = drawnIntrudersMap.find(iter.first);

			if (foundAircraft == drawnIntrudersMap.end())
			{
				//add it to drawnIntrudersMap if it's not in there
				addDrawnIntruder(iter.second);
			}
		}
	}



	//iterate through drawnIntrudersMap
	for (auto iter = drawnIntrudersMap.cbegin(), next_iter = iter; iter != drawnIntrudersMap.cend();
		iter = next_iter)
	{
		//store next interator in case we delete the current one
		next_iter++;

		//look for the aircraft in intrudersMap
		auto foundAircraft = this->intrudersMap->find(iter->first);

		if (foundAircraft == intrudersMap->end())
		{
			//remove it from drawnIntrudersMap if it's not in intrudersMap
			removeDrawnIntruder(iter->second);
		}
		else if (iter->second->threatClassification < threatChoice)
		{
			//also remove it if its threat classification is < threat classification we're drawing at now
			removeDrawnIntruder(iter->second);
		}

	}
}

void IntruderInstantiator::addDrawnIntruder(Aircraft* intruder)
{
	//add the intruder to drawnIntrudersMap
	(drawnIntrudersMap)[intruder->id] = intruder;
}

void IntruderInstantiator::removeDrawnIntruder(Aircraft* intruder)
{
	//remove the intruder from drawnIntrudersMap
	drawnIntrudersMap.unsafe_erase(intruder->id);
}


void AcquireAircraft(void)
{
	int PlaneCount;
	int Index;
	char FileName[256];
	XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, FileName, gIIAircraftPath);

	XPLMCountAircraft(&PlaneCount, 0, 0);
	if (PlaneCount > 1)
	{
		
		for (Index = 1; Index < PlaneCount; Index++)
		{	
			gpIIAircraft[Index - 1] = (char*)gIIAircraftPath;
		}
		if (XPLMAcquirePlanes((char**)&gpIIAircraft, AcquireAircraftPlanesAvailableCallback, NULL))
		{
			OutputDebugString("Aircraft Acquired successfully\n");
			XPLMDebugString("Aircraft Acquired successfully\n");

			for (Index = 1; Index < PlaneCount; Index++)
			{
				XPLMSetAircraftModel(Index, gIIAircraftPath);
			}
		}
		else
		{
			OutputDebugString("Aircraft not Acquired\n");
		}
	}
}

void AcquireAircraftPlanesAvailableCallback(void* inRefcon)
{
	OutputDebugString("AcquireAircraftPlanesAvailableCallback\n");
	XPLMDebugString("AcquireAircraftPlanesAvailableCallback\n");
	AcquireAircraft();
}









