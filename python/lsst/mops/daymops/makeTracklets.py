
from __future__ import print_function, division

# import lsst.mops.daymops as mops
import sys
import lsst.afw.geom as afwGeom
import lsst.pipe.base as pipeBase
import lsst.pex.config as pexConfig
from lsst.pipe.base import ArgumentParser, TaskError
from lsst.daf.base import DateTime
import traceback
import itertools
import sqlite3

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
                task.log.fatal("Failed on provided dataRefs: %s" % (e))

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

        if(len(catalog) == 0):
            return []

        SNR = catalog['slot_PsfFlux_flux']/catalog['slot_PsfFlux_fluxSigma']
        template_SNR = catalog['template_base_PsfFlux_flux']/catalog['template_base_PsfFlux_fluxSigma']
        sel, = np.where((SNR > 0) & (template_SNR < 100) &
                        (catalog['ip_diffim_DipoleFit_flag_classification'] == 0))
        detections = [MopsDetection(id, mjd, ra, dec, ImageID=visit, snr=snr) for
                      (id, ra, dec, snr) in
                      zip(catalog['id'][sel],
                          np.degrees(catalog['coord_ra'][sel]),
                          np.degrees(catalog['coord_dec'][sel]),
                          SNR[sel])]

        return detections

    def write_out_tracklets(self, detections, tracklets, output_filename):

        f_out = open(output_filename, "w")
        print("# Visit, MJD, RA, Dec, SNR", file=f_out)

        fmt_str = "{:d},{:f},{:f},{:f},{:.1f}"
        for tracklet in tracklets:
            indices = list(tracklet.indices())
            detection_params = []
            for det_index in indices:
                visit_str = fmt_str.format(detections[det_index].ImageID,
                                           detections[det_index].EpochMJD,
                                           detections[det_index].RA,
                                           detections[det_index].Dec,
                                           detections[det_index].SNR)
                detection_params.append(visit_str)
            output_string = ",".join(detection_params)
            print(output_string, file=f_out)
        f_out.close()


    @pipeBase.timeMethod
    def run(self, dataRefList):

        first_visit_id = min([ref.dataId['visit'] for ref in dataRefList])

        detections = []
        for ref in dataRefList:
            # For some reason the decam calexps don't have DATE-OBS
            instcal_md = ref.get("instcal_md")
            catalog = ref.get("deepDiff_diaSrc")

            # MJD of shutter open
            exposure_mjd = instcal_md.get("MJD-OBS")
            # Adjust to the midpoint of the observation
            exposure_mjd += 0.5*instcal_md.get("EXPTIME")/(24*3600.0)
            visit = ref.dataId['visit']
            detections.extend(self.make_detections(catalog, visit, exposure_mjd))

        # See tests/findTracklets.py testLongIds()
        for n, det in enumerate(detections):
            det.index = n

        # write_out_detections_ds9(detections)

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
        self.write_out_sqlite(detections, collapsed_tracklets)
        sys.exit(0)

        sel_three_det, = np.where([len(t.indices()) > 2 for t in collapsed_tracklets])
        print("Number of 3-detection tracklets: {:d}".format(len(sel_three_det)))

        sel_four_det, = np.where([len(t.indices()) > 3 for t in collapsed_tracklets])
        print("Number of 4-detection tracklets: {:d}".format(len(sel_four_det)))
        self.write_out_tracklets(detections,
                                 [collapsed_tracklets[n] for n in sel_four_det],
                                 "fourdet_tracklets_{:d}".format(first_visit_id))

        # write_out_tracklets_ds9(collapsed_tracklets, detections)

        return pipeBase.Struct(tracklets=collapsed_tracklets)

    def write_out_sqlite(self, detections, tracklets):
        db_conn = sqlite3.connect("tracklets.db")
        cursor = db_conn.cursor()

        cursor.execute("CREATE TABLE IF NOT EXISTS tracklets "
                       "(id INTEGER PRIMARY KEY, "
                       "MJD REAL, center_ra REAL, center_dec REAL)")
        cursor.execute("CREATE TABLE IF NOT EXISTS detections "
                       "(id INTEGER PRIMARY KEY, visit VISIT, MJD REAL, ra REAL, dec REAL)")
        cursor.execute("CREATE TABLE IF NOT EXISTS tracklet_links "
                       "(id INTEGER PRIMARY KEY, "
                       "tracklet INT, detection INT, "
                       "FOREIGN KEY(tracklet) REFERENCES tracklets(id), "
                       "FOREIGN KEY(detection) REFERENCES detections(id))")

        detections_to_load = set()
        for tracklet in tracklets:
            detections_to_load.update(tracklet.indices())

        for det_id in detections_to_load:
            detection = detections[det_id]
            cursor.execute("INSERT INTO detections VALUES (?,?,?,?,?)",
                           (detection.ID, detection.ImageID, detection.EpochMJD,
                            detection.RA, detection.Dec))

        for tracklet in tracklets:
            indices = tracklet.indices()
            tracklet_dets = [detections[x] for x in indices]
            mean_ra = np.mean([det.RA for det in tracklet_dets])
            mean_dec = np.mean([det.Dec for det in tracklet_dets])
            mean_mjd = np.mean([det.EpochMJD for det in tracklet_dets])
            cursor.execute("INSERT INTO tracklets (MJD, center_ra, center_dec) VALUES "
                           "(?, ?, ?)", (mean_mjd, mean_ra, mean_dec))
            tracklet_id = cursor.lastrowid

            for idx in indices:
                cursor.execute("INSERT INTO tracklet_links VALUES "
                               "(NULL, ?, ?)",
                               (tracklet_id, detections[idx].ID))



            # set([0L, 13391L, 3540L, 6855L])
        db_conn.commit()

        #import pdb
        #pdb.set_trace()

        db_conn.close()

    def write_out_detections_ds9(detections):
        """Debuging tool for writing out a ds9 region file.
        """
        det_region = open("detections.reg", "w")
        print("ICRS", file=det_region)
        colors = ['red', 'green', 'blue', 'yellow', 'pink', 'cyan']
        color_iter = itertools.cycle(colors)
        last_visit = 0
        for n, det in enumerate(detections):
            if det.ImageID != last_visit:
                visit_color = color_iter.next()
                last_visit = det.ImageID
            print("point({:f},{:f}) # point=x color={:s} text=\"{:d}\"".format(det.RA, det.Dec,
                                                                 visit_color, n),
                  file=det_region)
        det_region.close()

    def write_out_tracklets_ds9(tracklets, detections):
        tracklet_region = open("tracklets.reg", "w")
        print("ICRS", file=tracklet_region)

        colors = ['red', 'green', 'blue', 'yellow', 'pink', 'cyan']
        color_iter = itertools.cycle(colors)

        for tracklet in tracklets:
            indices = list(tracklet.indices())
            if len(indices) == 2:
                continue

            color = color_iter.next()
            for index1, index2 in zip(indices[:-1], indices[1:]):
                det1 = detections[index1]
                det2 = detections[index2]
                print("line({:f},{:f},{:f},{:f}) # color={:s}".format(det1.RA, det1.Dec,
                                                                      det2.RA, det2.Dec,
                                                                      color),
                  file=tracklet_region)
        tracklet_region.close()


    # Overriding these two functions prevent the task from attempting to save the config.
    # Can remove these when we have mapper entries for the config.
    def _getConfigName(self):
        return None
    def _getMetadataName(self):
        return None

