#ifndef __settings_h__
#define __settings_h__

#include <boost/property_tree/ptree.hpp>

struct Settings {
    boost::property_tree::ptree properties;
    void load(const std::string &filename);
    
    bool graphics_simulate_tracking;
    int graphics_num_boids;
    int graphics_num_attractors;
    
    int graphics_window_width;
    int graphics_window_height;
    
    int tracking_history;
    int tracking_threshold;
    int tracking_blur;
    
    int tracking_min_blob_area;
    int tracking_max_blob_area;
    float tracking_distance;
    int tracking_inactive;
    int tracking_near;
    int tracking_far;
};
#endif // __settings_h__