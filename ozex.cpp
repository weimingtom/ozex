
#include <iostream>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "SDL.h"

#include "map_loader.h"
//#include "map_container.h"
#include "map_datums.h"
//#include "map_render.h"
#include "wpt_container.h"
#include "log_stream.h"

#include "ozex_gps.h"

#include "mapwindow.h"
#include "ozex.h"


/*----------------------------------------------------------------------------*/
bool SDLApp::OnInit() 
{

	gpsOn = 0;
	waypoints = new wpt_container();

	logstream_to(NULL);

	mapdatums_init("./datums.xml");
	
    	frame = new MapWindow;
	frame->Show();

	//SetTopWindow(frame);

	return true;
}

/*----------------------------------------------------------------------------*/
int SDLApp::OnRun() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cerr << "unable to init SDL: " << SDL_GetError() << '\n';
        
        return -1;
    }
    
    wxPaintEvent event;
    event.SetEventObject(frame->getPanel());
    frame->getPanel()->AddPendingEvent(event);


    return wxApp::OnRun();
}

/*----------------------------------------------------------------------------*/
int SDLApp::OnExit() 
{
	if (gpsOn)
	{
		ozex_gps_close();
		gpsOn = 0;
	}


	delete waypoints;
	mapdatums_done();

	printf("SDLApp::OnExit()\n");
    SDL_Quit();
    
    return wxApp::OnExit();
}

int SDLApp::isGpsOn() 
{
     return gpsOn;
}

void SDLApp::setGpsOn(int on) 
{
     gpsOn = on;
}

wpt_container* SDLApp::getWaypoints()
{
    return waypoints;
}
/*----------------------------------------------------------------------------*/
void SDLApp::gpsOnOff()
{
	if (isGpsOn())
	{
		ozex_gps_close();
		setGpsOn(0);
	}
	else
	{
		int debug = GET_ENV_INT("OZEX_USE_LOG",0);
		
		if (debug)
		{
			ozex_gps_start_fromfile(GET_ENV_STR("OZEX_GPS_LOG","logs/gps.log"));
		}
		else
		{
			ozex_gps_start(GET_ENV_STR("OZEX_GPS_DEV","/dev/rfcomm0"), GET_ENV_INT("OZEX_GPS_BAUDS",9600));
		}	
		setGpsOn(1);
	}
}

time_t lastgpstime =0;
/*----------------------------------------------------------------------------*/
coords_t SDLApp::readGps()
{
	coords_t coords;
	coords.onPosition = 0;
	if (isGpsOn())
	{
		ozex_gps_update();

		time_t thistime = time(NULL);

		//if (!lastgpstime || thistime - lastgpstime >= 3 )
//		{
			lastgpstime = thistime;

			coords.lat = ozex_gps_lat();
			coords.lon = ozex_gps_lon();
			coords.onPosition = 1;
//		}
	}
	return coords;
}

wxString format_dist(double distInM)
{
	wxString fmtStr;
	double distInKm = distInM / 1000;
	if ( distInM < 1000 )
	{
		fmtStr.Printf ( wxT ( " %.0f m" ), distInM );
	}
	else
	if ( distInM < 1000 * 10 )
	{
		fmtStr.Printf ( wxT ( " %2.1f km" ),
				distInKm );
	}
	else
	{
		fmtStr.Printf ( wxT ( " %.0f km" ),
				distInKm );
	}
	return fmtStr;
}

IMPLEMENT_CLASS(SDLApp, wxApp)
IMPLEMENT_APP(SDLApp)

