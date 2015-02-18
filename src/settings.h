#ifndef __settings_h__
#define __settings_h__

#include <boost/property_tree/ptree.hpp>

struct Settings {
    boost::property_tree::ptree properties;
    void load(const std::string &filename);
    
    bool graphics_full_screen;

    bool graphics_simulate_tracking;
    bool graphics_draw_attractors;
    int graphics_num_boids;
    int graphics_num_attractors;
    
    int graphics_window_width;
    int graphics_window_height;
    
    int tracking_flip_horizontal;
    int tracking_flip_vertical;
    int tracking_history;
    int tracking_threshold;
    int tracking_blur;
    
    int tracking_min_blob_area;
    int tracking_max_blob_area;
    float tracking_distance;
    int tracking_inactive;
    int tracking_near;
    int tracking_far;
    float tracking_scale_x;
    float tracking_scale_y;
};
#endif // __settings_h__
