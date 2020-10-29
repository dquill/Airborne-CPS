#include "IntruderInstatiator.h"




int AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon);
IntruderInstatiator* IntruderInstatiator::instance = NULL;

// constructor
IntruderInstatiator::IntruderInstatiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap) {

	// we will have diff menu item names, and these calls should go in AirbornCPS.cpp
	//gAcquireAircraftSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "AcquireAircraft", 0, IGNOREDPARAMETER);
	//gAcquireAircraftMenu = XPLMCreateMenu("AcquireAircraft", XPLMFindPluginsMenu(), gAcquireAircraftSubMenuItem, AcquireAircraftMenuHandlerCallback, 0);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Acquire Planes", "Acquire Planes", IGNOREDPARAMETER);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Release Planes", "Release Planes", IGNOREDPARAMETER);
	//XPLMAppendMenuItem(gAcquireAircraftMenu, "Load Aircraft", "Load Aircraft", IGNOREDPARAMETER);

	concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap = intrudersMap;

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
IntruderInstatiator* IntruderInstatiator::getIntruderInstatiator(concurrency::concurrent_unordered_map<std::string, Aircraft*>* intrudersMap) {
	if (instance == NULL) { // why is it angry about this?
		instance = new IntruderInstatiator(intrudersMap);
	}
	return instance;
}
// for when we just want to call on our instance to do things, but don't have easy access to a pointer to the intruders map.
// this function returns a pointer to the IntruderInstantiator intance if it has already be constructed.
// Returns NULL if there is no instance yet.
IntruderInstatiator* IntruderInstatiator::getIntruderInstatiator() {
	if (instance == NULL) {
		return NULL;
	}
	return instance;
}

const double kFullPlaneDist = 5280.0 / 3.2 * 3.0;
static inline float sqr(float a) { return a * a; }
static inline float CalcDist3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1));
}

// inner method for draw callback that has access to class variables
int IntruderInstatiator::DrawCallback (XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon)
{
	int planeCount;
	double x, y, z, x1, y1, z1;

	float distMeters, Latitude, Longitude, Elevation;
	float Heading, Pitch, Roll;

	int drawFullPlane;
	int Index;

	XPLMCountAircraft(&planeCount, 0, 0);

	if (planeCount <= 1)
		return 0;

	XPLMCameraPosition_t cameraPos;

	XPLMReadCameraPosition(&cameraPos);

	Latitude = XPLMGetDataf(gLatitude);
	Longitude = XPLMGetDataf(gLongitude);
	Elevation = XPLMGetDataf(gElevation);
	Pitch = XPLMGetDataf(gPlaneTheta);
	Roll = XPLMGetDataf(gPlanePhi);
	Heading = XPLMGetDataf(gPlanePsi);

	//XPLMWorldToLocal(Latitude, Longitude, Elevation, &x, &y, &z);

	for (Index = 1; Index < NUMINTRUDERS; Index++)
	{
		//x1 = x - (Index * 50.0);
		//y1 = y;
		//z1 = z + (Index * 50.0);

		//distMeters = CalcDist3D(intruders[Index].x, intruders[Index].y, intruders[Index].z, cameraPos.x, cameraPos.y, cameraPos.z);
		if (cameraPos.zoom != 0.0)
			distMeters /= cameraPos.zoom;

		drawFullPlane = distMeters < kFullPlaneDist;

		float backAzimuth = 0;
		if (Heading >= 180) {
			backAzimuth = Heading - 180;
		}
		else {
			backAzimuth = Heading + 180;
		}


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
		//glTranslatef(intruders[Index].x, intruders[Index].y, intruders[Index].z);
		glRotatef(backAzimuth, 0.0, -1.0, 0.0);
		glRotatef(Pitch, 1.0, 0.0, 0.0);
		glRotatef(Roll, 0.0, 0.0, -1.0);
		//XPLMDrawAircraft(Index, (float)intruders[Index].x, (float)intruders[Index].y, (float)intruders[Index].z, Pitch, Roll, backAzimuth, drawFullPlane ? 1 : 0, &DrawState);
		//ai++;
		//if (ai > (planeCount - 1))
		//	ai = 1;
		glPopMatrix();
	}
	return 1;
}

int AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon)
{
	IntruderInstatiator* ii_inst = IntruderInstatiator::getIntruderInstatiator();
	if (ii_inst) {
		return ii_inst->DrawCallback(inPhase, inIsBefore, inRefcon);
	}
	else {
		return 1;
	}
	
}

void IntruderInstatiator::updateDrawnIntruders()
{
	// I made this based off of what our class and sequence diagrams currently say, tell me if it isn't what you had in mind
	// imIter is the intrudersMap iterator, dimIter is the drawnIntrudersMap iterator

	// What we really need to look at for this part is the Activity Diagram. I think when its all said and done, we 
	// are going to be going back and changing those other diagrams, such as the class diagram, to reflect what 
	// the code actually ends up being.

	/* So the activity diagram says that we have some sort of recurring event, or outer loop. We are thinking maybe 
	   registering a flight loop callback, or we could perhaps use the drawing callback, but i'm concerned that callback
	   happens so often (once per frame) and doing all this in it might be too much. so maybe we will register a separate
	   flight loop callback that happens at a slower interval. 

	   
	   Regardless of the implementation detail of that, there's an outer loop/callback. And inside that there are two
	   for loops. One of them iterates through the intruders map, and adds records to the DrawnIntrudersMap as it determines
	   is needed. The second for loop iterates through the DrawnIntrudersMap and deletes records from DrawnIntruders if they are 
	   no longer in the intrudersMap. I think that actually needs to be **deletes records from the DrawnIntrudersMap if they are
	   no longer in the intrudersMap or their threatclassificatoin is no longer >= TRAFFIC_ADVISORY**  but the activity diagram 
	   currently doesn't test for threatclassification. 

		   ********* I just thought of something. Our maps contain POINTERS to Aircraft objects, and those Aircraft already get their
	   locations updated. Both maps contain pointers. This means that the first for loop in the activity diagram, the box that
	   says Copy IntrudersMap[intruder] to drawnIntrudersMap[intruder] doesn't need to happen. That was in effect the updating 
	   of a location. The location in the Aircraft object will already be updated, which means the data that is read from reading
	   through the map is already updated. **************
	   */

	// iterate through intrudersMap

	// but if you were looking to make an iterator, this is how:
	concurrency::concurrent_unordered_map<std::string, Aircraft*> ::iterator iter;

	// But for C11 and newer, we should be able to use auto iter : myMap like this:
	for (auto iter : *this->intrudersMap) {

	}

	// as per SO post https://stackoverflow.com/questions/4844886/how-can-i-loop-through-a-c-map-of-maps

	for (concurrency::concurrent_unordered_map<std::string, Aircraft*>*::iterator& imIter = intrudersMap.begin(); imIter != intrudersMap.end();)
	{


		// don't iterate. Look up in 
		drawnIntrudersMap.find(imIter->first);

		//add an intruder if it's not in drawnIntrudersMap
		if (!dimIter)
		{
			addDrawnIntruder(imIter->second);
		}
		else
		{
			//here we would replace the information drawnIntrudersMap currently has about the aircraft with the information itrudersMap has about it
			//should we create a method called updateIntruderLocation() or something that will do that and we call that method here? or should we just put all the code that does that here?
		}
	}

	//this loop iterates through drawnIntrudersMap first, then through intrudersMap to see if we have any aircraft that are in drawnIntrudersMap but not in intrudersMap
	//let me know if there's a better way to do this
	for (concurrency::concurrent_unordered_map<std::string, Aircraft*>*::iterator dimIter = drawnIntrudersMap.begin(); dimIter != drawnIntrudersMap.end();)
	{
		concurrency::concurrent_unordered_map<std::string, Aircraft*>*::iterator imIter = intrudersMap.find(dimIter->first);

		//remove an intruder if it's not in intrudersMap
		if (!imIter)
		{
			removeDrawnIntruder(dimIter->second);
		}
	}
}








