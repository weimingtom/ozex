#ifndef __OZEX_H
#define OZEX_H

/*----------------------------------------------------------------------------*/
enum 
{
    IDF_FRAME = wxID_HIGHEST + 1,
    IDP_PANEL,    
    IDP_ZOOMIN,
    IDP_ZOOMOUT,
 	IDP_NEXTMAP,
 	IDP_PREVMAP,
 	IDP_TOOLS,
 	IDP_ADDPOINT,
	IDP_LOADPOINTS,
	IDP_POINTSLIST,
	IDP_INFO,
	IDP_NIGHT,
	IDP_TARGET,
	IDP_MAPONOFF,
	IDP_PAGEUP,
	IDP_PAGEDOWN
};

struct coords_t
{
	double lat, lon;
	int onPosition;
};

//FIXME: potential race condition
#define GET_ENV_INT(a,b) (getenv(a)?atoi(getenv(a)):(b))
#define GET_ENV_STR(a,b) (getenv(a)?getenv(a):(b))
#define GET_ENV_FLOAT(a,b) (getenv(a)?atof(getenv(a)):(b))

class MapWindow;
wxString format_dist(double distInM);
/*----------------------------------------------------------------------------*/
class SDLApp : public wxApp 
{
    DECLARE_CLASS(SDLApp)
    
private:
    MapWindow *frame;
    int gpsOn;
    wpt_container* waypoints;
    void setGpsOn(int);

public:
    bool OnInit();

    int OnRun();

    int OnExit();

    int isGpsOn();
    wpt_container* getWaypoints();
    void gpsOnOff();
    coords_t readGps();

};

DECLARE_APP(SDLApp);
#endif
