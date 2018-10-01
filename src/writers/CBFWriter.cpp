//
// Created by mhilgart on 2/15/17.
//

#include "../common/Log.h"
#include "../reader/Image.h"
#include "../reader/SeriesHeader.h"
#include <cbf.h>
#include "CBFWriter.h"
#include "src/http/Counter.h"
#include <boost/scoped_ptr.hpp>
#include <src/common/Util.h>

using namespace std;

boost::asio::io_service *CBFWriter::io_service;
boost::thread_group *CBFWriter::threads;
boost::asio::io_service::work *CBFWriter::work;
const int CBFWriter::THREAD_MAX = 300;

boost::mutex CBFWriter::lock;
int CBFWriter::pendingWrites = 0;

#define CBF_CHECK(ERR_CODE,ERR_STRING) { \
    int ec = (int)ERR_CODE;  \
    if (0!=ec) { \
        string errName = ""; \
        if (ec&CBF_FORMAT)    errName+="CBF_FORMAT ";    \
        if (ec&CBF_ALLOC)     errName+="CBF_ALLOC ";     \
        if (ec&CBF_ARGUMENT)  errName+="CBF_ARGUMENT ";  \
        if (ec&CBF_ASCII)     errName+="CBF_ASCII ";     \
        if (ec&CBF_BINARY)    errName+="CBF_BINARY ";    \
        if (ec&CBF_BITCOUNT)  errName+="CBF_BITCOUNT ";  \
        if (ec&CBF_ENDOFDATA) errName+="CBF_ENDOFDATA "; \
        if (ec&CBF_FILECLOSE) errName+="CBF_FILECLOSE "; \
        if (ec&CBF_FILEOPEN)  errName+="CBF_FILEOPEN ";  \
        if (ec&CBF_FILEREAD)  errName+="CBF_FILEREAD ";  \
        if (ec&CBF_FILESEEK)  errName+="CBF_FILESEEK ";  \
        if (ec&CBF_FILETELL)  errName+="CBF_FILETELL ";  \
        if (ec&CBF_FILEWRITE) errName+="CBF_FILEWRITE "; \
        if (ec&CBF_IDENTICAL) errName+="CBF_IDENTICAL "; \
        if (ec&CBF_NOTFOUND)  errName+="CBF_NOTFOUND ";  \
        if (ec&CBF_OVERFLOW)  errName+="CBF_OVERFLOW ";  \
        if (ec&CBF_UNDEFINED) errName+="CBF_UNDEFINED "; \
        if (ec&CBF_NOTIMPLEMENTED) errName+="CBF_NOTIMPLEMENTED "; \
        BOOST_LOG_TRIVIAL(error) << "CBF error " << ERR_STRING << ", code: " << errName;   \
        return; } }

void CBFWriter::writeCBF(std::string path, SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage)
{
    pImage->decompress();
    if (pImage->uncompressedData==NULL)
    {
        BOOST_LOG_TRIVIAL(debug) << "Skipping " << path << " due to failed decompress";
    }
    else
    {
        pImage->applyPixelMask(header->pixelMask, header->pixelMaskSize);

        // make sure directory exists
        string dir = path.substr(0, path.find_last_of('/'));
        ::mkdir(dir.c_str(), ACCESSPERMS);

        FILE *f = fopen(path.c_str(), "wb");
        bool areElementsSigned = true;

        // start a new cbf
        cbf_handle cbf;
        CBF_CHECK(cbf_make_handle(&cbf),"cbf_make_handle");
        CBF_CHECK(cbf_new_datablock(cbf, "image_1"),"cbf_new_datablock");

        // mini-cbf header
        CBF_CHECK(cbf_new_category(cbf, "array_data"),"cbf_new_category");
        CBF_CHECK(cbf_new_column(cbf, "header_convention"),"cbf_new_column");
        CBF_CHECK(cbf_set_value(cbf, "SLS_1.0"),"cbf_set_value");
        CBF_CHECK(cbf_new_column(cbf, "header_contents"),"cbf_new_column");
        CBF_CHECK(cbf_set_value(cbf, generateHeader(header, override, pImage).c_str()),"cbf_set_value");

        // write the image
        CBF_CHECK(cbf_new_category(cbf, "array_data"),"cbf_new_category");
        CBF_CHECK(cbf_new_column(cbf, "data"),"cbf_new_column");
        CBF_CHECK(cbf_set_integerarray_wdims_fs
        (
                cbf,
                CBF_BYTE_OFFSET,
                1,
                pImage->uncompressedData,
                pImage->bitDepth / 8,
                areElementsSigned ? 1 : 0,
                pImage->xPixels * pImage->yPixels,
                "little_endian",
                pImage->xPixels,
                pImage->yPixels,
                0,
                0
        ),"cbf_set_integerarray_wdims_fs");
        CBF_CHECK(cbf_write_file
                (
                        cbf,
                        f,
                        1,
                        CBF,
                        MSG_DIGEST | MIME_HEADERS | PAD_4K,
                        0
                ),"cbf_write_file");
        CBF_CHECK(cbf_free_handle(cbf),"cbf_free_handle");

        Counter::setImageWritten(pImage->frameIndex);

        BOOST_LOG_TRIVIAL(debug) << "Wrote " << path;
    }

    {
        boost::mutex::scoped_lock(lock);
        pendingWrites--;
    }
}

string CBFWriter::generateHeader(SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage)
{
    char header_format[] =
        "\n"
        "# Detector: %s, S/N %s\n"
        "# Pixel_size %de-6 m x %de-6 m\n"
        "# Silicon sensor, thickness %.6f m\n"
        "# Exposure_time %f s\n"
        "# Exposure_period %f s\n"
        "# Count_cutoff %d counts\n"
        "# Wavelength %f A\n"
        "# Detector_distance %f m\n"
        "# Beam_xy (%d, %d) pixels\n"
        "# Start_angle %f deg.\n"
        "# Angle_increment %f deg.\n"
        "# Detector_2theta %f deg.\n"
        "# Threshold_setting %d eV\n";

    char header_content[4096] = {};
    snprintf
    (
        header_content,4096,header_format,
        header->description.c_str(),
        header->detector_number.c_str(),
        (int)(header->x_pixel_size_m * 1E6),
        (int)(header->x_pixel_size_m * 1E6),
        header->sensorThickness_m,
        header->count_time_sec,
        header->frame_time_sec,
        header->countrate_cutoff,
        override.getWavelength_A(header),
        header->detector_distance_m,
        header->beam_center_x_px,
        header->beam_center_y_px,
        override.getOmegaStart(header) + override.getOmegaDelta(header) * pImage->frameIndex,
        header->omega_increment_deg,
        header->two_theta_deg,
        (int)(header->threshold_energy_eV + 0.5)
    );

    return string(header_content);
}

void CBFWriter::init()
{
    io_service = new boost::asio::io_service();
    threads = new boost::thread_group();
    work = new boost::asio::io_service::work(*io_service);

    for(size_t t = 0; t < THREAD_MAX; t++)
    {
        threads->create_thread(boost::bind(&boost::asio::io_service::run, io_service));
    }

}

void CBFWriter::submitTask(std::string path, SeriesHeader *header, HeaderOverride override, boost::shared_ptr<Image> pImage)
{
    {
        boost::mutex::scoped_lock(lock);
        pendingWrites++;
    }
    io_service->post(boost::bind(writeCBF, path, header, override, pImage));
}
