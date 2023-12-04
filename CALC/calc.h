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

#ifndef CALC_CALC_H_
#define CALC_CALC_H_

#include "../config.h"
#include "../RCFA/rcfa.h"

double r2d(double r);
double d2r(double d);
double calc_bearing(s_wsp a, s_wsp b);

#if RADIO_MODE==1

#define EQ_RADIUS			6378.1370
#define POLAR_RADIUS		6356.7523142
#define EARTH_ECC			0.081082

double r2d(double r);
double d2r(double d);

double calc_earth_radius(double lat);
double calc_dist(s_wsp p1, s_wsp p2);
double bearing_chg(double brng, double add);
void calc_target(s_wsp *ret, s_wsp p, double brng, double dist);

#endif
#endif /* CALC_CALC_H_ */
