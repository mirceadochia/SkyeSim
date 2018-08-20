// dynamics.hxx - a class to manage the higher order airport ground activities
// Written by Durk Talsma, started December 2004.
//
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
//
// $Id$


#ifndef _AIRPORT_DYNAMICS_HXX_
#define _AIRPORT_DYNAMICS_HXX_

#include <set>

#include <simgear/structure/SGReferenced.hxx>
#include <simgear/timing/timestamp.hxx>

#include <ATC/trafficcontrol.hxx>
#include <ATC/GroundController.hxx>

#include "airports_fwd.hxx"
#include "parking.hxx"
#include "runwayprefs.hxx"

class ParkingAssignment
{
public:
  ParkingAssignment();
  ~ParkingAssignment();
  
// create a parking assignment (and mark it as unavailable)
  ParkingAssignment(FGParking* pk, FGAirportDynamics* apt);
  
  ParkingAssignment(const ParkingAssignment& aOther);
  void operator=(const ParkingAssignment& aOther);
  
  bool isValid() const;
  FGParking* parking() const;
  
  void release();
private:
  void clear();

  class ParkingAssignmentPrivate;
  ParkingAssignmentPrivate* _sharedData;
};

class FGAirportDynamics : public SGReferenced
{

private:
    FGAirport* _ap;

    typedef std::set<FGParkingRef> ParkingSet;
    // if a parking item is in this set, it is occupied
    ParkingSet occupiedParkings;

    FGRunwayPreference   rwyPrefs;
    FGStartupController  startupController;
    FGTowerController    towerController;
    FGApproachController approachController;
    FGGroundController   groundController;

    time_t lastUpdate;
    std::string prevTrafficType;
    stringVec landing;
    stringVec takeoff;
    stringVec milActive, comActive, genActive, ulActive;
    stringVec *currentlyActive;

    int atisSequenceIndex;
    double atisSequenceTimeStamp;

    std::string chooseRunwayFallback();
    bool innerGetActiveRunway(const std::string &trafficType, int action, std::string &runway, double heading);
    std::string chooseRwyByHeading(stringVec rwys, double heading);

    FGParking* innerGetAvailableParking(double radius, const std::string & flType,
                               const std::string & airline,
                               bool skipEmptyAirlineCode);

    std::string fallbackGetActiveRunway(int action, double heading);

    // runway preference fallback data
    SGTimeStamp _lastFallbackUpdate;
    FGRunwayList _fallbackDepartureRunways,
        _fallbackArrivalRunways;
    unsigned int _fallbackRunwayCounter;

public:
    FGAirportDynamics(FGAirport* ap);
    virtual ~FGAirportDynamics();

    void init();
  
    double getElevation() const;
    const std::string getId() const;
  
    FGAirport* parent() const
    { return _ap; }
  
    void getActiveRunway( const std::string& trafficType,
                          int action,
                          std::string& runway,
                          double heading );

    bool hasParkings() const;

    /**
     * retrieve an available parking by GateID, or -1 if no suitable
     * parking location could be found.
     */
    ParkingAssignment getAvailableParking(double radius, const std::string& fltype,
                          const std::string& acType, const std::string& airline);

    void setParkingAvailable(FGParking* park, bool available);
  
    bool isParkingAvailable(FGParking* parking) const;
  
    void releaseParking(FGParking* id);

    FGParkingList getParkings(bool onlyAvailable, const std::string& type) const;

    /**
     * Find a parking gate index by name. Note names are often not unique
     * in our data, so will return the first match. If the parking is found,
     * it will be marked as in-use (unavailable)
     */
    ParkingAssignment getParkingByName(const std::string& name) const;

    // ATC related functions.
    FGStartupController    *getStartupController()    {
        return &startupController;
    };
    FGGroundController        *getGroundController()        {
        return &groundController;
    };
    FGTowerController      *getTowerController()      {
        return &towerController;
    };
    FGApproachController   *getApproachController()   {
        return &approachController;
    };

    int getGroundFrequency(unsigned leg);
    int getTowerFrequency  (unsigned nr);

    /// get current ATIS sequence letter
    const std::string getAtisSequence();

    /// get the current ATIS sequence number, updating it if necessary
    int updateAtisSequence(int interval, bool forceUpdate);

    void setRwyUse(const FGRunwayPreference& ref);
};



#endif
