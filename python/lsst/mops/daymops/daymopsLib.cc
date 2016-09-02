#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "lsst/mops/MopsDetection.h"
#include "lsst/mops/Tracklet.h"
#include "lsst/mops/Track.h"
#include "lsst/mops/daymops/findTracklets/findTracklets.h"
#include "lsst/mops/daymops/linkTracklets/linkTracklets.h"
#include "lsst/mops/daymops/collapseTrackletsAndPostfilters/collapseTracklets.h"

namespace py = pybind11;

using namespace lsst::mops;

PYBIND11_MAKE_OPAQUE(std::vector<Tracklet>);

PYBIND11_PLUGIN(_daymopsLib) {
    py::module m("_daymopsLib", "mops_daymops C++ wrapper module");


    py::class_<MopsDetection>(m, "MopsDetection")
        .def(py::init<>())
        .def(py::init<long int, double, double, double, double, double, int, long int, double, double>(),
                py::arg("ID"), py::arg("epochMJD"), py::arg("RA"), py::arg("Dec"),
                py::arg("RaErr") = 0.0, py::arg("DecErr") = 0.0, py::arg("ssmId") = -1,
                py::arg("obsHistId") = -1, py::arg("snr") = -1, py::arg("mag") = -1)
        .def("fromString", &MopsDetection::fromString)
        .def("toString", [](MopsDetection &d) { return "<MopsDetection " + std::to_string((&d)->getID()) + " >"; })
        .def("calculateTopoCorr", &MopsDetection::calculateTopoCorr)
        .def_property("ID", &MopsDetection::getID,
                &MopsDetection::setID)
        .def_property("ImageID", &MopsDetection::getImageID,
                &MopsDetection::setImageID)
        .def_property("EpochMJD", &MopsDetection::getEpochMJD,
                &MopsDetection::setEpochMJD)
        .def_property("RA", &MopsDetection::getRA,
                &MopsDetection::setRA)
        .def_property("Dec", &MopsDetection::getDec,
                &MopsDetection::setDec)
        .def_property("Mag", &MopsDetection::getMag,
                &MopsDetection::setMag)
        .def_property("SNR", &MopsDetection::getSNR,
                &MopsDetection::setSNR)
        .def_property("RaErr", &MopsDetection::getRaErr,
                &MopsDetection::setRaErr)
        .def_property("DecErr", &MopsDetection::getDecErr,
                &MopsDetection::setDecErr)
        .def_property_readonly("RaTopoCorr", &MopsDetection::getRaTopoCorr);

    // findTrackletsConfig
    py::class_<findTrackletsConfig>(m, "findTrackletsConfig")
        .def(py::init<>())
        .def_readwrite("maxDt", &findTrackletsConfig::maxDt)
        .def_readwrite("minDt", &findTrackletsConfig::minDt)
        .def_readwrite("maxV", &findTrackletsConfig::maxV)
        .def_readwrite("minV", &findTrackletsConfig::minV)
        .def_readwrite("outputFile", &findTrackletsConfig::outputFile)
        .def_readwrite("outputBufferSize", &findTrackletsConfig::outputBufferSize);

    // findTracklets
    m.def("findTracklets", &findTracklets);

    py::class_<Tracklet>(m, "Tracklet")
        .def(py::init<>())
        .def("__init__", [](Tracklet &t, py::list startIndices) {
                new (&t) Tracklet();
                for (auto value : startIndices) {
                    (&t)->indices.insert(value.cast<unsigned int>());
                    }
        })
        // Since we can't edit indices from python, exposing this as a function
        // to discourage anyone from trying.
        //.def_readonly("indices", &Tracklet::indices)
        .def("indices", [](Tracklet &t) {
                return py::cast((&t)->indices);
                })
        .def_readwrite("isCollapsed", &Tracklet::isCollapsed);

    // linkTracklets
    py::class_<linkTrackletsConfig>(m, "linkTrackletsConfig")
        .def(py::init<>())
        .def_readwrite("maxRAAccel", &linkTrackletsConfig::maxRAAccel)
        .def_readwrite("maxDecAccel", &linkTrackletsConfig::maxDecAccel)
        .def_readwrite("restrictTrackStartTimes",
                &linkTrackletsConfig::restrictTrackStartTimes)
        .def_readwrite("latestFirstEndpointTime",
                &linkTrackletsConfig::latestFirstEndpointTime)
        .def_readwrite("restrictTrackEndTimes",
                &linkTrackletsConfig::restrictTrackEndTimes)
        .def_readwrite("earliestLastEndpointTime",
                &linkTrackletsConfig::earliestLastEndpointTime)
        .def_readwrite("detectionLocationErrorThresh",
                &linkTrackletsConfig::detectionLocationErrorThresh)
        .def_readwrite("trackAdditionThreshold",
                &linkTrackletsConfig::trackAdditionThreshold)
        .def_readwrite("trackMaxRms",
                &linkTrackletsConfig::trackMaxRms)
        .def_readwrite("minSupportToEndpointTimeSeparation",
                &linkTrackletsConfig::minSupportToEndpointTimeSeparation)
        .def_readwrite("minEndpointTimeSeparation",
                &linkTrackletsConfig::minEndpointTimeSeparation)
        .def_readwrite("minUniqueNights",
                &linkTrackletsConfig::minUniqueNights)
        .def_readwrite("minDetectionsPerTrack",
                &linkTrackletsConfig::minDetectionsPerTrack)
        .def_readwrite("leafSize",
                &linkTrackletsConfig::leafSize)
        .def_readwrite("obsLat",
                &linkTrackletsConfig::obsLat)
        .def_readwrite("obsLong",
                &linkTrackletsConfig::obsLong)
        .def_readwrite("defaultAstromErr",
                &linkTrackletsConfig::defaultAstromErr)
        .def_readwrite("trackMinProbChisq",
                &linkTrackletsConfig::trackMinProbChisq)
        .def_readwrite("skyCenterRa",
                &linkTrackletsConfig::skyCenterRa)
        .def_readwrite("myVerbosity",
                &linkTrackletsConfig::myVerbosity);

    py::class_<linkTrackletsVerbositySettings>(m, "linkTrackletsVerbositySettings")
        .def(py::init<>())
        .def_readwrite("printStatus", &linkTrackletsVerbositySettings::printStatus)
        .def_readwrite("printVisitCounts", &linkTrackletsVerbositySettings::printStatus)
        .def_readwrite("printTimesByCategory", &linkTrackletsVerbositySettings::printStatus)
        .def_readwrite("printBoundsInfo", &linkTrackletsVerbositySettings::printStatus);

    m.def("linkTracklets", &linkTracklets,
            py::arg("allDetections"), py::arg("queryTracklets"), py::arg("searchConfig"));

    m.def("modifyWithAcceleration", &modifyWithAcceleration,
        py::arg("position"), py::arg("velocity"), py::arg("acceleration"), py::arg("time"));

    m.def("findLinkableObjects", &findLinkableObjects,
            py::arg("allDetections"), py::arg("allTracklets"), py::arg("searchConfig"),
            py::arg("findableObjectNames"));

    m.def("calculateTopoCorr", &calculateTopoCorr,
            py::arg("allDetections"), py::arg("searchConfig"));

    // collapseTracklets
    m.def("doCollapsingPopulateOutputVector",  &doCollapsingPopulateOutputVector,
            py::arg("detections"), py::arg("tracklets"), py::arg("tolerances"),
            py::arg("collapsedPairs"), py::arg("useMinimumRMS"),
            py::arg("useBestFit"), py::arg("useRMSFilt"), py::arg("maxRMS"),
            py::arg("beVerbose"));


    // Defining a typedef to avoid very lengthy lines in the wrapping code.
    typedef std::vector<Tracklet> PyTrackletSet;

    py::class_<std::vector<Tracklet>>(m, "TrackletSet")
        .def(py::init<>())
        .def("__init__", [](PyTrackletSet &v, py::list l) {
                new (&v) PyTrackletSet();
                for (auto item : l)
                    (&v)->push_back(item.cast<Tracklet>());
        })
        .def("append", (void (PyTrackletSet::*)(const Tracklet &))
                        &PyTrackletSet::push_back)
        //.def("__add__", &std::vector<Tracklet>::push_back)
        //.def("__iadd__", &std::vector<Tracklet>::push_back)
        .def("__getitem__", [](const PyTrackletSet &v, size_t i) {
            if (i >= v.size())
                throw py::index_error();
            return v[i];
        })
        .def("__len__", [](const PyTrackletSet &v) { return v.size(); })
        .def("__iter__", [](PyTrackletSet &v) {
           return py::make_iterator(v.begin(), v.end());
        }, py::keep_alive<0, 1>());

    m.def("paramterizeTracklet", [](py::list trackletDets,
                double normalTime) {
            std::vector<double> motionVector;
            const std::vector<MopsDetection> &tmp = trackletDets.cast<std::vector<lsst::mops::MopsDetection>>();
            parameterize(&tmp,
                    motionVector, normalTime);
            return py::cast(motionVector);
            });

    py::class_<Track>(m , "Track")
        .def("detectionIndices", &Track::getComponentDetectionIndices)
        .def("diaIds", &Track::getComponentDetectionDiaIds)
        .def("trackletIndices", &Track::getComponentDetectionDiaIds)
        .def("trackletIndices", [](Track &t) {
                return py::cast((&t)->componentTrackletIndices);
                });

    // This is a copy of TrackletSet for Tracks.
    // There must be some way to combine this boilerplate for list-like
    // containers.
    // Also TrackSet is a set, but Tracklets are in a vector. It would be good
    // to make these more parallel and pick one or the other.
    py::class_<TrackSet>(m, "TrackSet")
        .def(py::init<>())
        .def("toString", [](TrackSet &d) { return "<TrackSet>"; })
        .def("__len__", &TrackSet::size)
        .def("__iter__", [](TrackSet &v) {
           return py::make_iterator(v.componentTracks.begin(), v.componentTracks.end());
        }, py::keep_alive<0, 1>());


    return m.ptr();
}
