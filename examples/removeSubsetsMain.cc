// -*- LSST-C++ -*-
/* jonathan myers */

#include <unistd.h>
#include <getopt.h>

#include "lsst/mops/removeSubsets.h"
#include "lsst/mops/common.h"
#include "lsst/mops/Exceptions.h"
#include "lsst/mops/fileUtils.h"


namespace removeSubsets {

    int removeSubsetsMain(int argc, char** argv) {

        /* read pairs from a file, do the removal, and write the output */
        std::string USAGE("USAGE: removeSubsets --inFile <input pairfile> --outFile <output pairfile> [--removeSubsets <TRUE/FALSE> --keepOnlyLongest <TRUE/FALSE>]");
        std::ifstream inFile;
        std::ofstream outFile;
        std::vector <lsst::mops::Tracklet>* pairsVector = new std::vector<lsst::mops::Tracklet>;
        std::vector <lsst::mops::Tracklet>* outputVector = new std::vector<lsst::mops::Tracklet>;

        char* inFileName = NULL;
        char* outFileName = NULL;
        bool removeSubsets = true;
        bool keepOnlyLongestPerDet = false;
        
        static const struct option longOpts[] = {
            { "inFile", required_argument, NULL, 'i' },
            { "outFile", required_argument, NULL, 'o' },
            { "removeSubsets", required_argument, NULL, 'r' },
            { "keepOnlyLongest", required_argument, NULL, 'k'},
            { "help", no_argument, NULL, 'h' },
            { NULL, no_argument, NULL, 0 }
        };

        int longIndex = -1;
        const char* optString = "i:o:r:k:h";
        int opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        while( opt != -1 ) {
            switch( opt ) {
            case 'i':
                inFileName = optarg; 
                break;
                
            case 'o':
                outFileName = optarg ; 
                break;
                
            case 'r':
                removeSubsets = lsst::mops::guessBoolFromStringOrGiveErr(optarg, USAGE);
                break;
                
            case 'k':
                keepOnlyLongestPerDet = lsst::mops::guessBoolFromStringOrGiveErr(optarg, USAGE);
                break;
                
            case 'h':   /* fall-through is intentional */
            case '?':
                std::cout<<USAGE<<std::endl;
                return 0;
                break;
            default:
                throw LSST_EXCEPT(ProgrammerErrorException, "Programmer error in parsing of command-line options\n");
                break;
            }        
            opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        }

        if ((inFileName == NULL) || (outFileName == NULL)) {
            throw LSST_EXCEPT(CommandlineParseErrorException, 
                              "Please specify input and output filenames.\n\n" +
                              USAGE + "\n");
        }


        std::cout << "RemoveSubsets:" << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << "Input file:                                  " << inFileName << std::endl;
        std::cout << "Output file:                                 " << outFileName << std::endl;
        std::cout << "RemoveSubsets:                               "<< lsst::mops::boolToString(removeSubsets)
                  << std::endl;
        std::cout << "Keep only longest tracklet(s) per detection: "<< 
            lsst::mops::boolToString(keepOnlyLongestPerDet) << std::endl;

        inFile.open(inFileName);
        outFile.open(outFileName);
        lsst::mops::populatePairsVectorFromFile(inFile, *pairsVector);

        /* do the actual work */

        if (keepOnlyLongestPerDet == true) {
            std::vector <lsst::mops::Tracklet>* tmpVector = 
                new std::vector<lsst::mops::Tracklet>;
            putLongestPerDetInOutputVector(pairsVector, *tmpVector); 
            delete pairsVector;
            pairsVector = tmpVector;
        }

        lsst::mops::SubsetRemover mySR;
        
        if (removeSubsets == true) {
            mySR.removeSubsetsPopulateOutputVector(pairsVector, *outputVector);
        }
        else {
            delete outputVector;
            outputVector = pairsVector;
        }

        lsst::mops::writeTrackletsToOutFile(outputVector, outFile);
        if (outputVector != pairsVector) {
            delete outputVector;
        }
        delete pairsVector;
        return 0;
    }

}

int main(int argc, char** argv) {
    return removeSubsets::removeSubsetsMain(argc, argv);
}
