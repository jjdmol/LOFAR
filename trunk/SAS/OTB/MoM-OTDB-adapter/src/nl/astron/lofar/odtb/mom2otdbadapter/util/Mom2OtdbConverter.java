package nl.astron.lofar.odtb.mom2otdbadapter.util;

import java.text.DecimalFormat;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Converts mom2 values to otdb values
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class Mom2OtdbConverter {

	private static final String FITS = "FITS";
	private static final String HDF5 = "HDF5";
	private static final String AIPS_CASA = "AIPS++/CASA";
	private static final String FITS_EXTENSION = ".fits";
	private static final String H5_EXTENSION = ".h5";
	private static final String MS_EXTENSION = ".MS";
	public static final String OTDB_APPROVED_STATUS = "approved";
	public static final String OTDB_BEING_SPECIFIED_STATUS = "being specified";
	public static final String MOM2_DESCRIBED_STATUS = "described";
	public static final String MOM2_SPECIFIED_STATUS = "specified";
	public static final String MOM2_ACTIVE_STATUS = "active";
	public static final String MOM2_PREPARED_STATUS = "prepared";
	public static final String MOM2_FAILED_STATUS = "failed";
	public static final String MOM2_ABORTED_STATUS = "aborted";
	public static final String MOM2_SUCCESSFUL_STATUS = "successful";
	public static final String OTDB_FAILED_STATUS = "failed";
	public static final String OTDB_ABORTED_STATUS = "aborted";
	public static final String OTDB_FINISHED_STATUS = "finished";
	public static final String OTDB_ACTIVE_STATUS = "active";
	public static final String OTDB_SPECIFIED_STATUS = "specified";
	private static final String MHZ_SUFFIX = " MHz";
	private static final String CLOCK_MODE = "<<Clock";
	private static final DecimalFormat CLOCK_FORMATTER = new DecimalFormat("000");
	private static final Pattern INSTRUMENT_FILTER_PATTERN = Pattern.compile("(\\d+-\\d+)" + MHZ_SUFFIX);

	public static String getOTDBClockMode(Double clock) {

		return CLOCK_MODE + CLOCK_FORMATTER.format(clock);

	}

	public static String getOTDBAntennaArray(String antenna) {
		return antenna.substring(0, 3).toUpperCase();
	}

	public static String getOTDBAntennaSet(String antenna) {
		return antenna.toUpperCase().replace(' ', '_');
	}

	public static String getOTDBBandFilter(String instrumentFilter, String antennaArray) {
		String result = antennaArray + "_";
		Matcher filterMatcher = INSTRUMENT_FILTER_PATTERN.matcher(instrumentFilter);
		filterMatcher.find();
		return result + filterMatcher.group(1).replace('-', '_');

	}

	/**
	 * Convert OTDB status from mom status
	 * 
	 * @param status
	 *            Mom2 status
	 * @return OTDB status
	 */
	public static String getOTDBStatus(String status) {
		if (status.equals(MOM2_DESCRIBED_STATUS)) {
			return OTDB_BEING_SPECIFIED_STATUS;
		} else if (status.equals(MOM2_SPECIFIED_STATUS)) {
			return OTDB_APPROVED_STATUS;
		}
		return null;
	}

	public static String getMom2Antenna(String antennaSet) {
		if (antennaSet != null) {
			String result = antennaSet.replace('_', ' ');
			int n = result.indexOf(' ');
			n = n + 2;
			result = result.substring(0, n) + result.substring(n, result.length()).toLowerCase();
			return result;
		}
		return null;
	}

	public static String getMom2InstrumentFilter(String bandFilter) {
		if (bandFilter != null) {
			return bandFilter.substring(4).replace('_', '-') + MHZ_SUFFIX;
		}
		return null;
	}

	public static String[] getStringArray(String string) {
		String temp = string.replaceAll("[\\]\\[ ]", "");
		return temp.split(",");
	}

	public static String getMom2Subbands(List<Integer> subbands) {
		String result = "";
		for (int i = 0; i < subbands.size(); i++) {
			if (i > 0) {
				result += ",";
			}
			result += subbands.get(i);
		}
		return result;
	}

	public static String getMom2DPFileType(String fileMask) {
		if (fileMask.endsWith(MS_EXTENSION)) {
			return AIPS_CASA;
		} else if (fileMask.endsWith(H5_EXTENSION)) {
			return HDF5;
		} else if (fileMask.endsWith(FITS_EXTENSION)) {
			return FITS;
		}
		return null;
	}

	public static String getMom2Status(String code) {
		if (code.equals(OTDB_APPROVED_STATUS)) {
			return MOM2_SPECIFIED_STATUS;
		}
		if (code.equals(OTDB_SPECIFIED_STATUS)) {
			return MOM2_PREPARED_STATUS;
		}
		if (code.equals(OTDB_ACTIVE_STATUS)) {
			return MOM2_ACTIVE_STATUS;
		}
		if (code.equals(OTDB_FINISHED_STATUS)) {
			return MOM2_SUCCESSFUL_STATUS;
		}
		if (code.equals(OTDB_ABORTED_STATUS)) {
			return MOM2_ABORTED_STATUS;
		}
		if (code.equals(OTDB_FAILED_STATUS)) {
			return MOM2_FAILED_STATUS;
		}
		return null;
	}

}
