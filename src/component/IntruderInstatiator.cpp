#include "IntruderInstatiator.h"




class IntruderInstatiator
{
	// constructor
	IntruderInstatiator::IntruderInstatiator() {

		// we will have diff menu item names, and these calls should go in AirbornCPS.cpp
		//gAcquireAircraftSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "AcquireAircraft", 0, IGNOREDPARAMETER);
		//gAcquireAircraftMenu = XPLMCreateMenu("AcquireAircraft", XPLMFindPluginsMenu(), gAcquireAircraftSubMenuItem, AcquireAircraftMenuHandlerCallback, 0);
		//XPLMAppendMenuItem(gAcquireAircraftMenu, "Acquire Planes", "Acquire Planes", IGNOREDPARAMETER);
		//XPLMAppendMenuItem(gAcquireAircraftMenu, "Release Planes", "Release Planes", IGNOREDPARAMETER);
		//XPLMAppendMenuItem(gAcquireAircraftMenu, "Load Aircraft", "Load Aircraft", IGNOREDPARAMETER);


		IntruderInstatiator::gLatitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
		IntruderInstatiator::gLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
		IntruderInstatiator::gElevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
		IntruderInstatiator::gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
		IntruderInstatiator::gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
		IntruderInstatiator::gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
		IntruderInstatiator::gPlaneTheta = XPLMFindDataRef("sim/flightmodel/position/theta");
		IntruderInstatiator::gPlanePhi = XPLMFindDataRef("sim/flightmodel/position/phi");
		IntruderInstatiator::gPlanePsi = XPLMFindDataRef("sim/flightmodel/position/psi");

		// we might have to chain the callback
		// put the direct callback function in AirborneCPS.cpp, and have
		// it call our IntruderInstatiator objects callback
		// also, this call will be in AirbornCPS.cpp as well
		XPLMRegisterDrawCallback(AcquireAircraftDrawCallback, xplm_Phase_Airplanes, 0, NULL);
	}


	static IntruderInstatiator* IntruderInstatiator::getIntruderInstatiator() {
		if (*instance == NULL) {
			instance = new IntruderInstatiator();
		}
		return instance;
	}

	const double kFullPlaneDist = 5280.0 / 3.2 * 3.0;
	static inline float sqr(float a) { return a * a; }
	static inline float CalcDist3D(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		return sqrt(sqr(x2 - x1) + sqr(y2 - y1) + sqr(z2 - z1));
	}

	int IntruderInstatiator::AcquireAircraftDrawCallback(XPLMDrawingPhase inPhase,int inIsBefore,void* inRefcon)
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

		//Latitude = XPLMGetDataf(gLatitude);
		//Longitude = XPLMGetDataf(gLongitude);
		//Elevation = XPLMGetDataf(gElevation);
		//Pitch = XPLMGetDataf(gPlaneTheta);
		//Roll = XPLMGetDataf(gPlanePhi);
		//Heading = XPLMGetDataf(gPlanePsi);

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


};



