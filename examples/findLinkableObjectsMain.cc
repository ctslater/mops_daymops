

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

int main(int argc, char** argv) 
{
     std::string helpString = 
	  "Usage: linkTracklets -d <detections file> -t <tracklets file> -o <output (tracks) file>";
     
     static const struct option longOpts[] = {
	  { "detectionsFile", required_argument, NULL, 'd' },
	  { "trackletsFile", required_argument, NULL, 't' },
	  { "outputFile", required_argument, NULL, 'o' },
	  { "help", no_argument, NULL, 'h' },
	  { NULL, no_argument, NULL, 0 }
     };  
     
     
     std::stringstream ss;
     std::string detectionsFileName = "";
     std::string trackletsFileName = "";
     std::string outputFileName = "";
     
     int longIndex = -1;
     const char *optString = "d:t:o:h";
     int opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
     while( opt != -1 ) {
	  switch( opt ) {
	  case 'd':	       
	       /*ss << optarg; 
		 ss >> detectionsFileName;*/
	       detectionsFileName = optarg;
	       break;
	  case 't':
	       /*ss << optarg;
		 ss >> trackletsFileName; */
	       trackletsFileName = optarg;
	       break;
	  case 'o':
	       /*ss << optarg;
		 ss >> outputFileName; */
	       outputFileName = optarg;
	       break;
	  case 'h':
	       std::cout << helpString << std::endl;
	       return 0;
	  default:
	       break;
	  }
	  opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
     }

     if ((detectionsFileName == "") || (trackletsFileName == "") || (outputFileName == "")) {
	  std::cerr << helpString << std::endl;
	  return 1;
     }

     std::vector<lsst::mops::MopsDetection> allDets;
     std::vector<lsst::mops::Tracklet> allTracklets;


     populateDetVectorFromFile(detectionsFileName, allDets);
     populatePairsVectorFromFile(trackletsFileName, allTracklets);
     
     lsst::mops::linkTrackletsConfig searchConfig; 

     std::vector<int> findableObjectNames;

     lsst::mops::findLinkableObjects(allDets, allTracklets, searchConfig, findableObjectNames);

     // write to disk
     std::ofstream outFile;
     outFile.open(outputFileName.c_str());
     for (unsigned int i = 0; i < findableObjectNames.size(); i++) {
         outFile << findableObjectNames.at(i) << std::endl;
     }
     outFile.close();

}

