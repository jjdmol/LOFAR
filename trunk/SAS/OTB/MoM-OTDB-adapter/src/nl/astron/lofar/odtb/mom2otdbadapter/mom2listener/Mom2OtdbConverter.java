package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * Converts mom2 values to otdb values
 * 
 * @author Bastiaan Verhoef
 *
 */
public class Mom2OtdbConverter {
	
	private static final String CLOCK_MODE = "<<ClockMode";
	private static final Pattern CLOCK_PATTERN = Pattern.compile("(\\d+) MHz");
	private static final Pattern INSTRUMENT_FILTER_PATTERN = Pattern.compile("(\\d+-\\d+) MHz");
	
	public static String getOTDBClockMode(String clock){
		Matcher clockMatcher = CLOCK_PATTERN.matcher(clock);
		clockMatcher.find();
		return CLOCK_MODE + clockMatcher.group(1);

	}
	public static String getOTDBAntennaArray(String antenna){
		return antenna.substring(0,3).toUpperCase();
	}	
	public static String getOTDBAntennaSet(String antenna){
		return antenna.toUpperCase().replace(' ','_');
	}
	public static String getOTDBBandFilter(String instrumentFilter, String antennaArray){
		String result = antennaArray + "_";
		Matcher filterMatcher = INSTRUMENT_FILTER_PATTERN.matcher(instrumentFilter);
		filterMatcher.find();
		return result + filterMatcher.group(1).replace('-', '_');

	}


	/**
	 * Convert OTDB status from mom status
	 * @param status Mom2 status
	 * @return OTDB status
	 */
	public static String getOTDBStatus(String status){
		if (status.equals("described")){
			return "being specified";
		}else if (status.equals("specified")){
			return "approved";
		}
		return null;
	}

}
