

import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.TestMom2OtdbConverter;
import junit.framework.Test;
import junit.framework.TestSuite;

public class AllTests {

	public static Test suite() {
		TestSuite suite = new TestSuite(
				"Test for nl.astron.lofar.odtb.mom2otdbadapter.test");
		//$JUnit-BEGIN$
		suite.addTestSuite(TestMom2OtdbConverter.class);
		//$JUnit-END$
		return suite;
	}

}
