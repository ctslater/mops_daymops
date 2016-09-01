
from __future__ import print_function, division

# import lsst.mops.daymops as mops
import sys
import lsst.afw.geom as afwGeom
import lsst.pipe.base as pipeBase
import lsst.pex.config as pexConfig
from lsst.pipe.tasks.selectImages import WcsSelectImagesTask
from lsst.pipe.base import ArgumentParser, TaskError
from lsst.daf.base import DateTime
import traceback

import numpy as np

from . import findTracklets, findTrackletsConfig, MopsDetection

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

    select = pexConfig.ConfigurableField(
        doc = "Image selection subtask.",
        target = WcsSelectImagesTask,
    )

    def getFindTrackletsConfig(self):
        output_config = findTrackletsConfig()
        for field_name, field_value in self.iteritems():
            if field_name in dir(output_config):
                output_config.__setattr__(field_name, field_value)
        return output_config


def getDetections():
    detection_table = ascii.read("night1")
    detections = [MopsDetection(id, mjd, ra, dec, obsHistId=imgId, snr=snr) for
                  (id, mjd, ra, dec, imgId, snr) in
                  zip(xrange(len(detection_table)), detection_table['MJD'],
                      detection_table['coord_ra'],
                      detection_table['coord_dec'],
                      detection_table['visitid'],
                      detection_table['SNR'])]
    return detections


class MakeTrackletsTaskRunner(pipeBase.TaskRunner):


    @staticmethod
    def getTargetList(parsedCmd, **kwargs):
        #argDict = dict(calExpList=parsedCmd.calexp.idList)
        #return [(dataId, argDict) for dataId in parsedCmd.id.idList]
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
        self.makeSubtask("select")

    def make_detections(self, catalog, visit, mjd):
        SNR = catalog['slot_PsfFlux_flux']/catalog['slot_PsfFlux_fluxSigma']
        detections = [MopsDetection(id, mjd, ra, dec, obsHistId=visit, snr=snr) for
                      (id, ra, dec, snr) in
                      zip(catalog['id'],
                          np.degrees(catalog['coord_ra']),
                          np.degrees(catalog['coord_dec']),
                          SNR)]
        return detections


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
        print("Number of tracklets: {:d}".format(len(tracklets)))
        return pipeBase.Struct(tracklets=tracklets)

        sys.exit(0)

        skyInfo = self.getSkyInfo(dataRef)
        # calExpRefList = self.selectExposures(dataRef, skyInfo, selectDataList=selectDataList)

        skyInfo = self.getSkyInfo(dataRef)
        cornerPosList = afwGeom.Box2D(skyInfo.bbox).getCorners()
        coordList = [skyInfo.wcs.pixelToSky(pos) for pos in cornerPosList]
        selected_exposures = self.select.runDataRef(dataRef, coordList,
                                                    selectDataList=selectDataList).dataRefList
        print("Selected Exposures ", selected_exposures)

        detections = getDetections()
        tracklets = findTracklets(detections, self.config.getFindTrackletsConfig())
        print("Number of tracklets: {:d}".format(len(tracklets)))
        return pipeBase.Struct(tracklets=tracklets)

    # Overriding these two functions prevent the task from attempting to save the config.
    # Can remove these when we have mapper entries for the config.
    def _getConfigName(self):
        return None
    def _getMetadataName(self):
        return None

