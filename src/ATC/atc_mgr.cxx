/******************************************************************************
 * atc_mgr.cxx
 * Written by Durk Talsma, started August 1, 2010.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 **************************************************************************/


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <iostream>

#include <Airports/dynamics.hxx>
#include <Airports/airportdynamicsmanager.hxx>
#include <Airports/airport.hxx>
#include <Scenery/scenery.hxx>
#include <Main/globals.hxx>
#include <Main/fg_props.hxx>
#include <AIModel/AIAircraft.hxx>
#include <AIModel/AIManager.hxx>
#include <Traffic/Schedule.hxx>
#include <Traffic/SchedFlight.hxx>
#include <AIModel/AIFlightPlan.hxx>

#include "atc_mgr.hxx"


using std::string;

FGATCManager::FGATCManager() :
    controller(NULL),
    prevController(NULL),
    networkVisible(false),
    initSucceeded(false)
{
}

FGATCManager::~FGATCManager() {
}

void FGATCManager::postinit()
{
    int leg = 0;

    trans_num = globals->get_props()->getNode("/sim/atc/transmission-num", true);

    // find a reasonable controller for our user's aircraft..
    // Let's start by working out the following three scenarios: 
    // Starting on ground at a parking position
    // Starting on ground at the runway.
    // Starting in the Air
    bool onGround  = fgGetBool("/sim/presets/onground");
    string runway  = fgGetString("/sim/atc/runway");
    string airport = fgGetString("/sim/presets/airport-id");
    string parking = fgGetString("/sim/presets/parkpos");
    
    FGAIManager* aiManager = globals->get_subsystem<FGAIManager>();
    FGAIAircraft* userAircraft = aiManager->getUserAircraft();

    double aircraftRadius = 40; // note that this is currently hardcoded to a one-size-fits all JumboJet value. Should change later;

    // NEXT UP: Create a traffic Schedule and fill that with appropriate information. This we can use to flight planning.
    // Note that these are currently only defaults. 
    FGAISchedule *trafficRef = new FGAISchedule;
    trafficRef->setFlightType("gate");

    FGScheduledFlight *flight =  new FGScheduledFlight;
    flight->setDepartureAirport(airport);
    flight->setArrivalAirport(airport);
    flight->initializeAirports();
    flight->setFlightRules("IFR");
    flight->setCallSign(userAircraft->getCallSign());
    
    trafficRef->assign(flight);
    std::unique_ptr<FGAIFlightPlan> fp ;
    userAircraft->setTrafficRef(trafficRef);
    
    string flightPlanName = airport + "-" + airport + ".xml";
    //double cruiseAlt = 100; // Doesn't really matter right now.
    //double courseToDest = 180; // Just use something neutral; this value might affect the runway that is used though...
    //time_t deptime = 0;        // just make sure how flightplan processing is affected by this...


    FGAirportDynamicsRef dcs(flightgear::AirportDynamicsManager::find(airport));
    if (dcs && onGround) {// && !runway.empty()) {

        ParkingAssignment pk(dcs->getParkingByName(parking));
      
        if (pk.isValid()) {
            dcs->setParkingAvailable(pk.parking(), false);
            fp.reset(new FGAIFlightPlan);
            controller = dcs->getStartupController();
            int stationFreq = dcs->getGroundFrequency(1);
            if (stationFreq > 0)
            {
                //cerr << "Setting radio frequency to : " << stationFreq << endl;
                fgSetDouble("/instrumentation/comm[0]/frequencies/selected-mhz", ((double) stationFreq / 100.0));
            }
            leg = 1;
            //double, lat, lon, head; // Unused variables;
            //int getId = apt->getDynamics()->getParking(gateId, &lat, &lon, &head);
            aircraftRadius = pk.parking()->getRadius();
            string fltType = pk.parking()->getType(); // gate / ramp, ga, etc etc.
            string aircraftType; // Unused.
            string airline;      // Currently used for gate selection, but a fallback mechanism will apply when not specified.
            fp->setGate(pk);
            if (!(fp->createPushBack(userAircraft,
                                     false,
                                     dcs->parent(),
                                     aircraftRadius,
                                     fltType,
                                     aircraftType,
                                     airline))) {
                controller = 0;
                return;
            }

            
            
        } else if (!runway.empty()) {
            // on a runway

            controller = dcs->getTowerController();
            int stationFreq = dcs->getTowerFrequency(2);
            if (stationFreq > 0)
            {
                //cerr << "Setting radio frequency to in airfrequency: " << stationFreq << endl;
                fgSetDouble("/instrumentation/comm[0]/frequencies/selected-mhz", ((double) stationFreq / 100.0));
            }
            fp.reset(new FGAIFlightPlan);
            leg = 3;
            string fltType = "ga";
            fp->setRunway(runway);
            fp->createTakeOff(userAircraft, false, dcs->parent(), 0, fltType);
            userAircraft->setTakeOffStatus(2);
        } else {
            // We're on the ground somewhere. Handle this case later.
        }
        
        if (fp) {
            fp->getLastWaypoint()->setName( fp->getLastWaypoint()->getName() + string("legend"));
        }
     } else {
        controller = 0;
     }

    // Create an initial flightplan and assign it to the ai_ac. We won't use this flightplan, but it is necessary to
    // keep the ATC code happy. 
    if (fp) {
        fp->restart();
        fp->setLeg(leg);
        userAircraft->FGAIBase::setFlightPlan(std::move(fp));
    }
    if (controller) {
        FGAIFlightPlan* plan = userAircraft->GetFlightPlan();
        controller->announcePosition(userAircraft->getID(), plan, plan->getCurrentWaypoint()->getRouteIndex(),
                                     userAircraft->_getLatitude(),
                                     userAircraft->_getLongitude(),
                                     userAircraft->_getHeading(),
                                     userAircraft->getSpeed(),
                                     userAircraft->getAltitude(),
                                     aircraftRadius, leg, userAircraft);
    }
    initSucceeded = true;
}

void FGATCManager::shutdown()
{
    activeStations.clear();
}

void FGATCManager::addController(FGATCController *controller) {
    activeStations.push_back(controller);
}

void FGATCManager::removeController(FGATCController *controller)
{
    AtcVecIterator it;
    it = std::find(activeStations.begin(), activeStations.end(), controller);
    if (it != activeStations.end()) {
        activeStations.erase(it);
    }
}

void FGATCManager::update ( double time ) {
    //cerr << "ATC update code is running at time: " << time << endl;
    // Test code: let my virtual co-pilot handle ATC:
   
    FGAIManager* aiManager = globals->get_subsystem<FGAIManager>();
    FGAIAircraft* ai_ac = aiManager->getUserAircraft();
    FGAIFlightPlan *fp = ai_ac->GetFlightPlan();
        
    /* test code : find out how the routing develops */
    if (fp) {
        int size = fp->getNrOfWayPoints();
        //cerr << "Setting pos" << pos << " ";
        //cerr << "setting intentions " ;
        // This indicates that we have run out of waypoints: Im future versions, the
        // user should be able to select a new route, but for now just shut down the
        // system. 
        if (size < 3) {
            //cerr << "Shutting down the atc_mgr" << endl;
            return;
        }
#if 0
        // Test code: Print how far we're progressing along the taxi route. 
        //std::cerr << "Size of waypoint cue " << size << " ";
        for (int i = 0; i < size; i++) {
            int val = fp->getRouteIndex(i);
            //std::cerr << fp->getWayPoint(i)->getName() << " ";
            //if ((val) && (val != pos)) {
            //    intentions.push_back(val);
            //    std::cerr << "[done ] " << std::endl;
            //}
        }
        //std::cerr << "[done ] " << std::endl;
#endif
    }
    if (fp) {
        //cerr << "Currently at leg : " << fp->getLeg() << endl;
    }
    
    controller = ai_ac->getATCController();
    FGATCDialogNew::instance()->update(time);
    if (controller) {
       //cerr << "name of previous waypoint : " << fp->getPreviousWaypoint()->getName() << endl;

        //cerr << "Running FGATCManager::update()" << endl;
        //cerr << "Currently under control of " << controller->getName() << endl;
        controller->updateAircraftInformation(ai_ac->getID(),
                                              ai_ac->_getLatitude(),
                                              ai_ac->_getLongitude(),
                                              ai_ac->_getHeading(),
                                              ai_ac->getSpeed(),
                                              ai_ac->getAltitude(), time);
        //string airport = fgGetString("/sim/presets/airport-id");
        //FGAirport *apt = FGAirport::findByIdent(airport); 
        // AT this stage we should update the flightplan, so that waypoint incrementing is conducted as well as leg loading. 
        int n = trans_num->getIntValue();
        if (n == 1) {
            //cerr << "Toggling ground network visibility " << networkVisible << endl;
            networkVisible = !networkVisible;
            trans_num->setIntValue(-1);
        }
        if ((controller != prevController) && (prevController)) {
            prevController->render(false);
        }
        controller->render(networkVisible);

        //cerr << "Adding groundnetWork to the scenegraph::update" << endl;
        prevController = controller;
   }
   for (AtcVecIterator atc = activeStations.begin(); atc != activeStations.end(); atc++) {
       (*atc)->update(time);
   }
}
