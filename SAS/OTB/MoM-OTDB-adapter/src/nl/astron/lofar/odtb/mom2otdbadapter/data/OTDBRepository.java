package nl.astron.lofar.odtb.mom2otdbadapter.data;



public class OTDBRepository {

	public void story(LofarObservation lofarObservation){
		
	}

	public LofarObservation getLatestChanges(String date){
		LofarObservation observation = new LofarObservation();
		observation.setMom2Id("15");
		observation.setAngleTimes("[+0,+30,+60]");
		observation.setStatus("specified");
		observation.setMeasurementMom2Ids("[16,17,18]");
		observation.setStartTime("16-12-2005 12:00:00");
		observation.setEndTime("16-12-2005 13:00:00");
		return observation;
	}

}
