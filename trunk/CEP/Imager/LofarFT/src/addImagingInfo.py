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
def addQualityTable (image):
    # Create the table using TaQL.
    tab = pt.taql ("create table '" + image.name() + "/LOFAR_QUALITY' " + 
                   "QUALITY_MEASURE string, VALUE string, FLAG_ROW bool")
    # Get the rms noise of I,Q,U,V as list of tuples.
    noises = grn.get_rms_noise (image.name())
    for noise in noises:
        row = tab.nrows()
        tab.addrows (1)
        tab.putcell ("QUALITY_MEASURE", row, "RMS_NOISE_"+noise[0])
        tab.putcell ("VALUE", row, str(noise[1]))
        tab.putcell ("FLAG_ROW", row, False)
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
        w = subtab.getcell("FREQUENCY_MAX",i) - subtab.getcell("FREQUENCY_MIN",i)
        subtab.putcell ("CHANNEL_WIDTH", i, w * 1e6 / numchan)
        # Determine the averaging factors.
        avgfreq = 1
        avgtime = 1
        if ("LOFAR_FULL_RES_FLAG" in t.colnames()):
            avgfreq = t.getcolkeyword ("LOFAR_FULL_RES_FLAG", "NCHAN_AVG")
            avgtime = t.getcolkeyword ("LOFAR_FULL_RES_FLAG", "NTIME_AVG")
        subtab.putcell ("NCHAN_AVG", i, avgfreq)
        subtab.putcell ("NTIME_AVG", i, avgtime)
        t1.close()
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
def addSourceTable (image, sourcedbName, times):
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
    midtime = (times[0] + times[1]) / 2
    inttime = times[1] - times[0]
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
def updateObsTable (image, msName, minbl, maxbl):
    obstab = pt.table (image.name() + "/LOFAR_OBSERVATION", readonly=False,
                       ack=False)
    oritab = pt.table (image.name() + "/LOFAR_ORIGIN", ack=False)
    minfreq = pt.taql ("calc min([select FREQUENCY_MIN from '" +
                       oritab.name() + "'])")
    maxfreq = pt.taql ("calc max([select FREQUENCY_MAX from '" +
                       oritab.name() + "'])")
    mintime = pt.taql ("calc min([select START from '" +
                       oritab.name() + "'])")
    maxtime = pt.taql ("calc max([select END from '" +
                       oritab.name() + "'])")
    obstab.putcell ("OBSERVATION_FREQUENCY_MIN", 0, minfreq[0]);
    obstab.putcell ("OBSERVATION_FREQUENCY_MAX", 0, maxfreq[0]);
    obstab.putcell ("OBSERVATION_FREQUENCY_CENTER", 0, (maxfreq[0]-minfreq[0])/2);
    obstab.putcell ("OBSERVATION_START", 0, mintime[0]);
    obstab.putcell ("OBSERVATION_END", 0, maxtime[0]);
    obstab.putcell ("TIME_RANGE", 0, (mintime[0], maxtime[0]));
    obstab.putcell ("FILENAME", 0, os.path.basename(image.name()))
    obstab.putcell ("FILETYPE", 0, "sky")
    pt.taql ("update '" + obstab.name() + "' set FILEDATE = mjd(date()), " +
             "RELEASE_DATE = mjd(date()+365)")
    # Determine minimum and maximum baseline length
    mstab = pt.table(msName)
    if minbl <= 0:
        mbl = pt.taql ("calc min([select sumsqr(UVW[:2]) from " + msName + "])")
        minbl = max(mbl[0], abs(minbl))
    if maxbl <= 0:
        mbl = pt.taql ("calc max([select sumsqr(UVW[:2]) from " + msName + "])")
        if maxbl == 0:
            maxbl = mbl[0]
        else:
            maxbl = min(mbl[0], abs(maxbl))
    mstab.close()
    # Add and fill a few extra columns.
    col1 = pt.makescacoldesc ("MIN_BASELINE_LENGTH", 0, valuetype='double')
    col2 = pt.makescacoldesc ("MAX_BASELINE_LENGTH", 0, valuetype='double')
    obstab.addcols (pt.maketabdesc ([col1, col2]))
    obstab.putcolkeyword ("MIN_BASELINE_LENGTH", "QuantumUnits", ["m"])
    obstab.putcolkeyword ("MAX_BASELINE_LENGTH", "QuantumUnits", ["m"])
    obstab.putcell ("MIN_BASELINE_LENGTH", 0, minbl)
    obstab.putcell ("MAX_BASELINE_LENGTH", 0, maxbl)
    obstab.close()
    oritab.close()
    print "Updated subtable LOFAR_OBSERVATION"
    return (mintime[0], maxtime[0])

""" Add all imaging info """
def addImagingInfo (imageName, msNames, sourcedbName, minbl=0., maxbl=0.):
    image = pt.table (imageName, readonly=False, ack=False)
    # Check if ATTRGROUPS already exists.
    # If not, add it as an empty dict.
    if "ATTRGROUPS" in image.fieldnames():
        raise Exception("addImagingInfo already done (keyword ATTRGROUPS already exists)")
    image.putkeyword ("ATTRGROUPS", {})
    # Add all subtables while removing obsolete columns
    addSubTable (image, msNames[0], "POINTING")
    addSubTable (image, msNames[0], "FIELD")
    addSubTable (image, msNames[0], "ANTENNA")
    addSubTable (image, msNames[0], "LOFAR_STATION")
    addSubTable (image, msNames[0], "HISTORY")
    addSubTable (image, msNames[0], "OBSERVATION",
                 ["LOG", "SCHEDULE_TYPE", "SCHEDULE"])
    # Create the (empty) LOFAR_QUALITY subtable.
    addQualityTable (image)
    # Create the LOFAR_ORIGIN subtable from all MSs.
    addOriginTable (image, msNames)
    # Update times/frequencies/etc. in the LOFAR_OBSERVATION table.
    times = updateObsTable (image, msNames[0], minbl, maxbl)
    # Add the LOFAR_SOURCE table.
    addSourceTable (image, sourcedbName, times)
    # Flush and close the image.
    image.close()
