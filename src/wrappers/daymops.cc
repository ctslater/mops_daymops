#include <pybind11/pybind11.h>

#include "lsst/mops/MopsDetection.h"

namespace py = pybind11;

PYBIND11_PLUGIN(daymops) {
    py::module m("daymops", "mops_daymops module");

    py::class_<lsst::mops::MopsDetection>(m, "MopsDetection")
        .def(py::init<>())
        .def(py::init<long int, double, double, double, double, double, int, long int, double, double>())
        .def("fromString", &lsst::mops::MopsDetection::fromString)
        .def("calculateTopoCorr", &lsst::mops::MopsDetection::calculateTopoCorr)
        .def_property("ID", &lsst::mops::MopsDetection::getID, 
                &lsst::mops::MopsDetection::setID)
        .def_property("ImageID", &lsst::mops::MopsDetection::getImageID, 
                &lsst::mops::MopsDetection::setImageID)
        .def_property("EpochMJD", &lsst::mops::MopsDetection::getEpochMJD, 
                &lsst::mops::MopsDetection::setEpochMJD)
        .def_property("RA", &lsst::mops::MopsDetection::getRA, 
                &lsst::mops::MopsDetection::setRA)
        .def_property("Dec", &lsst::mops::MopsDetection::getDec, 
                &lsst::mops::MopsDetection::setDec)
        .def_property("Mag", &lsst::mops::MopsDetection::getMag, 
                &lsst::mops::MopsDetection::setMag)
        .def_property("SNR", &lsst::mops::MopsDetection::getSNR, 
                &lsst::mops::MopsDetection::setSNR)
        .def_property("RaErr", &lsst::mops::MopsDetection::getRaErr, 
                &lsst::mops::MopsDetection::setRaErr)
        .def_property("DecErr", &lsst::mops::MopsDetection::getDecErr, 
                &lsst::mops::MopsDetection::setDecErr)
        .def_property_readonly("RaTopoCorr", &lsst::mops::MopsDetection::getRaTopoCorr); 

    return m.ptr();
}
