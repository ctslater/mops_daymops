// -*- LSST-C++ -*-
/* jonathan myers */


#include <stdlib.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <getopt.h>
#include <map>


#include "lsst/mops/daymops/linkTracklets/linkTracklets.h"
#include "lsst/mops/fileUtils.h"


namespace lsst {
    namespace mops {

/*
  this is just a tool for looking through a pair of dets/tracklets
  files and finding objects which meet requirements for being found by
  linkTracklets.
*/


typedef double MJD;

MJD minMJD(std::set<MJD> mjds)
{
    MJD least;
    bool foundOne = false;
    std::set<MJD>::const_iterator mIter;
    for (mIter = mjds.begin(); mIter != mjds.end(); mIter++) {
        if ((!foundOne) || (*mIter < least)) {
            least = *mIter;
            foundOne = true;
        }
    }
    return least;
}


MJD maxMJD(std::set<MJD> mjds)
{
    MJD max;
    bool foundOne = false;
    std::set<MJD>::const_iterator mIter;
    for (mIter = mjds.begin(); mIter != mjds.end(); mIter++) {
        if ((!foundOne) || (*mIter > max)) {
            max = *mIter;
            foundOne = true;
        }
    }
    return max;
}


bool trackletIsCorrect(const std::vector<MopsDetection> & allDets,
                       Tracklet t) 
{
    std::string inferredName = "";
    std::set<unsigned int>::const_iterator detIter;

    /* getObjName is a MitiDetection thing, doesn't exist for MopsDetection

    for (detIter = t.indices.begin(); detIter != t.indices.end(); detIter++) {

        std::string detObjName = allDets.at(*detIter).getObjName();
        if (inferredName == "") {
            inferredName = detObjName;
        }


        if (detObjName == "NS") { 
            return false; }
        if (detObjName != inferredName) {
            return false;
        }

    }

    */
    return true;
}



MJD firstDetectionTime(const std::vector<MopsDetection> & allDets,
                       const Tracklet &t)
{
    bool foundOne = false;
    MJD minSoFar = 0;
    std::set<unsigned int>::const_iterator detIter;
    for (detIter = t.indices.begin(); detIter != t.indices.end(); detIter++) {
        double curMJD = allDets.at(*detIter).getEpochMJD();
        if ((!foundOne) || (curMJD < minSoFar)) {
            foundOne = true;
            minSoFar = curMJD;
        }
    }
    return minSoFar;
}



void findLinkableObjects(const std::vector<MopsDetection> & allDets, 
                         const std::vector<Tracklet> &allTracklets, 
                         linkTrackletsConfig searchConfig, 
                         std::vector<int> &findableObjectNames) 
{

    //
    // CTS: Switched getObjName() to getSsmId(), but I'm not sure that SsmId's
    // are the same logical thing as ObjNames.
    //

    //consider only detections which come from correct tracklets
    std::map<int, std::set<MJD> > nameToObsTimesMap;
    std::map<int, std::set<unsigned int> > nameToTrackletsMap;
    
    //get mappings from name to all observation times and name to all component tracklets.
    for (unsigned int trackletI = 0; trackletI != allTracklets.size(); trackletI++) {

        std::set<unsigned int>::const_iterator detIter;
        const Tracklet* curTracklet = &(allTracklets.at(trackletI));
        if (trackletIsCorrect(allDets, *curTracklet)) {
                
            for (detIter = curTracklet->indices.begin(); detIter != curTracklet->indices.end(); detIter++) {
                    
                int objName = allDets.at(*detIter).getSsmId();
                
                nameToObsTimesMap[objName].insert(allDets.at(*detIter).getEpochMJD());
                nameToTrackletsMap[objName].insert(trackletI);
                
            }
        }
    }


    // iterate through each object and see if its tracklets are legal.
    std::map<int, std::set<unsigned int> >::const_iterator objAndTrackletIter;
    for (objAndTrackletIter = nameToTrackletsMap.begin(); 
         objAndTrackletIter != nameToTrackletsMap.end();
         objAndTrackletIter++) {

        std::set<MJD> times = nameToObsTimesMap[objAndTrackletIter->first];

        std::set<MJD> trackletStartTimes;

        std::set<unsigned int>::const_iterator trackletIDIter;
        for (trackletIDIter = objAndTrackletIter->second.begin();
             trackletIDIter != objAndTrackletIter->second.end();
             trackletIDIter++) {
            trackletStartTimes.insert(firstDetectionTime(allDets, allTracklets.at(*trackletIDIter)));
        }
        
        MJD firstTrackletStartTime = minMJD(trackletStartTimes);
        MJD lastTrackletStartTime  = maxMJD(trackletStartTimes);
        
        //check if endpoint separation is sufficient
        if (fabs(lastTrackletStartTime - firstTrackletStartTime) > searchConfig.minEndpointTimeSeparation) {
            
            // collect all detection times and tracklets that could be used
            std::set<MJD> idealTrackDetTimes; 
            std::set<unsigned int> idealTrackTrackletIndices;
            
            // for each tracklet, if it is the first or last tracklet 
            // or a valid support tracklet, add to the idealTrack times and indices.
 
            for (trackletIDIter = objAndTrackletIter->second.begin();
                 trackletIDIter != objAndTrackletIter->second.end();
                 trackletIDIter++) {

                MJD firstObsTime = firstDetectionTime(allDets, allTracklets.at(*trackletIDIter));

                if ((firstObsTime == firstTrackletStartTime) || (firstObsTime == lastTrackletStartTime)) {

                    // this is an ideal endpoint
                    idealTrackTrackletIndices.insert(*trackletIDIter);
                    const Tracklet* curTracklet = &(allTracklets.at(*trackletIDIter));

                    std::set<unsigned int>::const_iterator detIter;
                    for (detIter = curTracklet->indices.begin();
                         detIter != curTracklet->indices.end();
                         detIter++) {
                        idealTrackDetTimes.insert(allDets.at(*detIter).getEpochMJD());
                    }
                }
                else if ((firstObsTime - firstTrackletStartTime > searchConfig.minSupportToEndpointTimeSeparation)
                         && 
                         (lastTrackletStartTime - firstObsTime  > searchConfig.minSupportToEndpointTimeSeparation)) {
                    // this is a valid support tracklet.
                    
                    idealTrackTrackletIndices.insert(*trackletIDIter);
                    const Tracklet* curTracklet = &(allTracklets.at(*trackletIDIter));

                    std::set<unsigned int>::const_iterator detIter;
                    for (detIter = curTracklet->indices.begin();
                         detIter != curTracklet->indices.end();
                         detIter++) {
                        idealTrackDetTimes.insert(allDets.at(*detIter).getEpochMJD());
                    }
                }
            }
           
            // we now have the set of all compatible detections/tracklet. check to see if there are enough.

            // CTS: linkTrackletsConfig has no member minSupportTracklets
            // (idealTrackTrackletIndices.size() - 2 > searchConfig.minSupportTracklets)
            if (idealTrackDetTimes.size() > searchConfig.minDetectionsPerTrack) {
                // track is valid - add object name to output
                findableObjectNames.push_back(objAndTrackletIter->first);
            }
        }
    }
        
}



    }} // close lsst::mops


