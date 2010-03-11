package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import nl.astron.lofar.odtb.mom2otdbadapter.util.Mom2OtdbConverter;

import org.junit.Assert;
import org.junit.Test;

public class Mom2OtdbConverterTest {


	//LBA_INNER|LBA_OUTER|LBA_SPARSE|LBA_X|LBA_Y|HBA_ONE|HBA_TWO|HBA_BOTH;-
	@Test
	public void testGetOTDBClockMode(){
		Assert.assertEquals("<<Clock200", Mom2OtdbConverter.getOTDBClockMode(200.00d));
		Assert.assertEquals("<<Clock160", Mom2OtdbConverter.getOTDBClockMode(160.0d));

	}
	
	@Test
	public void testGetOTDBStatus() {
		Assert.assertEquals("being specified", Mom2OtdbConverter.getOTDBStatus("described"));
		Assert.assertEquals("approved", Mom2OtdbConverter.getOTDBStatus("specified"));
	}
	
	@Test
	public void getOTDBAntennaArray(){
		Assert.assertEquals("HBA", Mom2OtdbConverter.getOTDBAntennaArray("HBA One"));
		Assert.assertEquals("HBA", Mom2OtdbConverter.getOTDBAntennaArray("HBA Two"));
		Assert.assertEquals("HBA", Mom2OtdbConverter.getOTDBAntennaArray("HBA Both"));
		Assert.assertEquals("LBA", Mom2OtdbConverter.getOTDBAntennaArray("LBA Outer"));
		Assert.assertEquals("LBA", Mom2OtdbConverter.getOTDBAntennaArray("LBA Inner"));
		Assert.assertEquals("LBA", Mom2OtdbConverter.getOTDBAntennaArray("LBA Sparse"));
		Assert.assertEquals("LBA", Mom2OtdbConverter.getOTDBAntennaArray("LBA X"));
		Assert.assertEquals("LBA", Mom2OtdbConverter.getOTDBAntennaArray("LBA Y"));
		
		}
	@Test
	public void getOTDBAntennaSet(){
		Assert.assertEquals("HBA_ONE", Mom2OtdbConverter.getOTDBAntennaSet("HBA One"));
		Assert.assertEquals("HBA_TWO", Mom2OtdbConverter.getOTDBAntennaSet("HBA Two"));
		Assert.assertEquals("HBA_BOTH", Mom2OtdbConverter.getOTDBAntennaSet("HBA Both"));
		Assert.assertEquals("LBA_OUTER", Mom2OtdbConverter.getOTDBAntennaSet("LBA Outer"));
		Assert.assertEquals("LBA_INNER", Mom2OtdbConverter.getOTDBAntennaSet("LBA Inner"));
		Assert.assertEquals("LBA_SPARSE", Mom2OtdbConverter.getOTDBAntennaSet("LBA Sparse"));
		Assert.assertEquals("LBA_X", Mom2OtdbConverter.getOTDBAntennaSet("LBA X"));
		Assert.assertEquals("LBA_Y", Mom2OtdbConverter.getOTDBAntennaSet("LBA Y"));
	}
	@Test
	public void getOTDBBandFilter(){
		Assert.assertEquals("LBA_10_90", Mom2OtdbConverter.getOTDBBandFilter("10-90 MHz","LBA"));
		Assert.assertEquals("LBA_30_80", Mom2OtdbConverter.getOTDBBandFilter("30-80 MHz","LBA"));
		Assert.assertEquals("HBA_110_190", Mom2OtdbConverter.getOTDBBandFilter("110-190 MHz","HBA"));
		Assert.assertEquals("HBA_170_230", Mom2OtdbConverter.getOTDBBandFilter("170-230 MHz","HBA"));
		Assert.assertEquals("HBA_210_250", Mom2OtdbConverter.getOTDBBandFilter("210-250 MHz","HBA"));

	}
	@Test
	public void getMom2Antenna(){
		Assert.assertEquals("HBA One", Mom2OtdbConverter.getMom2Antenna("HBA_ONE"));
		Assert.assertEquals("HBA Two", Mom2OtdbConverter.getMom2Antenna("HBA_TWO"));
		Assert.assertEquals("HBA Both", Mom2OtdbConverter.getMom2Antenna("HBA_BOTH"));
		Assert.assertEquals("LBA Outer", Mom2OtdbConverter.getMom2Antenna("LBA_OUTER"));
		Assert.assertEquals("LBA Inner", Mom2OtdbConverter.getMom2Antenna("LBA_INNER"));
		Assert.assertEquals("LBA Sparse", Mom2OtdbConverter.getMom2Antenna("LBA_SPARSE"));
		Assert.assertEquals("LBA X", Mom2OtdbConverter.getMom2Antenna("LBA_X"));
		Assert.assertEquals("LBA Y", Mom2OtdbConverter.getMom2Antenna("LBA_Y"));
	}
	
	@Test
	public void getMom2InstrumentFilter(){
		Assert.assertEquals("10-90 MHz", Mom2OtdbConverter.getMom2InstrumentFilter("LBA_10_90"));
		Assert.assertEquals("30-80 MHz", Mom2OtdbConverter.getMom2InstrumentFilter("LBA_30_80"));
		Assert.assertEquals("110-190 MHz", Mom2OtdbConverter.getMom2InstrumentFilter("HBA_110_190"));
		Assert.assertEquals("170-230 MHz", Mom2OtdbConverter.getMom2InstrumentFilter("HBA_170_230"));
		Assert.assertEquals("210-250 MHz", Mom2OtdbConverter.getMom2InstrumentFilter("HBA_210_250"));

	}
}
