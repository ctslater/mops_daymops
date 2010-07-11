// -*- LSST-C++ -*-

/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
 


/* jmyers 8/18/08
 *
 *This code is for getting the best-fit line for a series of observations
 * (i.e. a tracklet) and determining the RMS of the error for the detections.
 *
 * This also includes code for filtering out groups of tracklets which do not
 * fit the criteria.
 */


#ifndef LSST_RMSLINEFIT_H
#define LSST_RMSLINEFIT_H

#include <vector>
#include <map>

#include "lsst/mops/Tracklet.h"
#include "lsst/mops/MopsDetection.h"
#include "lsst/mops/Exceptions.h"

namespace lsst {
namespace mops {


    /*
     * return the root mean squared distance of the tracklet from the line.
     * (i.e. the sqrt of the sum of the squared distances of each detection from
     * the best-fit line, all divided by the size of the tracklet).
     *
     * if perDetSqDist != NULL (the default value), then we ASSUME that
     * perDetSqDist is a pointer to an allocated vector, and we will populate it
     * s.t. mean Sq. distance (trackletDets[i],line) = perDetSqDist[i] 
     */
    double rmsForTracklet(Tracklet t, const std::vector<MopsDetection> *detections, 
                          std::vector<double>*perDetSqDist=NULL);
    


    
    /*
      given an array of detections, solve for RA and Dec in terms of MJD on the
      assumption that both are linear.  Output vectors will look like this when done:
      
      if RA = MJD * m + b
      RASlopeAndOffsetOut = [m, b] 
      
      (and similarly for Dec.)
      
      if timeOffset is specified as non-zero, this value will be substracted from the 
      MJDs of each detection.
    */
    void leastSquaresSolveForRADecLinear(const std::vector <MopsDetection> *trackletDets,
                                         std::vector<double> &RASlopeAndOffsetOut,
                                         std::vector<double> &DecSlopeAndOffsetOut, 
                                         double timeOffset=0.0);
    
/*
 * given a vector of Tracklets and the corresponding vector of MopsDetections,
 * add to output only those tracklets for which rms < maxRMSm * av. magnitude + maxRMSm
 */
    
    void filterByLineFitAddToOutputVector(const std::vector<Tracklet> *tracklets, 
                                          const std::vector<MopsDetection> * allDets,
                                          double maxRMS,
                                          std::vector<Tracklet> &output);
    
    

    class TrackletPurifier {
    public:
        Tracklet purifyTracklet(const Tracklet *t, const std::vector<MopsDetection> *allDets, 
                                double maxRMS);

        void purifyTracklets(const std::vector<Tracklet> *trackletsVector,
                             const std::vector<MopsDetection> *detsVector,
                             double maxRMS, unsigned int minObs,
                             std::vector<Tracklet> &output);
    };


}} // close lsst::mops

#endif
