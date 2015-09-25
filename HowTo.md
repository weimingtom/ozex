# ozex HowTo: Building and Using #



## Installing dependencies on Ubuntu 9.10 ##

`sudo apt-get install subversion patch dmake libwxbase2.8-dev libwxgtk2.8-dev libsdl-image1.2-dev g++`

## Getting project from SVN ##

`svn checkout http://ozex.googlecode.com/svn/trunk/ ozex`

## Compiling ##

```
cd ozex
dmake
```

## Preparing config file (bashrc.txt) ##
```
export OZEX_MAPS_PATH=/media/disk/data/mapping/maps
export OZEX_WINDOW_DX=800
export OZEX_WINDOW_DY=600
export OZEX_BUTTON_DX=64
export OZEX_BUTTON_DY=64
export OZEX_BUTTON_GAP=10
export OZEX_MAP_DX=732
export OZEX_MAP_DY=542
export OZEX_LAT=60,382313
export OZEX_LON=29,562426
export OZEX_USE_LOG=1
export OZEX_GPS_LOG=./logs/orehovo.txt
export OZEX_GPS_DEV=/dev/rfcomm0
export OZEX_GPS_BAUDS=4800
```

These settings have following meaning:

OZEX\_MAPS\_PATH: map folder. May contain subfolders - will be scanned recursively.

OZEX\_WINDOW\_DX: application window width.
OZEX\_WINDOW\_DY: application window height.

OZEX\_BUTTON\_DX: Toolbar button width. Designed for your fingers :)
OZEX\_BUTTON\_DY: Toolbar button height
OZEX\_BUTTON\_GAP: Space between buttons

OZEX\_MAP\_DX: Map control width
OZEX\_MAP\_DY: Map control height

OZEX\_LAT: Latitude used at the start of an application
OZEX\_LON: Longitude used at the start of an application

OZEX\_USE\_LOG: 1 - if you want to use NMEA data from text file, not from real device
OZEX\_GPS\_LOG: log file name (with path). Just a text file with raw NMEA data

OZEX\_GPS\_DEV: filename of a real GPS device connected
OZEX\_GPS\_BAUDS: gps baud rate :)


There also a sample config (bashrc\_t91.txt) for ASUS T91 netbook running  Ubuntu 9.10 Netbook Remix, designed for 1024x600 resolution.

## Starting ozex ##
`. bashrc.txt && ./ozex`

## Known issues ##

  * GUI part is rather simple at the moment because it was written in a big hurry in car while driving to trophy raid start.

  * Time is not saved in track logs at the moment.

  * Maps with number of calibration points <= 2 are not supported (that's related more to swampex than ozex).