# addImagingInfo.py: Python function to add meta info to a CASA image
# Copyright (C) 2012
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
#
# @author Ger van Diepen

import os
import pyrap.tables as pt
import lofar.parmdb as pdb
import lofar.get_rms_noise as grn
import numpy as np

""" Add a subtable of an MS to the image """
def addSubTable (image, msName, subName, removeColumns=[]):
    # Make a selection of all rows/columns of the MS subtable
    sel = pt.taql ("select * from '" + msName + "/" + subName + "'")
    # Remove the required columns.
    if len(removeColumns) > 0:
        sel.removecols (removeColumns)
    # Strip LOFAR_ from column names
    for col in sel.colnames():
        if len(col) > 6  and  col[:6] == "LOFAR_":
            sel.renamecol (col, col[6:])
    # Copy the subtable to the image and add it as a subtable.
    # Always prefix subtable name with LOFAR_.
    subNameOut = subName;
    if len(subNameOut) < 6  or  subNameOut[:6] != "LOFAR_":
        subNameOut = "LOFAR_" + subNameOut
    subtab = sel.copy (image.name() + "/" + subNameOut, deep=True)
    image.putkeyword ("ATTRGROUPS." + subNameOut, subtab)
    print "Added subtable", subNameOut, "containing", subtab.nrows(), "rows"
    subtab.close()
    sel.close()

""" Create the empty LOFAR_QUALITY subtable """
def addQualityTable (image, usedCounts, visCounts):
    # Create the table using TaQL.
    tab = pt.taql ("create table '" + image.name() + "/LOFAR_QUALITY' " + 
                   "QUALITY_MEASURE string, VALUE string, FLAG_ROW bool")
    # Get the rms noise of I,Q,U,V as list of tuples.
    noises = grn.get_rms_noise (image.name())
    for noise in noises:
        row = tab.nrows()
        tab.addrows (2)
        tab.putcell ("QUALITY_MEASURE", row, "RMS_NOISE_"+noise[0])
        tab.putcell ("VALUE", row, str(noise[1]))
        tab.putcell ("FLAG_ROW", row, False)
        perc = 100.
        nvis = 1.0 * visCounts.sum()
        if nvis > 0:
            # Get flagged percentage to 2 decimals.
            perc = int(10000. * (1 - usedCounts.sum() / nvis) + 0.5) / 100.
        tab.putcell ("QUALITY_MEASURE", row+1, "PERC_FLAGGED_VIS")
        tab.putcell ("VALUE", row+1, str(perc)[:5])
        tab.putcell ("FLAG_ROW", row+1, False)
    tab.flush()
    image.putkeyword ("ATTRGROUPS." + "LOFAR_QUALITY", tab)
    print "Added subtable LOFAR_QUALITY containing", tab.nrows(), "rows"
    tab.close()

""" Create the LOFAR_ORIGIN subtable and fill from all MSs """
def addOriginTable (image, msNames):
    # Concatenate the OBSERVATION subtables of all MSs.
    obsNames = [name + "/OBSERVATION" for name in msNames]
    obstab = pt.table(obsNames, ack=False)
    # Select and rename the required columns.
    # Some columns are not in the LOFAR_OBSERVATION table. Create them by
    # selecting a similarly typed column and fill them later.
    selstr  =  "LOFAR_OBSERVATION_ID as OBSERVATION_ID"
    selstr += ",LOFAR_SUB_ARRAY_POINTING as SUB_ARRAY_POINTING"
    selstr += ",LOFAR_SUB_ARRAY_POINTING as SUBBAND"
    selstr += ",LOFAR_SUB_ARRAY_POINTING as NUM_CHAN"
    selstr += ",LOFAR_SUB_ARRAY_POINTING as NTIME_AVG"
    selstr += ",LOFAR_SUB_ARRAY_POINTING as NCHAN_AVG"
    selstr += ",LOFAR_OBSERVATION_FREQUENCY_MIN as CHANNEL_WIDTH"
    selstr += ",LOFAR_OBSERVATION_FREQUENCY_MIN as EXPOSURE"
    selstr += ",LOFAR_OBSERVATION_FREQUENCY_MIN as FREQUENCY_MIN"
    selstr += ",LOFAR_OBSERVATION_FREQUENCY_MAX as FREQUENCY_MAX"
    selstr += ",LOFAR_OBSERVATION_FREQUENCY_CENTER as FREQUENCY_CENTER"
    selstr += ",LOFAR_OBSERVATION_START as START"
    selstr += ",LOFAR_OBSERVATION_END as END"
    selstr += ",FLAG_ROW"
    sel = obstab.select(selstr)
    # Copy the subtable to the image and add it as a subtable.
    subtab = sel.copy (image.name() + "/" + "LOFAR_ORIGIN", deep=True)
    subtab = pt.table (image.name() + "/" + "LOFAR_ORIGIN", readonly=False,
                       ack=False)
    obstab.close()
    image.putkeyword ("ATTRGROUPS." + "LOFAR_ORIGIN", subtab)
    # Set the correct units of columns to update.
    subtab.putcolkeyword ("CHANNEL_WIDTH", "QuantumUnits", ["Hz"])
    subtab.putcolkeyword ("EXPOSURE", "QuantumUnits", ["s"])
    subtab.putcolkeyword ("START", "MEASINFO", {"Ref":"UTC", "type":"epoch"})
    subtab.putcolkeyword ("END", "MEASINFO", {"Ref":"UTC", "type":"epoch"})
    # Update the columns not in OBSERVATION table.
    # Get EXPOSURE from first row in main tables.
    # Get NUM_CHAN from SPECTRAL_WINDOW subtables.
    # Calculate CHANNEL_WIDTH (convert from MHz to Hz).
    # Get SUBBAND from MS name.
    for i in range(len(msNames)):
        t = pt.table(msNames[i], ack=False)
        subtab.putcell ("EXPOSURE", i, t.getcell("EXPOSURE", 0))
        t1 = pt.table(t.getkeyword("SPECTRAL_WINDOW"), ack=False)
        numchan = t1.getcell("NUM_CHAN", 0)
        subtab.putcell ("NUM_CHAN", i, numchan)
        freqs   = t1.getcell("CHAN_FREQ", 0);
        fwidths = t1.getcell("CHAN_WIDTH", 0);
        sfreq = freqs[0]  - 0.5*fwidths[0];
        efreq = freqs[-1] + 0.5*fwidths[-1]
        subtab.putcell ("FREQUENCY_MIN", i, sfreq);
        subtab.putcell ("FREQUENCY_MAX", i, efreq);
        subtab.putcell ("FREQUENCY_CENTER", i, t1.getcell("REF_FREQUENCY",0));
        subtab.putcell ("CHANNEL_WIDTH", i, fwidths[0])
        # Determine the averaging factors.
        avgfreq = 1
        avgtime = 1
        if ("LOFAR_FULL_RES_FLAG" in t.colnames()):
            avgfreq = t.getcolkeyword ("LOFAR_FULL_RES_FLAG", "NCHAN_AVG")
            avgtime = t.getcolkeyword ("LOFAR_FULL_RES_FLAG", "NTIME_AVG")
        subtab.putcell ("NCHAN_AVG", i, avgfreq)
        subtab.putcell ("NTIME_AVG", i, avgtime)
        t1.close()
        # Determine nr of data points flagged
        t.close()
        subband = 0
        inx = msNames[i].find ("SB")
        if inx>= 0:
            try:
                subband = int(msNames[i][inx+2:inx+5])
            except:
                pass
        subtab.putcell ("SUBBAND", i, subband)
    # Ready
    subtab.close()
    sel.close()
    print "Added subtable LOFAR_ORIGIN containing", len(msNames), "rows"

""" Create the LOFAR_SOURCE subtable and fill from the SourceDB """
def addSourceTable (image, sourcedbName, minTime, maxTime):
    # Create the table using TaQL.
    tab = pt.taql ("create table '" + image.name() + "/LOFAR_SOURCE' " + 
                   "SOURCE_ID int, \TIME double, INTERVAL double, " +
                   "NUM_LINES int, NAME string, " +
                   "DIRECTION double shape=[2], " +
                   "PROPER_MOTION double shape=[2], " +
                   "FLUX double shape=[4], " +
                   "SPINX double, REF_FREQUENCY double, " +
                   "SHAPE double shape=[3]")
    tab.putcolkeyword ("TIME", "QuantumUnits", ["s"])
    tab.putcolkeyword ("INTERVAL", "QuantumUnits", ["s"])
    tab.putcolkeyword ("DIRECTION", "QuantumUnits", ["rad"])
    tab.putcolkeyword ("PROPER_MOTION", "QuantumUnits", ["rad/s"])
    tab.putcolkeyword ("FLUX", "QuantumUnits", ["Jy"])
    tab.putcolkeyword ("REF_FREQUENCY", "QuantumUnits", ["MHz"])
    tab.putcolkeyword ("SHAPE", "QuantumUnits", ["rad", "rad", "rad"])
    tab.putcolkeyword ("TIME", "MEASINFO", {"Ref":"UTC", "type":"epoch"})
    tab.putcolkeyword ("DIRECTION", "MEASINFO", {"Ref":"J2000", "type":"direction"})
    tab.flush()
    image.putkeyword ("ATTRGROUPS." + "LOFAR_SOURCE", tab)
    # Get all parameters from the source parmdb.
    midtime = (minTime + maxTime) / 2
    inttime = maxTime - minTime
    sourcedb = pdb.parmdb(sourcedbName)
    # Get all source names by getting the Ra parms from DEFAULTVALUES
    names = [name[3:] for name in sourcedb.getDefNames ("Ra:*")]
    values = sourcedb.getDefValues()
    sourcedb = 0   # close
    row = 0
    tab.addrows (len(names))
    # Add the info of all sources.
    # The field names below are as used in SourceDB.
    fldnames = ["Ra", "Dec", "I", "Q", "U", "V", "SpectralIndex:0",
                "ReferenceFrequency", "Orientation", "MajorAxis", "MinorAxis"]
    vals = [0. for fld in fldnames]
    for name in names:
        for i in range(len(fldnames)):
            key = fldnames[i] + ":" + name
            if values.has_key (key):
                vals[i] = values[key][0][0]
            else:
                vals[i] = 0.
        tab.putcell ("SOURCE_ID", row, row)
        tab.putcell ("TIME", row, midtime);
        tab.putcell ("INTERVAL", row, inttime);
        tab.putcell ("NUM_LINES", row, 0);
        tab.putcell ("NAME", row, name);
        tab.putcell ("DIRECTION", row, vals[:2]);
        tab.putcell ("PROPER_MOTION", row, (0.,0.));
        tab.putcell ("FLUX", row, vals[2:6]);
        tab.putcell ("SPINX", row, vals[6]);
        tab.putcell ("REF_FREQUENCY", row, vals[7]);
        tab.putcell ("SHAPE", row, vals[8:11]);
        row += 1
    # Ready.
    tab.close()
    print "Added subtable LOFAR_SOURCE containing", row, "rows"

""" Update times and frequencies in the LOFAR_OBSERVATION subtable """
def updateObsTable (image, msName, minbl, maxbl, usedCounts, visCounts,
                    minTime, maxTime):
    obstab = pt.table (image.name() + "/LOFAR_OBSERVATION", readonly=False,
                       ack=False)
    oritab = pt.table (image.name() + "/LOFAR_ORIGIN", ack=False)
    minfreq = pt.taql ("calc min([select FREQUENCY_MIN from '" +
                       oritab.name() + "'])")
    maxfreq = pt.taql ("calc max([select FREQUENCY_MAX from '" +
                       oritab.name() + "'])")
    obstab.putcell ("OBSERVATION_FREQUENCY_MIN", 0, minfreq[0]);
    obstab.putcell ("OBSERVATION_FREQUENCY_MAX", 0, maxfreq[0]);
    obstab.putcell ("OBSERVATION_FREQUENCY_CENTER", 0, (minfreq[0]+maxfreq[0])/2);
    obstab.putcell ("OBSERVATION_START", 0, minTime);
    obstab.putcell ("OBSERVATION_END", 0, maxTime);
    obstab.putcell ("TIME_RANGE", 0, (minTime, maxTime));
    obstab.putcell ("FILENAME", 0, os.path.basename(image.name()))
    obstab.putcell ("FILETYPE", 0, "sky")
    pt.taql ("update '" + obstab.name() + "' set FILEDATE = mjd(date()), " +
             "RELEASE_DATE = mjd(date()+365)")
    # Determine minimum and maximum baseline length
    mstab = pt.table(msName, ack=False)
    if minbl <= 0:
        mbl = pt.taql ("calc sqrt(min([select sumsqr(UVW[:2]) from " + msName + "]))")
        minbl = max(mbl[0], abs(minbl))
    if maxbl <= 0:
        mbl = pt.taql ("calc sqrt(max([select sumsqr(UVW[:2]) from " + msName + "]))")
        if maxbl == 0:
            maxbl = mbl[0]
        else:
            maxbl = min(mbl[0], abs(maxbl))
    mstab.close()
    # Add and fill a few extra columns.
    col1 = pt.makescacoldesc ("MIN_BASELINE_LENGTH", 0, valuetype='double')
    col2 = pt.makescacoldesc ("MAX_BASELINE_LENGTH", 0, valuetype='double')
    col3 = pt.makearrcoldesc ("NVIS_USED", 0, valuetype='int')
    col4 = pt.makearrcoldesc ("NVIS_TOTAL", 0, valuetype='int')
    obstab.addcols (pt.maketabdesc ([col1, col2, col3, col4]))
    obstab.putcolkeyword ("MIN_BASELINE_LENGTH", "QuantumUnits", ["m"])
    obstab.putcolkeyword ("MAX_BASELINE_LENGTH", "QuantumUnits", ["m"])
    obstab.putcell ("MIN_BASELINE_LENGTH", 0, minbl)
    obstab.putcell ("MAX_BASELINE_LENGTH", 0, maxbl)
    # Get sum for all MSs.
    tusedCounts = usedCounts.sum (axis=0)
    tvisCounts  =  visCounts.sum (axis=0)
    obstab.putcell ("NVIS_USED", 0, tusedCounts)
    obstab.putcell ("NVIS_TOTAL", 0, tvisCounts)
    obstab.close()
    oritab.close()
    print "Updated subtable LOFAR_OBSERVATION"

""" Count number of unflagged visibilities per MS """
def countVisTime (msNames, taqlStr, baselineStr, minbl, maxbl):
    print "Counting visibility flags ..."
    t = pt.table(msNames[0] + '/ANTENNA', ack=False)
    nant = t.nrows();
    t.close()
    visCounts  = np.zeros ((len(msNames), nant, nant), 'int');
    usedCounts = np.zeros ((len(msNames), nant, nant), 'int');
    minTime    = +1e30
    maxTime    = -1e30
    for j in range(len(msNames)):
        # If baseline selection is done, use msselect to apply it.
        msname = msNames[j];
        if len(baselineStr) > 0:
            msname = msNames[j] + '.sel_addimginfo'
            os.system ("msselect 'in=" + msNames[j] + "' 'out=" + msname +
                       "' 'baseline=" + baselineStr + "'")
        # Skip auto-correlations and apply possible TaQL selection
        whereStr = ' where ANTENNA1!=ANTENNA2'
        if len(taqlStr) > 0:
            whereStr += ' && (' + taqlStr + ')'
        if minbl > 0:
            whereStr += ' && sumsqr(UVW[:2]) >= sqr(' + str(minbl) + ')'
        if maxbl > 0:
            whereStr += ' && sumsqr(UVW[:2]) <= sqr(' + str(maxbl) + ')'
        t = pt.taql ('select TIME-0.5*INTERVAL as STIME, TIME+0.5*INTERVAL as ETIME, ANTENNA1,ANTENNA2,nfalse(FLAG) as NUSED,count(FLAG) as NVIS from ' +
                     msname + whereStr + ' giving as memory')
        ant1  = t.getcol ('ANTENNA1')
        ant2  = t.getcol ('ANTENNA2')
        nused = t.getcol ('NUSED')
        nvis  = t.getcol ('NVIS')
        # Count number of used visibilities per antenna per MS
        for i in range(len(ant1)):
            usedCounts[j, ant1[i], ant2[i]] += nused[i]
            usedCounts[j, ant2[i], ant1[i]] += nused[i]
            visCounts [j, ant1[i], ant2[i]] += nvis[i]
            visCounts [j, ant2[i], ant1[i]] += nvis[i]
        minTime = min(minTime, t.getcol('STIME').min())
        maxTime = max(maxTime, t.getcol('ETIME').max())
        t.close()
        if msname != msNames[j]:
            os.system ('rm -rf ' + msname)
    return (usedCounts, visCounts, minTime, maxTime)


""" Add all imaging info

It creates several subtables in the MS containing the attributes.
They contain meta info telling where the image is made from.

imageName      Name of the image to add the info to.
msNames        Names of the MSs the image was made of.
sourcedbName   Name of the SourceDB containing the sources in the image.
               It will be used to create the LOFAR_SOURCE subtable from.
               If the name is empty, no LOFAR_SOURCE subtable will be created.
minbl          Minimum baseline length used. If 0, it will be calculated.
maxbl          Maximum baseline length used. If 0, it will be calculated.
taqlStr        TaQL string used in imager to limit the MS data used.
               It will always add ANTENNA1!=ANTENNA2 (thus only cross-corr).
baseline       Baseline selection string (in CASA syntax) used in imager

"""
def addImagingInfo (imageName, msNames, sourcedbName="", minbl=0., maxbl=0.,
                    taqlStr="", baseline=""):
    image = pt.table (imageName, readonly=False, ack=False)
    # Check if ATTRGROUPS already exists.
    # If not, add it as an empty dict.
    if "ATTRGROUPS" in image.fieldnames():
        raise Exception("addImagingInfo already done (keyword ATTRGROUPS already exists)")
    image.putkeyword ("ATTRGROUPS", {})
    # Find the number of unflagged visibilities per antenna per MS.
    (usedCounts,visCounts,minTime,maxTime) = countVisTime (msNames, taqlStr, baseline, minbl, maxbl)
    # Add all subtables while removing obsolete columns.
    addSubTable (image, msNames[0], "POINTING")
    addSubTable (image, msNames[0], "FIELD")
    addSubTable (image, msNames[0], "ANTENNA")
    addSubTable (image, msNames[0], "LOFAR_STATION")
    addSubTable (image, msNames[0], "HISTORY")
    addSubTable (image, msNames[0], "OBSERVATION",
                 ["LOG", "SCHEDULE_TYPE", "SCHEDULE"])
    # Create the LOFAR_QUALITY subtable.
    addQualityTable (image, usedCounts, visCounts)
    # Create the LOFAR_ORIGIN subtable from all MSs.
    addOriginTable (image, msNames)
    # Update times/frequencies/etc. in the LOFAR_OBSERVATION table.
    updateObsTable (image, msNames[0], minbl, maxbl,
                    usedCounts, visCounts, minTime, maxTime)
    # If needed, add the LOFAR_SOURCE table.
    if len(sourcedbName) > 0:
        addSourceTable (image, sourcedbName, minTime, maxTime)
    # Flush and close the image.
    image.close()
