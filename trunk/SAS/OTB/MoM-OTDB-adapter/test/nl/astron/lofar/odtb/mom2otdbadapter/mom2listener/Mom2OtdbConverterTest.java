package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import org.junit.Assert;
import org.junit.Test;

public class Mom2OtdbConverterTest {


	@Test
	public void testGetOTDBStatus() {
		String status = Mom2OtdbConverter.getOTDBStatus("described");
		Assert.assertEquals("being specified", status);
		status = Mom2OtdbConverter.getOTDBStatus("specified");
		Assert.assertEquals("approved", status);
	}
}
