
from __future__ import print_function, division

# import lsst.mops.daymops as mops
import sys
import lsst.afw.geom as afwGeom
import lsst.pipe.base as pipeBase
import lsst.pex.config as pexConfig
from lsst.pipe.base import ArgumentParser, TaskError
from lsst.daf.base import DateTime
import traceback

import numpy as np

from . import findTracklets, findTrackletsConfig, MopsDetection, TrackletSet, doCollapsingPopulateOutputVector

from astropy.io import ascii

__all__ = ("MakeTrackletsTask", "MakeTrackletsConfig")


class MakeTrackletsConfig(pexConfig.Config):

    minV = pexConfig.Field(doc="Minimum velocity of tracklets (deg/day)",
                           dtype=float,
                           default=0.0)

    maxV = pexConfig.Field(doc="Maximum velocity of tracklets (deg/day)",
                           dtype=float,
                           default=2.0)

    minDt = pexConfig.Field(doc="Minimum time spacing between epochs in a tracklet (hours)",
                            dtype=float,
                            default=0.0)

    def getFindTrackletsConfig(self):
        output_config = findTrackletsConfig()
        for field_name, field_value in self.iteritems():
            if field_name in dir(output_config):
                output_config.__setattr__(field_name, field_value)
        return output_config


class MakeTrackletsTaskRunner(pipeBase.TaskRunner):


    @staticmethod
    def getTargetList(parsedCmd, **kwargs):
        return [parsedCmd.id.refList]


    def __call__(self, dataRefList):
        task = self.makeTask()
        result = None  # in case the task fails
        if self.doRaise:
            result = task.run(dataRefList)
        else:
            try:
                result = task.run(dataRefList)
            except Exception as e:
                # don't use a try block as we need to preserve the original exception
                task.log.fatal("Failed on dataRefs=%s: %s" % (dataRefList, e))

                if not isinstance(e, TaskError):
                    traceback.print_exc(file=sys.stderr)

class MakeTrackletsTask(pipeBase.CmdLineTask):
    ConfigClass = MakeTrackletsConfig
    RunnerClass = MakeTrackletsTaskRunner
    _DefaultName = "MakeTrackletsTask"

    @classmethod
    def _makeArgumentParser(cls):
        parser = ArgumentParser(name=cls._DefaultName)
        parser.add_id_argument(name="--id", datasetType="deepDiff_diaSrc",
                               help="data IDs, e.g. --id visit=12345 ccd=1,2^0,3")
        return parser

    def __init__(self, *args, **kwargs):
        pipeBase.Task.__init__(self, *args, **kwargs)

    def make_detections(self, catalog, visit, mjd):
        SNR = catalog['slot_PsfFlux_flux']/catalog['slot_PsfFlux_fluxSigma']
        detections = [MopsDetection(id, mjd, ra, dec, obsHistId=visit, snr=snr) for
                      (id, ra, dec, snr) in
                      zip(catalog['id'] % 10000,
                          np.degrees(catalog['coord_ra']),
                          np.degrees(catalog['coord_dec']),
                          SNR)]
        print(np.min(np.degrees(catalog['coord_ra'])),
              np.max(np.degrees(catalog['coord_ra'])),
                          np.min(np.degrees(catalog['coord_dec'])),
                          np.max(np.degrees(catalog['coord_dec'])))
        return detections

    def write_out_tracklets(self, tracklets, output_filename):

        f_out = open(output_filename, "w")
        for tracklet in tracklets:
            indices = list(tracklet.indices())
            detection_params = []
            for det_index in indices:
                visit_str = "{:d},{:f},{:f},{:.1f}".format(detection_table['visitid'][det_index],
                                                           detection_table['coord_ra'][det_index],
                                                           detection_table['coord_dec'][det_index],
                                                           detection_table['SNR'][det_index])
                detection_params.append(visit_str)
            output_string = ",".join(detection_params)
            print(output_string, file=f_out)
        f_out.close()


    @pipeBase.timeMethod
    def run(self, dataRefList):

        detections = []
        for ref in dataRefList:
            exposure = ref.get("deepDiff_differenceExp")
            catalog = ref.get("deepDiff_diaSrc")

            exposure_time_string = exposure.getMetadata().get("DATE")
            exposure_mjd = DateTime(exposure_time_string + "Z").get(DateTime.MJD)
            visit = ref.dataId['visit']
            detections.extend(self.make_detections(catalog, visit, exposure_mjd))

        tracklets = findTracklets(detections, self.config.getFindTrackletsConfig())
        print("Number of uncollapsed tracklets: {:d}".format(len(tracklets)))


        # Need to convert these to config settings
        # Tolerances in RA, Dec, Angle, Velocity
        tolerances = [0.002, 0.002, 5, 0.05]
        useMinimumRMS = False
        useBestFit = False
        useRMSFilt = False
        verbose = False
        maxRMS = 0.001

        collapsed_tracklets = TrackletSet()
        doCollapsingPopulateOutputVector(detections, tracklets,
                                         tolerances,
                                         collapsed_tracklets,
                                         useMinimumRMS, useBestFit, useRMSFilt,
                                         maxRMS, verbose)
        print("Number of collapsed tracklets: {:d}".format(len(collapsed_tracklets)))

        sel_three_det, = np.where([len(t.indices()) > 2 for t in collapsed_tracklets])
        self.write_out_tracklets([collapsed_tracklets[n] for n in sel_three_det],
                                 "threedet_tracklets_Sept1")
        print("Number of 3-detection tracklets: {:d}".format(len(sel_three_det)))

        return pipeBase.Struct(tracklets=collapsed_tracklets)

    # Overriding these two functions prevent the task from attempting to save the config.
    # Can remove these when we have mapper entries for the config.
    def _getConfigName(self):
        return None
    def _getMetadataName(self):
        return None

