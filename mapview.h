#include "SDL.h"
#include "map_index.h"
#include "map_render.h"
/*----------------------------------------------------------------------------*/
class MapView : public wxPanel 
{
    DECLARE_CLASS(MapView)
    DECLARE_EVENT_TABLE()
    
private:
    SDL_Surface *screen;
    SDL_Surface *bg;
	SDL_Surface* tile;

	wxBitmap* position;
	wxBitmap* radioOn;
	wxBitmap* radioOff;
	wxBitmap* point;
	wxBitmap* pointnew;
	long pen_x, pen_y, delta_x, delta_y, pressed;
	int mapOn;
	int ozexMapDX, ozexMapDY;
	map_index* collection;
	map_render* render;

    void onPaint(wxPaintEvent &);
    void onEraseBackground(wxEraseEvent &);
    
    void onIdle(wxIdleEvent &);
    void OnMouseDown( wxMouseEvent &event );
    void OnMouseUp( wxMouseEvent &event );
    void OnMouseMove( wxMouseEvent &event );
    void Clear(void);
    void DrawPosition(wxPaintDC &dc);
    void DrawWaypoints(wxPaintDC &dc);
    void DrawWpt(wxPaintDC &dc, int i, double center_lat, double center_lon);
    void UpdateNearestWpt(wxPaintDC &dc, int k, int n, double nearest);
    int FindWptByXyOnMap(long x, long y);
    void EditWpt(int i);
    
public:

    int isPressed();
    void createScreen();
    MapView(wxFrame *parent);
    int isMapOn();
    void mapOnOff();
    wxSize getMapSize();
    map_render* getMapRender();
    ~MapView();
};