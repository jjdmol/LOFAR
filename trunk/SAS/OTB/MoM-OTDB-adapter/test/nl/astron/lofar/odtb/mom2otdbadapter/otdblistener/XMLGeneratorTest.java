package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Beam;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.util.AstronConverter;

import org.junit.Test;

public class XMLGeneratorTest {

	@Test
	public void testGetObservationXml() throws IOException, ParserConfigurationException {
	
		LofarObservation lofarObservation = new LofarObservation();
		lofarObservation.setMom2Id(3);

		lofarObservation.setAntennaSet("HBA_TWO");
		lofarObservation.setBandFilter("HBA_110_190");
		lofarObservation.setClockMode("<<Clock200");
		lofarObservation.setStations("[CS302, CS010]");
		lofarObservation.setStatus("finished");
		lofarObservation.setObservationId(15);	
		lofarObservation.setStartTime(AstronConverter.toDate("2009/06/24 13:00 UTC"));
		lofarObservation.setEndTime(AstronConverter.toDate("2009/06/24 13:30 UTC"));
		lofarObservation.setClockFrequency(200000000d);
		lofarObservation.setSubbandWidth(195312.5d);
		lofarObservation.setChannelWidth(762.93945d);
		lofarObservation.setChannelsPerSubband(256);
		lofarObservation.setSamplesPerSecond(196608);
		lofarObservation.setFileNameMask("/data/L${YEAR}_${MSNUMBER}/L${OBSERVATION}_B${BEAM}_SB${SUBBAND}.MS" );
		Beam beam = new Beam(lofarObservation);
		beam.setMom2Id(4);
		beam.setRa(85.650575);
		beam.setDec(49.852009);
		beam.setEquinox("J2000");
		beam.setDuration(4230);
		beam.setSubbands("[123,124]");
		lofarObservation.getBeams().add(beam);
		beam = new Beam(lofarObservation);
		beam.setMom2Id(5);
		beam.setRa(85.650575);
		beam.setDec(49.852009);
		beam.setEquinox("J2000");
		beam.setDuration(420);
		beam.setSubbands("[1,2,3,4]");
		lofarObservation.getBeams().add(beam);
		Mom2Configuration config = new Mom2Configuration();
		config.setMom2SchemasUrl("C:/java/workspace/MoM-OTDB-adapter/schemas/");
		String xml = XMLGenerator.getObservationXml(lofarObservation, config);
		File file = new File("examples/obs.xml");
		PrintWriter writer = new PrintWriter(file);
		writer.print(xml);
		writer.flush();
		writer.close();
	}

}
