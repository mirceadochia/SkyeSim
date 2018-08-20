// antenna.hxx -- FGRadioAntenna: class to represent antenna properties
//
// Written by Adrian Musceac YO8RZZ, started December 2011.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef __cplusplus
# error This library requires C++
#endif

#include <simgear/compiler.h>
#include <simgear/structure/subsystem_mgr.hxx>
#include <Main/fg_props.hxx>

#include <simgear/math/sg_geodesy.hxx>
#include <simgear/debug/logstream.hxx>

using std::string;

class FGRadioAntenna
{
private:
	
/*** load external plot file generated by NEC2. needs to have a txt extension
*	when naming plots, use the following scheme: type_frequencyMHz.txt
*	eg: yagi_110.txt or LPDA_333.txt
*	@param: name of file
*	@return: none
***/
	void load_NEC_antenna_pattern(string type);
	
	int _mirror_y;	
	int _mirror_z;
	int _invert_ground;
	double _heading_deg;
	double _elevation_angle_deg;
	struct AntennaGain {
		double azimuth;
		double elevation;
		double gain;
	};
	SGPath _pattern_file;
	typedef std::vector<AntennaGain*> AntennaPattern;
	AntennaPattern _pattern;
	
public:
	
	FGRadioAntenna(string type);
    ~FGRadioAntenna();

/*** calculate far-field antenna gain on a 3D volume around it
*	@param: bearing to the other station, vertical angle (some call it theta)
*	@return: gain relative to maximum normalized gain. will be negative in all cases
***/
	double calculate_gain(double bearing, double angle);
	
	/// some convenience setters and getters (unused for now)
	inline void set_heading(double heading_deg) {_heading_deg = heading_deg ;};
	inline void set_elevation_angle(double elevation_angle_deg) {_elevation_angle_deg = elevation_angle_deg ;};
};
