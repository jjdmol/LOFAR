package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import junit.framework.TestCase;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.Mom2OtdbConverter;

public class TestMom2OtdbConverter extends TestCase {

	public TestMom2OtdbConverter(String name) {
		super(name);
	}

	protected void setUp() throws Exception {
		super.setUp();
	}

	protected void tearDown() throws Exception {
		super.tearDown();
	}

	/*
	 * Test method for
	 * 'nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.Mom2OtdbConverter.getOTDBSubbands(String,
	 * String, String, String, String)'
	 */
	public void testGetOTDBContiguousSubbands() {
		String subbands = Mom2OtdbConverter.getOTDBSubbands(new Integer(
				160000000), new Integer(50), "Contiguous",
				new Integer(79000000), new Integer(0));
		assertEquals("[505,506,507,508,509,510,511]", subbands);
	}

	public void testGetOTDBScatteredSubbands() {
		String subbands = Mom2OtdbConverter.getOTDBSubbands(new Integer(
				160000000), new Integer(50), "Scattered",
				new Integer(79000000), new Integer(1));
		assertEquals("[505,507,509,511]", subbands);
	}

	public void testGetOTDBMaxSubbands() {
		String subbands = Mom2OtdbConverter.getOTDBSubbands(new Integer(
				160000000), new Integer(500), "Scattered",
				new Integer(1000000), new Integer(1));
		assertEquals("[6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,"
				+ "40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,"
				+ "74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,"
				+ "106,108,110,112,114,116,118,120,122,124,126,128,130,"
				+ "132,134,136,138,140,142,144,146,148,150,152,154,156,"
				+ "158,160,162,164,166,168,170,172,174,176,178,180,182,"
				+ "184,186,188,190,192,194,196,198,200,202,204,206,208,"
				+ "210,212,214,216,218,220,222,224,226,228,230,232,234,"
				+ "236,238,240,242,244,246,248,250,252,254,256,258,260,"
				+ "262,264,266,268,270,272,274,276,278,280,282,284,286,"
				+ "288,290,292,294,296,298,300,302,304,306,308,310,312,"
				+ "314,316,318,320,322,324,326,328,330,332,334,336,338,"
				+ "340,342,344,346,348,350,352,354,356,358,360,362,364,"
				+ "366,368,370,372,374,376,378,380,382,384,386,388,390,"
				+ "392,394,396,398,400,402,404,406,408,410,412]", subbands);
	}

	/*
	 * Test method for
	 * 'nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.Mom2OtdbConverter.getOTDBBandSelection(String)'
	 */
	public void testGetOTDBBandSelection() {
		String bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("10-70 MHz (160 MHz samplingrate)");
		assertEquals("LB_10_90", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("10-80 MHz (200 MHz samplingrate)");
		assertEquals("LB_10_90", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("110-130 MHz (160 MHz samplingrate)");
		assertEquals("HB_110_190", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("110-190 MHz (200 MHz samplingrate)");
		assertEquals("HB_110_190", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("170-230 MHz (160 MHz samplingrate)");
		assertEquals("HB_170_230", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("210-230 MHz (160 MHz samplingrate)");
		assertEquals("HB_210_250", bandSelection);
		bandSelection = Mom2OtdbConverter
				.getOTDBBandSelection("210-250 MHz (200 MHz samplingrate)");
		assertEquals("HB_210_250", bandSelection);
	}

	public void testGetOTDBSamplingFrequency() {
		Integer lowBand = Mom2OtdbConverter
				.getOTDBSamplingFrequency("10-80 (200 MHz sampling rate)");
		assertEquals(new Integer(200000000), lowBand);
		lowBand = Mom2OtdbConverter
				.getOTDBSamplingFrequency("10-70 (160 MHz sampling rate)");
		assertEquals(new Integer(160000000), lowBand);
	}

	public void testGetOTDBFrequency() {
		Integer frequency = Mom2OtdbConverter.getOTDBFrequency("10.0");
		assertEquals(new Integer(10000000), frequency);
	}

	public void testGetOTDBStatus() {
		String status = Mom2OtdbConverter.getOTDBStatus("described");
		assertEquals("being specified", status);
		status = Mom2OtdbConverter.getOTDBStatus("specified");
		assertEquals("approved", status);
	}
}
