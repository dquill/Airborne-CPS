#pragma warning(disable: 4996)

/*
* NOTES:
* 1. On windows you will need to add these to the linker/input/AdditionalDependencies settings
* glu32.lib
* glaux.lib
*
* 2. If you want to have the xpl go directly to the plugin directory you need to
* set path variables. Currently I set it to build in the top directory of the
* project.
*
* 3. Networking might be easier to do with UDP through the menu options as it is
* available. There are options for things like reading inputs from the network
* and also saving to the local disk. These are found under the settings menu ->
* data input and output, and network options. This is called the Data Set in
* x-plane. info here:
* http://www.x-plane.com/manuals/desktop/#datainputandoutputfromx-plane
* http://www.x-plane.com/?article=data-set-output-table
* http://www.nuclearprojects.com/xplane/info.shtml
*
* Added the define IBM 1 thing because you have to specify it before doing
* compiling. It is system specific. For Macs you must use 'define APL 1' and
* set the ibm to 0. Info about this is here:
* http://www.xsquawkbox.net/xpsdk/docs/macbuild.html
*
* Also added the header file for using the data refs. I might need to add other
* header files for the navigation "XPLMNavigation". "XPLMDataAccess" is to
* read plane info and set other options. Navigation has lookups for gps and
* fms, while the data access is the api for reading and writing plane/sensor
* info. DataRefs:
* http://www.xsquawkbox.net/xpsdk/docs/DataRefs.html
*
*
* Notes added by Michael K.:
* Information on data references for obtaining variable information may be found here:
* http://www.xsquawkbox.net/xpsdk/docs/DataRefs.html
*
*
*/

#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

#include "component/VSpeedIndicatorGaugeRenderer.h"
#include "component/ArtHorizGaugeRenderer.h"
#include "component/ArtHorizGaugeRenderer.h"
#include "component/Transponder.h"
#include "component/NASADecider.h"
#include "component/IntruderInstatiator.h"

#include "XPWidgets.h"
#include "XPStandardWidgets.h"

// Menu Options
#define XBEE_CONFIG_MENU		1  // inItemRef for XBee Configuration Menue
#define II_CONFIG_MENU			2  // inItemRef for Intruder Instantator menu item
#define DRAW_INTRUDERS			3  // temporary until the window widget gets working
#define STOP_DRAWING_INTRUDERS	4  // temporary until the window widget gets working

#define MAX_DEVICE_PATH		200
#define XB_INTRUCTIONS_NUMLINES    9
#define XB_INSTRUCTIONS_LINELENGTH 50

#define NUM_THREAT_CLASSES 4
#define MAX_THREAT_CLASS_NAME_LENGTH 35
/*
* These variables are used in the toggling of gauges during the simulation.
* Currently, there are two Gauges (NUMBER_OF_GAUGES) in development:
* 1) Vertical Speed Indicator Gauge (VSI_GAUGE)
* 2) Artificial Horizon Gauge (AH_GAUGE)
*
* Note that while toggling, the gauges may also be turned off. So there
* will effectively be NUMBER_OF_GAUGES + 1 options.
* */
static const int NUMBER_OF_GAUGES = 2;
static const int NO_GAUGE = 0;
static const int VSI_GAUGE = 1;
static const int AH_GAUGE = 2;

/*
* From http://www.xsquawkbox.net/xpsdk/docs/DataRefs.html
*
* thetaRef: References theta, the pitch relative to the plane normal to the Y axis in degrees - OpenGL coordinates
* phiRef:   References phi, the roll of the aircraft in degrees - OpenGL coordinates
* */
XPLMDataRef latitudeRef, longitudeRef, altitudeRef;
XPLMDataRef headingTrueNorthDegRef, headingTrueMagDegRef;
XPLMDataRef vertSpeedRef, trueAirspeedRef, indAirspeedRef;
XPLMDataRef thetaRef, phiRef;

// These datarefs represent the RGB color of the lighting inside the cockpit
XPLMDataRef	cockpitLightingRed, cockpitLightingGreen, cockpitLightingBlue;

static XPLMWindowID	gExampleGaugePanelDisplayWindow = NULL;
static int gaugeOnDisplay = 1;
static bool debug = true;


// Declares Hotkey Toggles
static XPLMHotKeyID gExampleGaugeHotKey = NULL;
static XPLMHotKeyID debugToggle = NULL;
static XPLMHotKeyID hostileToggle = NULL;
static XPLMHotKeyID tcasToggle = NULL;


//Menu Declarations
int menuContainerID;
XPLMMenuID menuID;
void MenuHandler(void*, void*);

// Menu Delcarations for Intruder Instantiator
XPWidgetID	IIWidget = NULL;  // XBeeWidget
XPWidgetID	IIWindow = NULL;  // XBeeWindow
XPWidgetID	IITextWidget[50] = { NULL }; // IITextWidget[50];
XPWidgetID	IIInsctructionsWidget[NUM_THREAT_CLASSES] = { NULL };
XPWidgetID	IIDrawingEnabledCheckbox = NULL;
int gIIMenuItem; // flag to tells us if the xbee widget is being displayed
bool EnableDrawingIntruders = false;


void CreateIIWidget(int x1, int y1, int w, int h);
int IIHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);
int	IIThreatChoiceHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);

int IIDrawingEnabledCheckboxHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);


// more Menu Declarations added for XBee
XPWidgetID	XBeeWidget = NULL;  // XBeeWidget
XPWidgetID	XBeeWindow = NULL;  // XBeeWindow
XPWidgetID	XBeeTextWidget[50] = { NULL }; // XBeeTextWidget[50];
XPWidgetID	XBeeInsctructionsWidget[XB_INTRUCTIONS_NUMLINES] = { NULL };
XPWidgetID	XBeeEnableRoutingCheckbox = NULL;
int gXBeeMenuItem;  // flag to tell us if the xbee widget is being displayed

void CreateXBeeWidget(int x1, int y1, int w, int h);  //CreateXBeeWidget
int XBeeHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);  //XBeeHandler
int	XBeePortNumHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);
int XBeeRoutingCheckboxHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2);
void GenerateComPortList();
char XBeeCOMPortList[MAX_COMPORT][MAX_DEVICE_PATH] = { {'\0'} };

char XBeeInstructionText[XB_INTRUCTIONS_NUMLINES][XB_INSTRUCTIONS_LINELENGTH] = {   
"   XBee must be COM3.",
"   The selector to left is NOT implemented yet.",
"   Use XCTU to configure XBees as follows:",
"   Baud Rate BD = 115200 [7]",
"   AP = 1    API Mode Without Escapes",
"   AO = 1    API Explicit RX Indicator 0x91",
"   D6 = 1    RTS Flow Control",
"   D7 = 1    CTS Flow Control",
"end"

};

char TCAS_Threat_Classifications[NUM_THREAT_CLASSES][MAX_THREAT_CLASS_NAME_LENGTH] = {
"   RESOLUTION_ADVISORY",
"   TRAFFIC_ADVISORY",
"   PROXIMITY_INTRUDER_TRAFFIC",
"   NON_THREAT_TRAFFIC",
};



// This Global is used to activate hostile mode. This is TEMPORARY
//TODO: Turn on Hostile TA/RA without Globals.
static bool hostile = false;


// The plugin application paths
static char gVSIPluginDataFile[255];
static char gAHPluginDataFile[255];

Aircraft* userAircraft;

VSIGaugeRenderer* vsiGaugeRenderer;
AHGaugeRenderer* ahGaugeRenderer;

concurrency::concurrent_unordered_map<std::string, Aircraft*> intrudingAircraft;
concurrency::concurrent_unordered_map<std::string, ResolutionConnection*> openConnections;
Transponder* transponder;
IntruderInstantiator* intruderInstantiator;

Decider* decider;

/// Used for dragging plugin panel window.
static int	coordInRect(int x, int y, int l, int t, int r, int b);
static int	coordInRect(int x, int y, int l, int t, int r, int b) {
	return ((x >= l) && (x < r) && (y < t) && (y >= b));
}

/// Prototypes for callbacks etc.
static void drawGLSceneForVSI();
static void drawGLSceneForAH();

static int	gaugeDrawingCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon);
static void exampleGaugeHotKey(void* refCon);
static void debugWindow(void* refCon);
static void hostileGauge(void* refCon);
static void exampleGaugePanelWindowCallback(XPLMWindowID inWindowID, void* inRefcon);
static void exampleGaugePanelKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus);
static int exampleGaugePanelMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void* inRefcon);

static XPLMWindowID	gWindow = NULL;
static int gClicked = 0;

//Instrument data variables
float groundSpeed = 0;
float tcasBearing = 0;
float tcasDistance = 0;
float tcasAltitude = 0;
float verticalVelocity = 0;
float indAirspeed = 0;
float indAirspeed2 = 0;
float trueAirspeed = 0;
float verticalSpeedData = 0;
float latREF, lonREF = 0;
float ilatREF, ilonREF = 0;

static void myDrawWindowCallback(XPLMWindowID inWindowID, void* inRefcon);

static void myHandleKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus);

static int myHandleMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void* inRefcon);

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
	/// Handle cross platform differences
#if IBM
	char* pVSIFileName = "Resources\\Plugins\\AirborneCPS\\Vertical_Speed_Indicator\\";
	char* pAHFileName = "Resources\\Plugins\\AirborneCPS\\Artificial_Horizon\\";
#elif LIN
	char* pVSIFileName = "Resources/plugins/AirborneCPS/Vertical_Speed_Indicator/";
	char* pAHFileName = "Resources/plugins/AirborneCPS/Artificial_Horizon/";
#else
	char* pVSIFileName = "Resources:Plugins:AirborneCPS:Vertical_Speed_Indicator:";
	char* pAHFileName = "Resources:Plugins:AirborneCPS:Artificial_Horizon:";
#endif
	/// Setup texture file locations
	XPLMGetSystemPath(gVSIPluginDataFile);
	strcat(gVSIPluginDataFile, pVSIFileName);

	XPLMGetSystemPath(gAHPluginDataFile);
	strcat(gAHPluginDataFile, pAHFileName);


	strcpy(outName, "AirborneCPS");
	strcpy(outSig, "AirborneCPS");
	strcpy(outDesc, "A plug-in for displaying several TCAS gauges.");

	/*Start of Plugin Menu Creation*/
	menuContainerID = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Airborne-CPS", 0, 0);
	menuID = XPLMCreateMenu("Airborne CPS", XPLMFindPluginsMenu(), menuContainerID, MenuHandler, NULL);

	XPLMAppendMenuItem(menuID, "Toggle CPS", (void*)"exampleGaugeHotkey", 1);
	XPLMAppendMenuItem(menuID, "Toggle Hostile", (void*)"hostileToggle", 1);
	XPLMAppendMenuItem(menuID, "Toggle Debug", (void*)"debugToggle", 1);
	XPLMAppendMenuItem(menuID, "XBee Config", (void*)XBEE_CONFIG_MENU, 1);
	XPLMAppendMenuItem(menuID, "Draw Intruders Config", (void*)II_CONFIG_MENU, 1);

	
	//XPLMAppendMenuItem(menuID, "Draw Intruders", (void*)DRAW_INTRUDERS, 1);
	//XPLMAppendMenuItem(menuID, "Stop Drawing Intruders", (void*)STOP_DRAWING_INTRUDERS, 1);


	gXBeeMenuItem = 0;  // set the two widget menu items as not being shown
	gIIMenuItem = 0;

	/*End of Plugin Menu Creation*/

	/* Now we create a window.  We pass in a rectangle in left, top, right, bottom screen coordinates.  We pass in three callbacks. */
	gWindow = XPLMCreateWindow(50, 600, 300, 200, 1, myDrawWindowCallback, myHandleKeyCallback, myHandleMouseClickCallback, NULL);

	/// Register so that our gauge is drawing during the Xplane gauge phase
	XPLMRegisterDrawCallback(gaugeDrawingCallback, XPLM_PHASE_GAUGES, 0, NULL);

	/// Create our window, setup datarefs and register our hotkey.
	gExampleGaugePanelDisplayWindow = XPLMCreateWindow(1024, 256, 1280, 0, 1, exampleGaugePanelWindowCallback, exampleGaugePanelKeyCallback, exampleGaugePanelMouseClickCallback, NULL);

	vertSpeedRef = XPLMFindDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot");

	latitudeRef = XPLMFindDataRef("sim/flightmodel/position/latitude");
	longitudeRef = XPLMFindDataRef("sim/flightmodel/position/longitude");
	altitudeRef = XPLMFindDataRef("sim/flightmodel/position/elevation");

	headingTrueMagDegRef = XPLMFindDataRef("sim/flightmodel/position/mag_psi");
	headingTrueNorthDegRef = XPLMFindDataRef("sim/flightmodel/position/true_psi");

	trueAirspeedRef = XPLMFindDataRef("sim/flightmodel/position/true_airspeed");
	indAirspeedRef = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");

	thetaRef = XPLMFindDataRef("sim/flightmodel/position/theta");
	phiRef = XPLMFindDataRef("sim/flightmodel/position/phi");

	cockpitLightingRed = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_r");
	cockpitLightingGreen = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_g");
	cockpitLightingBlue = XPLMFindDataRef("sim/graphics/misc/cockpit_light_level_b");

	//Assign keybinds for hotkeys
	debugToggle = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag, "F8", debugWindow, NULL);
	gExampleGaugeHotKey = XPLMRegisterHotKey(XPLM_VK_F9, xplm_DownFlag, "F9", exampleGaugeHotKey, NULL);
	hostileToggle = XPLMRegisterHotKey(XPLM_VK_F10, xplm_DownFlag, "F10", hostileGauge, NULL);

	Transponder::initNetworking();
	std::string myMac = Transponder::getHardwareAddress();



	LLA currentPos = LLA::ZERO;
	userAircraft = new Aircraft(myMac, "127.0.0.1", currentPos, Angle::ZERO, Velocity::ZERO, Angle::ZERO, Angle::ZERO);
	std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	userAircraft->positionCurrentTime = msSinceEpoch;
	userAircraft->positionOldTime = msSinceEpoch;

	decider = new NASADecider(userAircraft, &openConnections);

	vsiGaugeRenderer = new VSIGaugeRenderer(gVSIPluginDataFile, decider, userAircraft, &intrudingAircraft);
	vsiGaugeRenderer->loadTextures();

	ahGaugeRenderer = new AHGaugeRenderer(gAHPluginDataFile, decider, userAircraft, &intrudingAircraft);
	ahGaugeRenderer->loadTextures();

	// start broadcasting location, and listening for aircraft
	transponder = new Transponder(userAircraft, &intrudingAircraft, &openConnections, decider);
	transponder->initXBee(3);  // ******* get this value from the Menu ************
	transponder->start();
	intruderInstantiator = IntruderInstantiator::getIntruderInstatiator(&intrudingAircraft);
	
	return 1;
}


PLUGIN_API void	XPluginStop(void) {
	/// Clean up
	if (gXBeeMenuItem == 1) {
		XPDestroyWidget(XBeeWidget, 1);
		gXBeeMenuItem = 0;
	}

	XPLMUnregisterDrawCallback(gaugeDrawingCallback, XPLM_PHASE_GAUGES, 0, NULL);
	XPLMDestroyWindow(gWindow);
	XPLMUnregisterHotKey(gExampleGaugeHotKey);
	XPLMUnregisterHotKey(debugToggle);
	XPLMUnregisterHotKey(hostileToggle);
	XPLMDestroyWindow(gExampleGaugePanelDisplayWindow);

	delete vsiGaugeRenderer;
	delete ahGaugeRenderer;
	delete transponder;
}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID	inFromWho, int	inMessage, void* inParam) {}

/* The callback responsible for drawing the gauge during the X-Plane gauge drawing phase. */
int	gaugeDrawingCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon) {
	// Do the actual drawing, but only if the window is active
	if (gaugeOnDisplay == NO_GAUGE) {
		/*
		* Do nothing. The display window is not active.
		* */

	}
	else if (gaugeOnDisplay == VSI_GAUGE) {
		LLA updated = { Angle{ XPLMGetDatad(latitudeRef), Angle::AngleUnits::DEGREES },
			Angle{ XPLMGetDatad(longitudeRef), Angle::AngleUnits::DEGREES },
			Distance{ XPLMGetDatad(altitudeRef), Distance::DistanceUnits::METERS } };
		Velocity updatedVvel = Velocity(XPLMGetDataf(vertSpeedRef), Velocity::VelocityUnits::FEET_PER_MIN);
		std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		userAircraft->lock.lock();
		userAircraft->positionOld = userAircraft->positionCurrent;
		userAircraft->positionOldTime = userAircraft->positionCurrentTime;

		userAircraft->positionCurrent = updated;
		userAircraft->positionCurrentTime = msSinceEpoch;

		userAircraft->verticalVelocity = updatedVvel;
		userAircraft->heading = Angle(XPLMGetDataf(headingTrueMagDegRef), Angle::AngleUnits::DEGREES);
		userAircraft->trueAirspeed = Velocity(XPLMGetDataf(trueAirspeedRef), Velocity::VelocityUnits::METERS_PER_S);

		userAircraft->theta = Angle(XPLMGetDataf(thetaRef), Angle::AngleUnits::DEGREES);
		userAircraft->phi = Angle(XPLMGetDataf(phiRef), Angle::AngleUnits::DEGREES);

		userAircraft->lock.unlock();

		drawGLSceneForVSI();

	}
	else if (gaugeOnDisplay == AH_GAUGE) {
		LLA updated = { Angle{ XPLMGetDatad(latitudeRef), Angle::AngleUnits::DEGREES },
			Angle{ XPLMGetDatad(longitudeRef), Angle::AngleUnits::DEGREES },
			Distance{ XPLMGetDatad(altitudeRef), Distance::DistanceUnits::METERS } };
		Velocity updatedVvel = Velocity(XPLMGetDataf(vertSpeedRef), Velocity::VelocityUnits::FEET_PER_MIN);
		std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		userAircraft->lock.lock();
		userAircraft->positionOld = userAircraft->positionCurrent;
		userAircraft->positionOldTime = userAircraft->positionCurrentTime;

		userAircraft->positionCurrent = updated;
		userAircraft->positionCurrentTime = msSinceEpoch;

		userAircraft->verticalVelocity = updatedVvel;
		userAircraft->heading = Angle(XPLMGetDataf(headingTrueMagDegRef), Angle::AngleUnits::DEGREES);
		userAircraft->trueAirspeed = Velocity(XPLMGetDataf(trueAirspeedRef), Velocity::VelocityUnits::METERS_PER_S);

		userAircraft->theta = Angle(XPLMGetDataf(thetaRef), Angle::AngleUnits::DEGREES);
		userAircraft->phi = Angle(XPLMGetDataf(phiRef), Angle::AngleUnits::DEGREES);

		userAircraft->lock.unlock();

		drawGLSceneForAH();

	}
	return 1;
}


/* This callback does not do any drawing as such.
* We use the mouse callback below to handle dragging of the window
* X-Plane will automatically do the redraw. */
void exampleGaugePanelWindowCallback(XPLMWindowID inWindowID, void* inRefcon) {}

/* Our key handling callback does nothing in this plugin.  This is ok;
* we simply don't use keyboard input.*/
void exampleGaugePanelKeyCallback(XPLMWindowID inWindowID, char inKey,
	XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus) {}

/* Our mouse click callback updates the position that the windows is dragged to. */
int exampleGaugePanelMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void* inRefcon) {
	int	dX = 0, dY = 0;
	int	weight = 0, height = 0;
	int	left, top, right, bottom;

	int	gDragging = 0;

	if (gaugeOnDisplay == NO_GAUGE)
		return 0;

	/// Get the windows current position
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

	switch (inMouse) {
	case XPLM_MOUSE_DOWN:
		/// Test for the mouse in the top part of the window
		if (coordInRect(x, y, left, top, right, top - 15)) {
			dX = x - left;
			dY = y - top;
			weight = right - left;
			height = bottom - top;
			gDragging = 1;
		}
		break;
	case XPLM_MOUSE_DRAG:
		/// We are dragging so update the window position
		if (gDragging) {
			left = (x - dX);
			right = left + weight;
			top = (y - dY);
			bottom = top + height;
			XPLMSetWindowGeometry(inWindowID, left, top, right, bottom);
		}
		break;
	case xplm_MouseUp:
		gDragging = 0;
		break;
	}
	return 1;
}

/**
* Toggle between displaying one of the N NUMBER_OF_GAUGES, or no gauge.
* */
void exampleGaugeHotKey(void* refCon) {
	gaugeOnDisplay = ((gaugeOnDisplay + 1) % (NUMBER_OF_GAUGES + 1));
}

void debugWindow(void* refCon) {
	debug = !debug;
}

void hostileGauge(void* refCon) {
	//TODO: Toggle Hostile Mode.
	vsiGaugeRenderer->markHostile();
	ahGaugeRenderer->markHostile();
}

/// Draws the textures that make up the gauge for the Vertical Speed Indicator
void drawGLSceneForVSI() {
	vsitextureconstants::GlRgb8Color cockpit_lighting = { XPLMGetDataf(cockpitLightingRed), XPLMGetDataf(cockpitLightingGreen), XPLMGetDataf(cockpitLightingBlue) };
	vsiGaugeRenderer->render(cockpit_lighting);
}

/**
* Draws the textures that make up the gauge for the Artificial Horizon
* */
void drawGLSceneForAH() {
	ahtextureconstants::GlRgb8Color cockpit_lighting = { XPLMGetDataf(cockpitLightingRed), XPLMGetDataf(cockpitLightingGreen), XPLMGetDataf(cockpitLightingBlue) };
	ahGaugeRenderer->render(cockpit_lighting);
}

/* This callback does the work of drawing our window once per sim cycle each time
* it is needed.  It dynamically changes the text depending on the saved mouse
* status.  Note that we don't have to tell X-Plane to redraw us when our text
* changes; we are redrawn by the sim continuously. */
/// This function draws the window that is currently being used for debug text rendering
void myDrawWindowCallback(XPLMWindowID inWindowID, void* inRefcon) {
	int		left, top, right, bottom;
	static float color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */

	if (debug) {
		/* First we get the location of the window passed in to us. */
		XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

		/* We now use an XPLMGraphics routine to draw a translucent dark rectangle that is our window's shape. */
		XPLMDrawTranslucentDarkBox(left, top, right, bottom);

		/* Finally we draw the text into the window, also using XPLMGraphics routines.  The NULL indicates no word wrapping. */
		char positionBuf[128];
		snprintf(positionBuf, 128, "Position: (%.3f, %.3f, %.3f)", XPLMGetDataf(latitudeRef), XPLMGetDataf(longitudeRef), XPLMGetDataf(altitudeRef));
		XPLMDrawString(color, left + 5, top - 20, positionBuf, NULL, XPLM_FONT_BASIC);

		/* Drawing the LLA for each intruder aircraft in the intruding_aircraft set */
		int offsetYPxls = 40;

		for (auto& iter = intrudingAircraft.cbegin(); iter != intrudingAircraft.cend(); ++iter) {
			Aircraft* intruder = iter->second;

			intruder->lock.lock();
			LLA const intruderPos = intruder->positionCurrent;
			LLA const intruderPosOld = intruder->positionOld;
			std::string intrId = intruder->id;
			intruder->lock.unlock();

			Calculations c = ((NASADecider*)decider)->getCalculations(intrId);

			positionBuf[0] = '\0';
			snprintf(positionBuf, 128, "intrPos: (%.3f, %.3f, %3f)", intruderPos.latitude.toDegrees(), intruderPos.longitude.toDegrees(), intruderPos.altitude.toMeters());
			XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
			offsetYPxls += 20;

			positionBuf[0] = '\0';
			snprintf(positionBuf, 128, "modTauS: %.3f", c.modTau);
			XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
			offsetYPxls += 20;

			positionBuf[0] = '\0';
			snprintf(positionBuf, 128, "altSepFt: %.3f", c.altSepFt);
			XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
			offsetYPxls += 20;
		}

		positionBuf[0] = '\0';
		snprintf(positionBuf, 128, "Theta (degrees): %.3f", XPLMGetDataf(thetaRef));
		XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
		offsetYPxls += 20;

		positionBuf[0] = '\0';
		snprintf(positionBuf, 128, "Phi (degrees): %.3f", XPLMGetDataf(phiRef));
		XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
		offsetYPxls += 20;

		decider->recommendationRangeLock.lock();
		RecommendationRange positive = decider->positiveRecommendationRange;
		RecommendationRange neg = decider->negativeRecommendationRange;
		decider->recommendationRangeLock.unlock();

		float vMin = 1.0f * mathutil::clampd(positive.minVerticalSpeed.toFeetPerMin(), -4000.0, 4000.0);
		float vMax = 1.0f * mathutil::clampd(positive.maxVerticalSpeed.toFeetPerMin(), -4000.0, 4000.0);

		positionBuf[0] = '\0';
		snprintf(positionBuf, 128, "Decider (+) vMin, vMax : %.3f, %.3f", vMin, vMax);
		XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);
		offsetYPxls += 20;

		vMin = 1.0f * mathutil::clampd(neg.minVerticalSpeed.toFeetPerMin(), -4000.0, 4000.0);
		vMax = 1.0f * mathutil::clampd(neg.maxVerticalSpeed.toFeetPerMin(), -4000.0, 4000.0);

		positionBuf[0] = '\0';
		snprintf(positionBuf, 128, "Decider (-) vMin, vMax : %.3f, %.3f", vMin, vMax);
		XPLMDrawString(color, left + 5, top - offsetYPxls, (char*)positionBuf, NULL, XPLM_FONT_BASIC);

	}
}

/* Our key handling callback does nothing in this plugin.  This is ok; we simply don't use keyboard input. */
void myHandleKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags,
	char inVirtualKey, void* inRefcon, int losingFocus) {}

/*Our mouse click callback toggles the status of our mouse variable
* as the mouse is clicked.  We then update our text on the next sim
* cycle. */
int myHandleMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void* inRefcon) {
	/* If we get a down or up, toggle our status click.  We will
	* never get a down without an up if we accept the down. */
	if ((inMouse == XPLM_MOUSE_DOWN) || (inMouse == xplm_MouseUp))
		gClicked = 1 - gClicked;

	/* Returning 1 tells X-Plane that we 'accepted' the click; otherwise
	* it would be passed to the next window behind us.  If we accept
	* the click we get mouse moved and mouse up callbacks, if we don't
	* we do not get any more callbacks.  It is worth noting that we
	* will receive mouse moved and mouse up even if the mouse is dragged
	* out of our window's box as long as the click started in our window's
	* box. */
	return 1;
}

void MenuHandler(void* in_menu_ref, void* in_item_ref) {

	int menuSelection = (int)in_item_ref;

	switch (menuSelection) {
	case XBEE_CONFIG_MENU :
		if (gXBeeMenuItem == 0)
		{
			CreateXBeeWidget(50, 712, 700, 300);  //left, top, right, bottom
			gXBeeMenuItem = 1;
		}
		else
		{
			if (!XPIsWidgetVisible(XBeeWidget)) {
				XPShowWidget(XBeeWidget);
			}
		}
		break;

	case II_CONFIG_MENU :
		if (gIIMenuItem == 0)
		{
			CreateIIWidget(50, 712, 700, 300);  //left, top, right, bottom
			gIIMenuItem = 1;
		}
		else
		{
			if (!XPIsWidgetVisible(IIWidget))
				XPShowWidget(IIWidget);
		}
		break;

	case DRAW_INTRUDERS :
		AcquireAircraft();
		break;

	case STOP_DRAWING_INTRUDERS :
		XPLMReleasePlanes();
		break;

	}

}

//__________________________________________________________________

// This will create our Intruder Instantiator dialog.
void CreateIIWidget(int x, int y, int w, int h)
{
	int Index;

	// right side
	int x2 = x + w;

	// bottom - i could be wrong. correct this comment if so. could be top
	int y2 = y - h;


	// Create the Main Widget window.
	IIWidget = XPCreateWidget(x, y, x2, y2,
		1,										// Visible
		"Draw Intruders Configuration - SUNY Oswego",		// desc
		1,										// is root
		NULL,									// not in a container
		xpWidgetClass_MainWindow);


	// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.  
	XPSetWidgetProperty(IIWidget, xpProperty_MainWindowHasCloseBoxes, 1);

	// Draw/Stop Drawing Checkbox
	IIDrawingEnabledCheckbox = XPCreateWidget(x + 25, y - 25, x + 50, y - 50,
		1, // visible
		"      Enable Drawing Intruders",
		0, // root
		IIWidget,
		xpWidgetClass_Button);
	XPSetWidgetProperty(IIDrawingEnabledCheckbox, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(IIDrawingEnabledCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(IIDrawingEnabledCheckbox, xpProperty_ButtonState, EnableDrawingIntruders);
	XPAddWidgetCallback(IIDrawingEnabledCheckbox, (XPWidgetFunc_t)IIDrawingEnabledCheckboxHandler);

	// Print each line of instructions.
	for (Index = 0; Index < NUM_THREAT_CLASSES; Index++)
	{

		// Create a text widget                left ,   top   ,    right      , bottom
		IITextWidget[Index] = XPCreateWidget(
			x + 25, // left
			y - (100 + (Index * 20)), // top
			x + 50, // right
			y - (112 + (Index * 20)), // bottom
			1,	// is Visible
			TCAS_Threat_Classifications[Index],  // Threat Classification
			0,		// not root
			IIWidget,
			xpWidgetClass_Button);
		XPSetWidgetProperty(IITextWidget[Index], xpProperty_ButtonType, xpRadioButton);
		XPSetWidgetProperty(IITextWidget[Index], xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
		XPAddWidgetCallback(IITextWidget[Index], (XPWidgetFunc_t)IIThreatChoiceHandler);
		XPSetWidgetProperty(IITextWidget[2], xpProperty_ButtonState, 1);  // check the PROXIMITY_INTRUDER_TRAFFIC checkbox
	}


	// Register our widget handler
	XPAddWidgetCallback(IIWidget, (XPWidgetFunc_t)IIHandler);
}

//__________________________________________________________________

int IIDrawingEnabledCheckboxHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2) {
	if (inMessage == xpMsg_ButtonStateChanged) {
		EnableDrawingIntruders = inParam2;
		if (EnableDrawingIntruders) {
			AcquireAircraft();
		}
		else {
			XPLMReleasePlanes();
		}
		return 1;
	}
	return 0;
}

int	IIThreatChoiceHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2) {

	if (inMessage == xpMsg_ButtonStateChanged) {
		for (int i = 0; i < NUM_THREAT_CLASSES; i++) {
			if ((long)IITextWidget[i] != inParam1 && inParam2 == 1) {
				XPSetWidgetProperty(IITextWidget[i], xpProperty_ButtonState, 0);
			}
			else if ((long)IITextWidget[i] == inParam1) {
				char buf[100];
				XPGetWidgetDescriptor(IITextWidget[i], buf, 100);
				int threatNum = NUM_THREAT_CLASSES - i;
				intruderInstantiator->getThreatChoice(threatNum);
			}

		}
		return 1;
	}

	return 0;
}

int	IIHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2)
{
	std::string debugstring = "IIHandler Hit. gIIMenuItem = " + std::to_string(gIIMenuItem) + "inMessage: " + std::to_string(inMessage) + "\n";
	XPLMDebugString(debugstring.c_str());

	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (gIIMenuItem == 1)
		{ 
			XPHideWidget(IIWidget);
		}
		return 1;
	}

	return 0;
}





// This will create our widget dialog.
void CreateXBeeWidget(int x, int y, int w, int h)  
{
	int Index;

	int x2 = x + w;
	int y2 = y - h;
	GenerateComPortList();
	XPLMDebugString("\nCom Port list generated\n");
	// Create the Main Widget window.
	XBeeWidget = XPCreateWidget(x, y, x2, y2,
		1,										// Visible
		"XBee Configuration - SUNY Oswego",		// desc
		1,										// is root
		NULL,									// not in a container
		xpWidgetClass_MainWindow);

	XPLMDebugString("XPCreateWidget Returned\n");
	// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.  
	XPSetWidgetProperty(XBeeWidget, xpProperty_MainWindowHasCloseBoxes, 1);


	// Print each line of instructions.
	for (Index = 0; Index < 50; Index++)
	{
		if (strcmp(XBeeCOMPortList[Index], "end") == 0) { break; }

		// Create a text widget
		XBeeTextWidget[Index] = XPCreateWidget(x + 10, y - (30 + (Index * 20)), x + 40, y - (42 + (Index * 20)),
			1,	// is Visible
			XBeeCOMPortList[Index],// desc
			0,		// not root
			XBeeWidget,
			xpWidgetClass_Button);
		XPSetWidgetProperty(XBeeTextWidget[Index], xpProperty_ButtonType, xpRadioButton);
		XPSetWidgetProperty(XBeeTextWidget[Index], xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
		XPAddWidgetCallback(XBeeTextWidget[Index], (XPWidgetFunc_t)XBeePortNumHandler);
	}
	XPLMDebugString("Created Radio Buttons\n");
	// Instructions section
	// Print each line of instructions.
	for (Index = 0; Index < 50; Index++)
	{
		if (strcmp(XBeeInstructionText[Index], "end") == 0) { break; }

		// Create a text widget
		XBeeInsctructionsWidget[Index] = XPCreateWidget(x + 350, y - (30 + (Index * 20)), x2 - 50, y - (42 + (Index * 20)),
			1,	// is Visible
			XBeeInstructionText[Index],// desc
			0,		// not root
			XBeeWidget,
			xpWidgetClass_Caption);

	}

	XPLMDebugString("Wrote Instructions\n");
	// enable/disable routing checkbox
	XBeeEnableRoutingCheckbox = XPCreateWidget(x + 25, y2 + 100, x + 65, y2 + 50,
		1,	// Visible
		"          Route XBee <-> UDP",// desc
		0,		// root
		XBeeWidget,
		xpWidgetClass_Button);
	XPSetWidgetProperty(XBeeEnableRoutingCheckbox, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(XBeeEnableRoutingCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty (XBeeEnableRoutingCheckbox, xpProperty_ButtonState, transponder->enableXBeeRouting);
	XPAddWidgetCallback(XBeeEnableRoutingCheckbox, (XPWidgetFunc_t)XBeeRoutingCheckboxHandler);

	XPLMDebugString("Registered Handlers\n");

	// Register our widget handler
	XPAddWidgetCallback(XBeeWidget, (XPWidgetFunc_t)XBeeHandler);
	XPLMDebugString("Registered CAllback\n");
}

int XBeeRoutingCheckboxHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2) {
	
	if (inMessage == xpMsg_ButtonStateChanged) {
		transponder->enableXBeeRouting = inParam2;
		return 1;
	}
	return 0;
}

int	XBeePortNumHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2) {

	if (inMessage == xpMsg_ButtonStateChanged) {
		for (int i = 0; i < 50; i++) {
			if ((long)XBeeTextWidget[i] != inParam1 && inParam2 == 1) {
				XPSetWidgetProperty(XBeeTextWidget[i], xpProperty_ButtonState, 0);
			}
			else if ((long)XBeeTextWidget[i] == inParam1) {
				char buf[100];
				XPGetWidgetDescriptor(XBeeTextWidget[i], buf, 100);

			}

		}
		return 1;
	}

	return 0;
}

// This is our widget handler.  In this example we are only interested when the close box is pressed.
int	XBeeHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2)
{

	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (gXBeeMenuItem == 1)
		{
			XPHideWidget(XBeeWidget);
		}
		return 1;
	}

	return 0;
}


void GenerateComPortList() {
	//char XBeeCOMPortList[MAX_COMPORT][MAX_DEVICE_PATH] = { {'\0'} };
	TCHAR path[5000];
	int comPortListLineNum = 0;


	for (unsigned i = 1; i <= MAX_COMPORT; ++i) {

		// iterate through possible com ports by name

		TCHAR buffer[10];
		_stprintf(buffer, "COM%u", i);
		DWORD result = QueryDosDevice(buffer, path, (sizeof path) / 2);


		if (result != 0) {
			sprintf(XBeeCOMPortList[comPortListLineNum], "   COM%u          %s", i, path);
			XPLMDebugString(XBeeCOMPortList[comPortListLineNum]);
			comPortListLineNum++;
		}
		else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			XPLMDebugString("Airborne-CPS Error: COM Portpath buffer too small");

		}
		sprintf(XBeeCOMPortList[comPortListLineNum], "end");
	}

}
