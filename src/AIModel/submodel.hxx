// submodel.hxx - models a releasable submodel.
// Written by Dave Culp, started Aug 2004
//
// This file is in the Public Domain and comes with no warranty.


#ifndef __SYSTEMS_SUBMODEL_HXX
#define __SYSTEMS_SUBMODEL_HXX 1

#ifndef __cplusplus
# error This library requires C++
#endif

#include <simgear/props/props.hxx>
#include <simgear/structure/subsystem_mgr.hxx>
#include <simgear/math/SGMath.hxx>

#include <Autopilot/inputvalue.hxx>

#include <vector>
#include <string>

class FGAIBase;
class FGAIManager;

class FGSubmodelMgr : public SGSubsystem, public SGPropertyChangeListener
{

public:

    typedef struct {
        SGPropertyNode_ptr trigger_node;
        SGPropertyNode_ptr prop;
        SGPropertyNode_ptr contents_node;
        SGPropertyNode_ptr submodel_node;
        SGPropertyNode_ptr speed_node;

        std::string        name;
        std::string        model;
        double             speed;
        bool               slaved;
        bool               repeat;
        double             delay;
        double             timer;
        int                count;
        bool               offsets_in_meter;
        FGXMLAutopilot::InputValue_ptr x_offset;
        FGXMLAutopilot::InputValue_ptr y_offset;
        FGXMLAutopilot::InputValue_ptr z_offset;
        FGXMLAutopilot::InputValue_ptr yaw_offset;
        FGXMLAutopilot::InputValue_ptr pitch_offset;
        double             drag_area;
        double             life;
        double             buoyancy;
        FGXMLAutopilot::InputValue_ptr azimuth_error;
        FGXMLAutopilot::InputValue_ptr elevation_error;
        FGXMLAutopilot::InputValue_ptr cd_randomness;
        FGXMLAutopilot::InputValue_ptr life_randomness;
        bool               wind;
        bool               first_time;
        double             cd;
        double             weight;
        double             mass;
        double             contents;
        bool               aero_stabilised;
        int                id;
        bool               no_roll;
        bool               serviceable;
        bool               random;
        bool               collision;
        bool               expiry;
        bool               impact;
        std::string        impact_report;
        double             fuse_range;
        std::string        submodel;
        int                sub_id;
        bool               force_stabilised;
        bool               ext_force;
        std::string        force_path;
    }   submodel;

    typedef struct {
        double     lat;
        double     lon;
        double     alt;
        double     roll;
        double     azimuth;
        double     elevation;
        double     speed;
        double     wind_from_east;
        double     wind_from_north;
        double     speed_down_fps;
        double     speed_east_fps;
        double     speed_north_fps;
        double     mass;
        int        id;
        bool       no_roll;
        int        parent_id;
    }   IC_struct;

    FGSubmodelMgr();
    ~FGSubmodelMgr();

    void load();
    void init();
    void postinit();
    void bind();
    void unbind();
    void update(double dt);
    void updatelat(double lat);

private:

    typedef std::vector <submodel*> submodel_vector_type;
    typedef submodel_vector_type::iterator submodel_vector_iterator;

    submodel_vector_type       submodels;
    submodel_vector_type       subsubmodels;
    submodel_vector_iterator   submodel_iterator, subsubmodel_iterator;

    int index;

    double ft_per_deg_longitude;
    double ft_per_deg_latitude;

    double x_offset, y_offset, z_offset;
    double pitch_offset, yaw_offset;

    double _parent_lat;
    double _parent_lon;
    double _parent_elev;
    double _parent_hdg;
    double _parent_pitch;
    double _parent_roll;
    double _parent_speed;

    double _x_offset;
    double _y_offset;
    double _z_offset;

    // Conversion factor
    static const double lbs_to_slugs;

    double contrail_altitude;

    bool _impact;
    bool _hit;
    bool _expiry;
    bool _found_sub;

    SGPropertyNode_ptr _serviceable_node;
    SGPropertyNode_ptr _user_lat_node;
    SGPropertyNode_ptr _user_lon_node;
    SGPropertyNode_ptr _user_heading_node;
    SGPropertyNode_ptr _user_alt_node;
    SGPropertyNode_ptr _user_pitch_node;
    SGPropertyNode_ptr _user_roll_node;
    SGPropertyNode_ptr _user_yaw_node;
    SGPropertyNode_ptr _user_alpha_node;
    SGPropertyNode_ptr _user_speed_node;
    SGPropertyNode_ptr _user_wind_from_east_node;
    SGPropertyNode_ptr _user_wind_from_north_node;
    SGPropertyNode_ptr _user_speed_down_fps_node;
    SGPropertyNode_ptr _user_speed_east_fps_node;
    SGPropertyNode_ptr _user_speed_north_fps_node;
    SGPropertyNode_ptr _contrail_altitude_node;
    SGPropertyNode_ptr _contrail_trigger;
    SGPropertyNode_ptr _count_node;
    SGPropertyNode_ptr _trigger_node;
    SGPropertyNode_ptr props;
    SGPropertyNode_ptr _model_added_node;
    SGPropertyNode_ptr _path_node;
    SGPropertyNode_ptr _selected_ac;

    IC_struct  IC;

    // Helper to retrieve the AI manager, if it currently exists
    FGAIManager* aiManager();

    void loadAI();
    void loadSubmodels();
    void setData(int id, const std::string& path, bool serviceable, const std::string& property_path, submodel_vector_type& models);
    void valueChanged (SGPropertyNode *);
    void transform(submodel *);
    void setParentNode(int parent_id);
    bool release(submodel *, double dt);

    int _count;

    SGGeod userpos;
    SGGeod offsetpos;
    SGVec3d getCartOffsetPos() const;
    void setOffsetPos();

};

#endif // __SYSTEMS_SUBMODEL_HXX
