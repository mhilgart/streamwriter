// Adapted from eiger2cbf:
// EIGER HDF5 to CBF converter
//      Written by Takanori Nakane
// BSD license.

#include <src/main/Config.h>
#include <src/common/Log.h>
#include "SeriesHeader.h"
#include <iostream>
#include <src/common/HttpGet.h>
#include <src/common/Util.h>

using namespace std;

bool SeriesHeader::isValid()
{
    return NULL==zmq_msg;
}

SeriesHeader::SeriesHeader(ZMQMultipartMessage *msg)
{
    zmq_msg = msg;
    if (NULL==msg) return;

    Json::Reader reader;
    string s = Util::getZMQString(msg->getPart(2));
    reader.parse(s, headerRoot, false);

    imageCount           = headerRoot["nimages"].asInt();
    description          = headerRoot["description"].asString();
    detector_number      = headerRoot["detector_number"].asString();
    software_version     = headerRoot["software_version"].asString();
    // bit_depth_image is stored in the image header
    // countrate cutoff or saturation value not found, is it written anywhere?
    countrate_cutoff     = getCountCutoff();
    sensorThickness_m    = headerRoot["sensor_thickness"].asDouble();
    x_pixels_in_detector = headerRoot["x_pixels_in_detector"].asInt();
    y_pixels_in_detector = headerRoot["y_pixels_in_detector"].asInt();
    beam_center_x_px     = headerRoot["beam_center_x"].asInt();
    beam_center_y_px     = headerRoot["beam_center_y"].asInt();
    count_time_sec       = headerRoot["count_time"].asDouble();
    frame_time_sec       = headerRoot["frame_time"].asDouble();
    x_pixel_size_m       = headerRoot["x_pixel_size"].asDouble();
    detector_distance_m  = headerRoot["detector_distance"].asDouble();
    wavelength_A         = headerRoot["wavelength"].asDouble();
    omega_start_deg      = headerRoot["omega_start"].asDouble();
    omega_increment_deg  = headerRoot["omega_increment"].asDouble();
    two_theta_deg        = headerRoot["two_theta_start"].asDouble();
    threshold_energy_eV  = headerRoot["threshold_energy"].asDouble();
    // omega angles are not reported for all frames, they must be computed
    pixelMask            = msg->getPart(6)->data();
    pixelMaskSize        = msg->getPart(6)->size();
}

SeriesHeader::~SeriesHeader()
{
    delete zmq_msg;
}

int SeriesHeader::getCountCutoff()
{
    Json::Reader reader;
    Json::Value responseRoot;
    string response = HttpGet::get(Config::values["eiger_ip"],"/detector/api/1.6.0/config/countrate_correction_count_cutoff");
    BOOST_LOG_TRIVIAL(debug) << "Got cutoff response: ";
    BOOST_LOG_TRIVIAL(debug) << "[" << response << "]";

    reader.parse(response.c_str(), responseRoot, false);
    int cutoff = responseRoot["value"].asInt();
    return cutoff;
}
