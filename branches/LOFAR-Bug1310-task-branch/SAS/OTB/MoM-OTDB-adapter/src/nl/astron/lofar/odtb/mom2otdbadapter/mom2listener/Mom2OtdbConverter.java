package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import nl.astron.wsrt.util.WsrtConverter;
import nl.astron.wsrt.util.WsrtValidator;

/**
 * Converts mom2 values to otdb values
 * 
 * @author Bastiaan Verhoef
 *
 */
public class Mom2OtdbConverter {
	private static final double MAX_BANDWIDTH = 32000000;
	private static final int TOTAL_SUBBANDS = 512;
	/**
	 * Calculate an array of subbands
	 * 
	 * @param samplingFrequency sampling frequency in Hz
	 * @param numberOfBands number of bands
	 * @param subbandPlacement kind of subband placement (e.g. contiguous, scattered) 
	 * @param startFrequency start frequency in Hz
	 * @param spacing spacing in bands
	 * @return subbands e.g. [12,34]
	 */
	public static String getOTDBSubbands(Integer samplingFrequency, Integer numberOfBands,
			String subbandPlacement, Integer startFrequency, Integer spacing) {
		if (samplingFrequency == null 
				|| numberOfBands == null
				|| WsrtValidator.isBlankOrNull(subbandPlacement)
				|| startFrequency == null
				|| spacing == null){
			return null;
		}
		String subbandPlacementLowerCase = subbandPlacement.toLowerCase();
		/*
		 * if subband placement is special, no calculation can be made
		 */
		if (subbandPlacementLowerCase.equals("special")){
			return null;
		}
		/*
		 * if contiguous or scattered, calculate subbands
		 */
		else if(subbandPlacementLowerCase.equals("contiguous") ||
				subbandPlacementLowerCase.equals("scattered")){
			double samplingFreq = samplingFrequency.doubleValue();
			double startFreq = startFrequency.doubleValue();
			int nBands = numberOfBands.intValue();
			int spac = 0;
			if (subbandPlacementLowerCase.equals("scattered")){
				spac = spacing.intValue();
			}
			/*
			 * calculate subband width
			 */
			double subbandWidth = samplingFreq /(2*TOTAL_SUBBANDS);
			/*
			 * calculate firstband
			 */
			double firstBand = startFreq/subbandWidth;
			firstBand = Math.floor(firstBand);

			/*
			 * calculate max number of bands
			 */
			double maxBands = MAX_BANDWIDTH/subbandWidth;
			maxBands = Math.floor(maxBands);
			/*
			 * check if bands exceeds max number of bands
			 */
			if (nBands > maxBands){
				nBands = (int) maxBands;
			}
			int currentSubband = (int) firstBand;
			String result ="[" ;
			for (int i = 0; i < nBands && currentSubband < TOTAL_SUBBANDS;i++){
				result+=currentSubband;
				if (i < nBands-1 && currentSubband < TOTAL_SUBBANDS-(1 + spac)){
					result += ",";
				}
				currentSubband = currentSubband + spac + 1;
			}
			result += "]";
			return result;
			
			
		}
		return null;
	}
	/**
	 * Calculate bandselection from filter
	 * @param filter
	 * @return band selecitons
	 */
	public static String getOTDBBandSelection(String filter){

		/*
		 * check if filter is not null
		 */

		if (!WsrtValidator.isBlankOrNull(filter)) {
			/*
			 * filter looks like 10-70 (160 MHz sampling rate) split it into:
			 * '10-70 ' '160 MHz sampling rate)'
			 */
			String[] splitted = filter.split("\\(");
			/*
			 * get '10-70'
			 */
			String band = splitted[0].trim();
			/*
			 * split it into
			 * '10' '70'
			 */
			String[] frequencies = band.split("[- ]");
			/*
			 * get '10'
			 */
			String startFrequency = frequencies[0];
			String endFrequency = frequencies[1];
			int startFreq = 0;
			int endFreq = 0;
			if (WsrtValidator.isPositiveInt(startFrequency)){
	
				startFreq = WsrtConverter.toInt(WsrtConverter.toInteger(startFrequency));
			}
			if (WsrtValidator.isPositiveInt(endFrequency)){
				
				endFreq = WsrtConverter.toInt(WsrtConverter.toInteger(endFrequency));
			}
			if (startFreq >= 10 && endFreq <= 90){
				return "LB_10_90";
			}else if (startFreq >= 110 && endFreq <= 190){
				return "HB_110_190";
			}else if (startFreq >= 210 && endFreq <= 250){
				return "HB_210_250";
			}else if (startFreq >= 170 && endFreq <= 230){
				return "HB_170_230";
			}

			
		}
		return null;
	}

	/**
	 * Calculate sampling frequency
	 * @param filter
	 * @return sampling frequency in Hz
	 */
	public static Integer getOTDBSamplingFrequency(String filter) {
		/*
		 * check if filter is not null
		 */

		if (!WsrtValidator.isBlankOrNull(filter)) {
			/*
			 * filter looks like 10-70 (160 MHz sampling rate) split it into:
			 * '10-70 ' '160 MHz sampling rate)'
			 */
			String[] splitted = filter.split("\\(");
			/*
			 * get '160 MHz sampling rate)'
			 */
			String samplingRate = splitted[1].trim();
			/*
			 * split it by ' ' result is '160' 'MHz' 'sampling' 'rate)'
			 */
			splitted = samplingRate.split(" ");
			/*
			 * get '160'
			 */
			String samplingFrequency = splitted[0];
			/*
			 * if it is a positive int
			 */
			if (WsrtValidator.isPositiveInt(samplingFrequency)) {
				/*
				 * convert it to an integer
				 */
				int number = WsrtConverter.toInteger(samplingFrequency)
						.intValue();
				/*
				 * convert it from MHz to Hz
				 */
				return new Integer(number * 1000000);
			}
		}
		return null;
	}
	/**
	 * Calculate frequency from mom frequency
	 * @param frequency Mom2 frequency
	 * @return OTDB frequency
	 */
	public static Integer getOTDBFrequency(String frequency){
		Double freq = WsrtConverter.toDouble(frequency);
		return new Integer((int)freq.doubleValue()*1000000);
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
