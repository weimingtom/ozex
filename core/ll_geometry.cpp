/**
 * swampex, a map processing library
 *
 * Authors: 
 *
 * Daniil Smelov <dn.smelov@gmail.com>
 *
 * Copyright (C) 2006-2009 Daniil Smelov, Slava Baryshnikov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <math.h>
#include "ll_geometry.h"

/*--------------------------------------------------------------------------*/
#define	PI 3.14159265
#define absd(x) (((x) < 0) ? (-(x)) : (x))
#define deg2rad (PI / 180)
#define rad2deg (180/PI)
#define rad(x) (deg2rad*(x))
#define deg(x) (rad2deg*(x))

/*--------------------------------------------------------------------------*/
void llbd_to_ll(	double lat0, double lon0, double brng, double dist,
				double &lat1, double &lon1)
{
	double a = 6378137, b = 6356752.3142,  f = 1/298.257223563;  // WGS-84 ellipsiod
	double s = dist;
	double alpha1 = rad(brng);

	double lat = rad(lat0);
	double lon = rad(lon0);

	double sinAlpha1 = sin(alpha1), cosAlpha1 = cos(alpha1);

	double tanU1 = (1-f) * tan(lat);
	double cosU1 = 1 / sqrt((1 + tanU1*tanU1)), sinU1 = tanU1*cosU1;

	double sigma1 = atan2(tanU1, cosAlpha1);
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1 - sinAlpha*sinAlpha;
	double uSq = cosSqAlpha * (a*a - b*b) / (b*b);

	double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
	double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));

	double sigma = s / (b*A), sigmaP = 2*PI;

	double cos2SigmaM;
	double sinSigma;
	double deltaSigma;
	double cosSigma;

	while (absd(sigma-sigmaP) > 1e-12)
	{
		cos2SigmaM = cos(2*sigma1 + sigma);
		sinSigma = sin(sigma), cosSigma = cos(sigma);
		deltaSigma =	B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-
							B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
		sigmaP = sigma;
		sigma = s / (b*A) + deltaSigma;
	}

	double tmp = sinU1*sinSigma - cosU1*cosSigma*cosAlpha1;
	double lat2 = atan2(sinU1*cosSigma + cosU1*sinSigma*cosAlpha1,
						(1-f)*sqrt(sinAlpha*sinAlpha + tmp*tmp));

	double lambda = atan2(sinSigma*sinAlpha1, cosU1*cosSigma - sinU1*sinSigma*cosAlpha1);
	double C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
	double L = lambda - (1-C) * f * sinAlpha *
      (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));

  	double revAz = atan2(sinAlpha, -tmp);  // final bearing

	lat1 = lat2*180/PI;
	lon1 = (lon+L)*180/PI;
}

/*--------------------------------------------------------------------------*/
double ll_to_distance(double lt1, double lg1, double lt2, double lg2)
{
	double lat1 = rad(lt1);
	double lon1 = rad(lg1);
	double lat2 = rad(lt2);
	double lon2 = rad(lg2);

	double a = 6378137, b = 6356752.3142,  f = 1/298.257223563;  // WGS-84 ellipsiod
 	double L = lon2 - lon1;
 	double U1 = atan((1-f) * tan(lat1));
 	double U2 = atan((1-f) * tan(lat2));
 	double sinU1 = sin(U1), cosU1 = cos(U1);
	double sinU2 = sin(U2), cosU2 = cos(U2);

	double lambda = L, lambdaP = 2*PI;
	double iterLimit = 20;

    double cosSqAlpha;
	double cos2SigmaM;
    double cosSigma, sinSigma, sigma;


	while (absd(lambda-lambdaP) > 1e-12 && --iterLimit>0)
	{
		double sinLambda = sin(lambda), cosLambda = cos(lambda);
		sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) +
						(cosU1*sinU2-sinU1*cosU2*cosLambda) *
						(cosU1*sinU2-sinU1*cosU2*cosLambda));

		if (sinSigma==0)
			return 0;  // co-incident points

		cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
		sigma = atan2(sinSigma, cosSigma);
		double sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
		cosSqAlpha = 1 - sinAlpha*sinAlpha;
		cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;

		//if (isNaN(cos2SigmaM)) cos2SigmaM = 0;  // equatorial line: cosSqAlpha=0 (§6)

		double C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1-C) * f * sinAlpha *
				(sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
	}

	if (iterLimit==0)
		return 0;  // formula failed to converge

	double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
	double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
	double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
	double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-
						B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
	double s = b*A*(sigma-deltaSigma);

	return s;
}

/*--------------------------------------------------------------------------*/
double ll_to_bearing(double lt1, double lg1, double lt2, double lg2)
{

	double b;

	double lat1 = rad(lt1);
	double lon1 = rad(lg1);
	double lat2 = rad(lt2);
	double lon2 = rad(lg2);

	double y =	sin(lon2 - lon1) * cos(lat2);
  	double x =	cos(lat1) * sin(lat2) -
				sin(lat1) * cos(lat2) * cos(lon2 - lon1);
	b = atan2(y, x);
	b = deg(b);

	return b;
}


/*--------------------------------------------------------------------------*/
double bearing_to_direction(double degs)
{
	if(degs >= 0)
		return degs;
	
	return 360.0 + degs;
}
