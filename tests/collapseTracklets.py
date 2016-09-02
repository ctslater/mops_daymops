

import unittest
import lsst.mops.daymops as daymops

class CollapseTracklets(unittest.TestCase):

    @unittest.skip("Test not fully written yet")
    def broken_test(self):
        detections = [# 4 matching detections
                      daymops.MopsDetection(5330.0, 1.0, 1.0),
                      daymops.MopsDetection(5330.1, 0.0, 0.0),
                      daymops.MopsDetection(5330.2, 359.0, 359.0),
                      daymops.MopsDetection(5330.3, 358.0, 358.0),
                      # 4 unrelated detections
                      daymops.MopsDetection(5330.0, 1.0, 0.8),
                      daymops.MopsDetection(5330.1, 0.0, 0.2),
                      daymops.MopsDetection(5330.2, 359.0, 358.8),
                      daymops.MopsDetection(5330.3, 358.0, 358.2),
                     ]

        t1 = daymops.Tracklet([0, 1])
        t2 = daymops.Tracklet([4, 5])

        # BROKEN

    # Copy of C++ test doCollapsingPopulateOutputVector_blackbox_1
    def testCollapsing(self):
        detections = [daymops.MopsDetection(0, 5330.0, 10.0, 10.0),
                      daymops.MopsDetection(1, 5331.0, 11.0, 11.0),
                      daymops.MopsDetection(2, 5332.0, 12.0, 12.0),
                      daymops.MopsDetection(3, 5333.0, 13.0, 13.0),
                     ]

        tolerances = [0.01, 0.01, 1.0, 0.01]
        t1 = daymops.Tracklet([0, 1])
        t2 = daymops.Tracklet([2, 3])

        tracklets = daymops.TrackletSet([t1, t2])
        output = daymops.TrackletSet()
        daymops.doCollapsingPopulateOutputVector(detections, tracklets,
                                                 tolerances, output, False,
                                                 False, False, 0.0, False)
        self.assertTrue(all([x in output[0].indices() for x in (0,1,2,3)]))

    # C++ version: doCollapsingPopulateOutputVector_blackbox_2
    def testCollapsing_lessPrecise(self):
        detections = [daymops.MopsDetection(0, 5330.0, 10.0, 10.0),
                      daymops.MopsDetection(1, 5331.0, 11.0, 11.5),
                      daymops.MopsDetection(2, 5332.0, 12.0, 12.0),
                      daymops.MopsDetection(3, 5333.0, 13.0, 13.5),
                     ]

        tolerances = [2.0, 2.0, 45.0, 1.0]
        t1 = daymops.Tracklet([0, 1])
        t2 = daymops.Tracklet([2, 3])

        tracklets = daymops.TrackletSet([t1, t2])
        output = daymops.TrackletSet()
        daymops.doCollapsingPopulateOutputVector(detections, tracklets,
                                                 tolerances, output, False,
                                                 False, False, 0.0, False)
        self.assertTrue(all([x in output[0].indices() for x in (0,1,2,3)]))

    # doCollapsingPopulateOutputVector_blackbox_3
    def testCollapsing_morePrecise(self):
        # Should not collapse anything
        detections = [daymops.MopsDetection(0, 5330.0, 10.0, 10.0),
                      daymops.MopsDetection(1, 5331.0, 11.0, 11.5),
                      daymops.MopsDetection(2, 5332.0, 12.0, 12.0),
                      daymops.MopsDetection(3, 5333.0, 13.0, 13.5),
                     ]

        tolerances = [0.5, 0.5, 5.0, 0.5]
        t1 = daymops.Tracklet([0, 1])
        t2 = daymops.Tracklet([2, 3])

        tracklets = daymops.TrackletSet([t1, t2])
        output_tracklets = daymops.TrackletSet()
        daymops.doCollapsingPopulateOutputVector(detections, tracklets,
                                                 tolerances, output_tracklets, False,
                                                 False, False, 0.0, False)
        self.assertTrue(len(output_tracklets) == 2)

    def testLongIDs(self):
        """Ensure that arbitrary detection IDs are handled properly between
        findTracklets and collapseTracklets. This is necessary because the index
        fields in a tracklet can become confused between detection ID and
        position in the detection vector, leading to out of bounds errors.
        """
        detections = [daymops.MopsDetection(130344998938869947, 5330.0, 10.0, 10.0),
                      daymops.MopsDetection(130344998938869948, 5331.0, 11.0, 11.0),
                      daymops.MopsDetection(130344998938869949, 5332.0, 12.0, 12.0),
                      daymops.MopsDetection(130344998938869950, 5333.0, 13.0, 13.0),
                     ]

        config  = daymops.findTrackletsConfig()
        config.maxV = 1.5
        config.maxDt = 3.0
        tracklets = daymops.findTracklets(detections, config)
        self.assertTrue(len(tracklets) > 2)
        output = daymops.TrackletSet()

        tolerances = [0.01, 0.01, 1.0, 0.01]
        import pdb
        pdb.set_trace()
        daymops.doCollapsingPopulateOutputVector(detections, tracklets,
                                                 tolerances, output, False,
                                                 False, False, 0.0, False)
        print(output, len(output))
        self.assertEqual(len(output), 1)
        self.assertTrue(all([x in output[0].indices() for x in (0,1,2,3)]))


