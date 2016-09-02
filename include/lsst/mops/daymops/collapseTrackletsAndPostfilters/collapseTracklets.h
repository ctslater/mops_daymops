// -*- LSST-C++ -*-

/*
 * jmyers 7/29/08
 * 
 * collapse-tracklets is my specialized Hough transform code.  It is used for
 * quickly finding tracklets which describe similar linear motion and joining
 * them into longer tracklets.  Currently it is intended to be run from the
 * command-line, but the interface exported here should be fairly easy to port
 * into an LSST pipeline.
 *
 */



#ifndef LSST_COLLAPSE_TRACKLETS_H
#define LSST_COLLAPSE_TRACKLETS_H

#include <set>
#include <string>
#include <vector>

#include "lsst/mops/KDTree.h"
#include "lsst/mops/MopsDetection.h" 
#include "lsst/mops/Exceptions.h"
#include "lsst/mops/Tracklet.h"

namespace lsst {
    namespace mops {


        
        
        /* *pairs is modified - the Tracklets will have isCollapsed
         * set. collapsedPairs will actual output data.  I.e. if pairs
         * contains similar tracklets [1,2] and [2,3] they will be
         * marked as collapsed, and [1,2,3] will be added to the
         * collapsedPairs vector.*/
        void doCollapsingPopulateOutputVector(
            const std::vector<MopsDetection> * detections, 
            std::vector<Tracklet> &pairs,
            std::vector<double> tolerances, 
            std::vector<Tracklet> &collapsedPairs,
            bool useMinimumRMS, bool useBestFit, 
            bool useRMSFilt, double maxRMS, bool beVerbose);
            
    void parameterize(const std::vector<MopsDetection> *trackletDets,
                      std::vector<double> &motionVector,
                      double normalTime);
    }} // close lsst::mops

#endif
