#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "lsst/mops/MopsDetection.h"
#include "lsst/mops/Tracklet.h"
#include "lsst/mops/daymops/findTracklets/findTracklets.h"

namespace py = pybind11;


PYBIND11_PLUGIN(daymops) {
    py::module m("daymops", "mops_daymops module");

    py::class_<lsst::mops::MopsDetection>(m, "MopsDetection")
        .def(py::init<>())
        .def(py::init<long int, double, double, double, double, double, int, long int, double, double>(),
                py::arg("ID"), py::arg("epochMJD"), py::arg("RA"), py::arg("Dec"),
                py::arg("RaErr") = 0.0, py::arg("DecErr") = 0.0, py::arg("ssmId") = -1,
                py::arg("obsHistId") = -1, py::arg("snr") = -1, py::arg("mag") = -1) 
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

    // findTrackletsConfig
    py::class_<lsst::mops::findTrackletsConfig>(m, "findTrackletsConfig")
        .def(py::init<>())
        .def_readwrite("maxDt", &lsst::mops::findTrackletsConfig::maxDt)
        .def_readwrite("minDt", &lsst::mops::findTrackletsConfig::minDt)
        .def_readwrite("maxV", &lsst::mops::findTrackletsConfig::maxV)
        .def_readwrite("minV", &lsst::mops::findTrackletsConfig::minV)
        .def_readwrite("outputFile", &lsst::mops::findTrackletsConfig::outputFile)
        .def_readwrite("outputBufferSize", &lsst::mops::findTrackletsConfig::outputBufferSize);

    // findTracklets
    m.def("findTracklets", &lsst::mops::findTracklets);

    py::class_<lsst::mops::Tracklet>(m, "Tracklet")
        .def(py::init<>())
        .def(py::init<std::set<unsigned int>>(), py::arg("startIndices"))
        .def_readwrite("indices", &lsst::mops::Tracklet::indices)
        .def_readwrite("isCollapsed", &lsst::mops::Tracklet::isCollapsed);


    return m.ptr();
}
