

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



