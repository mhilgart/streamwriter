#ifndef STREAMWRITER_SERIESHEADER_H
#define STREAMWRITER_SERIESHEADER_H

#include "ZMQMultipartMessage.h"
#include <json.h>

class SeriesHeader
{
public:
    SeriesHeader(ZMQMultipartMessage *msg);
    ~SeriesHeader();

    bool isValid();

    int imageCount;
    string description;
    string detector_number;
    string software_version;
    int countrate_cutoff;
    double sensorThickness_m;
    int x_pixels_in_detector;
    int y_pixels_in_detector;
    int beam_center_x_px;
    int beam_center_y_px;
    double count_time_sec;
    double frame_time_sec;
    double x_pixel_size_m;
    double detector_distance_m;
    double wavelength_A;
    double omega_start_deg;
    double omega_increment_deg;
    double two_theta_deg;
    double threshold_energy_eV;
    void *pixelMask;
    size_t pixelMaskSize;

private:
    ZMQMultipartMessage *zmq_msg;
    Json::Value headerRoot;

    int getCountCutoff();
};

struct HeaderOverride
{
    double omegaStart, omegaDelta, wavelength_A;
    double getOmegaStart(SeriesHeader *header) { return !std::isnan(omegaStart)?omegaStart:header->omega_start_deg; }
    double getOmegaDelta(SeriesHeader *header) { return !std::isnan(omegaDelta)?omegaDelta:header->omega_increment_deg; }
    double getWavelength_A(SeriesHeader *header) { return !std::isnan(wavelength_A)?wavelength_A:header->wavelength_A; }
};

#endif //STREAMWRITER_SERIESHEADER_H
