/*
 * This file is part of RC Flight Assist (RCFA).
 *
 * RCFA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RCFA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Created by Arek "Artyum" Witczak for F3A pattern flying trening
 * Developed since 2014/08
 *
 * Project HomePage: http://www.littlecircuit.com/
 *
 */

#include "../config.h"
#include <math.h>
#include "calc.h"

#include "../RCFA/rcfa.h"
#include "../UART/uart.h"

double r2d(double r) { return r * 180.0 / M_PI; }
double d2r(double d) { return d * M_PI / 180.0; }

double calc_bearing(s_wsp a, s_wsp b) {
	a.lat = d2r(a.lat);
	a.lon = d2r(a.lon);
	b.lat = d2r(b.lat);
	b.lon = d2r(b.lon);

	double y = sin(b.lon-a.lon)*cos(b.lat);
	double x = cos(a.lat)*sin(b.lat)-sin(a.lat)*cos(b.lat)*cos(b.lon-a.lon);
	double brng = r2d(atan2(y,x));
	return fmod(brng+360.0,360.0);
}

#if RADIO_MODE==1

/*
 * http://www.movable-type.co.uk/scripts/latlong.html
 */

//Distance in meters between coordinates
double calc_dist(s_wsp p1, s_wsp p2) {
	#if 1

	//Method 1 (float)
	double a = (p2.lon-p1.lon)*cos(d2r(p1.lat));
	double b = p2.lat-p1.lat;
	double ret = sqrt(a*a+b*b)*M_PI*rcfa.radius;
	return ret / 180.0;

	#else

	//Method 2 (integer optimized)
	p1.lat = d2r(p1.lat); p1.lon = d2r(p1.lon);
	p2.lat = d2r(p2.lat); p2.lon = d2r(p2.lon);
	double dlat = (p2.lat-p1.lat)/2;
	double dlon = (p2.lon-p1.lon)/2;
	p2.lat = d2r(p2.lat);
	double a = sin(dlat)*sin(dlat) + cos(p1.lat) * cos(p2.lat) * sin(dlon)*sin(dlon);
	return rcfa.radius * 2 * atan2(sqrt(a), sqrt(1-a));

	#endif
}

//The function counts the radius of the Earth on a given latitude
double calc_earth_radius(double lat) {
	double ret;
	lat = d2r(lat);

	double a = EQ_RADIUS;
	double b = POLAR_RADIUS;
	double q = a*a*cos(lat);	q = q*q;
	double w = b*b*sin(lat);	w = w*w;
	double e = a*cos(lat);		e = e*e;
	double r = b*sin(lat);		r = r*r;
	ret = sqrt((q+w)/(e+r));

	return ret * 1000.0;
}

double bearing_chg(double brng, double add) {
	return fmod(brng+add,360.0);
}

void calc_target(s_wsp *ret, s_wsp p, double brng, double dist) {
	brng = d2r(brng);
	p.lat = d2r(p.lat);
	p.lon = d2r(p.lon);

	double lat = asin(sin(p.lat)*cos(dist/rcfa.radius) + cos(p.lat)*sin(dist/rcfa.radius)*cos(brng));
	double lon = p.lon + atan2(sin(brng)*sin(dist/rcfa.radius)*cos(p.lat), cos(dist/rcfa.radius)-sin(p.lat)*sin(lat));

	ret->lat = r2d(lat);
	ret->lon = r2d(lon);
}

#endif
