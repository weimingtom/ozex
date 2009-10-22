#ifndef __OZEX_GPS_INCLUDED
#define __OZEX_GPS_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "nmeap.h"


void ozex_gps_start(const char *port, int bauds);
void ozex_gps_start_fromfile(const char *port);
void ozex_gps_update();
void ozex_gps_close();

double ozex_gps_lat();
double ozex_gps_lon();


#endif
