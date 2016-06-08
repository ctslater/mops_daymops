
import unittest

import lsst.mops.daymops as daymops

def containsPair(id1, id2, tracklet_list):

    for tracklet in tracklet_list:
        if id1 in tracklet.indices() and id2 in tracklet.indices():
            return True
    return False

class FindTracklets(unittest.TestCase):

    def testEmptyDetections(self):
        detections = []
        config = daymops.findTrackletsConfig()
        config.maxV = 1.0
        config.maxDt = 1.5
        tracklets = daymops.findTracklets(detections, config)
        self.assertEqual(len(tracklets), 0)

    def testSingleTracklet(self):
        detections = [daymops.MopsDetection(0, 53736.0, 100.0, 10.0),
                      daymops.MopsDetection(1, 53737.0, 100.1, 10.1),
                     ]
        config = daymops.findTrackletsConfig()
        config.maxV = 1.0
        config.maxDt = 1.5
        tracklets = daymops.findTracklets(detections, config)

        self.assertEqual(len(tracklets), 1)

    # test 3 dets of same object gets 3 pairs
    def testSingleTracklet(self):
        detections = [daymops.MopsDetection(0, 53736.0, 100.0, 10.0),
                      daymops.MopsDetection(1, 53737.0, 100.1, 10.1),
                      daymops.MopsDetection(2, 53738.0, 100.2, 10.2),
                     ]
        config = daymops.findTrackletsConfig()
        config.maxV = 1.0
        config.maxDt = 3.0
        tracklets = daymops.findTracklets(detections, config)

        self.assertEqual(len(tracklets), 3)
        self.assertTrue(containsPair(0, 1, tracklets))
        self.assertTrue(containsPair(0, 2, tracklets))
        self.assertTrue(containsPair(1, 2, tracklets))

    # 2 image times, 2 objects = 2 tracklets
    def testTwoPairs(self):
        detections = [daymops.MopsDetection(0, 53736.0, 100.0, -10.0),
                      daymops.MopsDetection(1, 53737.0, 100.1, -10.1),
                      daymops.MopsDetection(2, 53736.0, 150.0, -10.0),
                      daymops.MopsDetection(3, 53737.0, 150.1, -10.1),
                     ]
        config = daymops.findTrackletsConfig()
        config.maxV = 1.0
        config.maxDt = 3.0
        tracklets = daymops.findTracklets(detections, config)

        self.assertEqual(len(tracklets), 2)
        self.assertTrue(containsPair(0, 1, tracklets))
        self.assertTrue(containsPair(2, 3, tracklets))


    # check that velocity filter is working
    def testVelocityFilter(self):
        detections = [daymops.MopsDetection(0, 53736.0, 100.0, 80.0),
                      daymops.MopsDetection(1, 53737.0, 100.1, 80.1),
                      daymops.MopsDetection(2, 53736.0, 150.0, 0.0),
                      daymops.MopsDetection(3, 53737.0, 151.1, 0.1),
                     ]
        config = daymops.findTrackletsConfig()
        config.maxV = 1.0
        config.maxDt = 3.0
        tracklets = daymops.findTracklets(detections, config)

        self.assertEqual(len(tracklets), 1)
        self.assertTrue(containsPair(0, 1, tracklets))
