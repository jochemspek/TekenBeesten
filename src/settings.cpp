#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "settings.h"

void Settings::load(const std::string &filename)
{
    using boost::property_tree::ptree;
    
    read_json(filename, properties);
    
    graphics_full_screen = properties.get<bool>("graphics.full_screen");

    graphics_simulate_tracking = properties.get<bool>("graphics.simulate_tracking");
    
    graphics_num_boids = properties.get<int>("graphics.num_boids");
    graphics_num_attractors = properties.get<int>("graphics.num_attractors");
    
    graphics_window_width = properties.get<int>("graphics.window_width");
    graphics_window_height = properties.get<int>("graphics.window_height");
    
    tracking_history = properties.get<int>("tracking.history");
    tracking_threshold = properties.get<int>("tracking.threshold");
    tracking_blur = properties.get<int>("tracking.blur");
    
    tracking_min_blob_area = properties.get<int>("tracking.min_blob_area");
    tracking_max_blob_area = properties.get<int>("tracking.max_blob_area");
    tracking_distance = properties.get<float>("tracking.distance");
    tracking_inactive = properties.get<int>("tracking.inactive");
    tracking_near = properties.get<int>("tracking.near");
    tracking_far = properties.get<int>("tracking.far");
}
