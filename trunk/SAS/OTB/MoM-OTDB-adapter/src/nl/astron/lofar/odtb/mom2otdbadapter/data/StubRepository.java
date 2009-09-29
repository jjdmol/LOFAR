package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import nl.astron.lofar.odtb.mom2otdbadapter.util.Mom2OtdbConverter;

public class StubRepository implements Repository {

	private Map<Integer, LofarObservation> observationsMap = new HashMap<Integer, LofarObservation>();
	private int lastObservationId = 1;

	@Override
	public List<LofarObservation> getLatestChanges(Date startDate, Date endDate) throws RepositoryException {
		List<Integer> observationToRemoved = new ArrayList<Integer>();
		List<LofarObservation> results = new ArrayList<LofarObservation>();
		for (Map.Entry<Integer, LofarObservation> entry : observationsMap.entrySet()) {
			LofarObservation observation = entry.getValue();
			observation.setStatus(changeStatus(observation.getStatus()));
			observation.setTimeStamp(new Date());
			if (observation.getStatus() != null) {
				if (Mom2OtdbConverter.OTDB_ACTIVE_STATUS.equals(observation.getStatus())){
					observation.setStartTime(new Date());
				}else if (Mom2OtdbConverter.OTDB_FINISHED_STATUS.equals(observation.getStatus())){
					int duration = 0;
					for (Beam beam : observation.getBeams()) {
						duration += beam.getDuration();
					}
					Date endTime = (Date) observation.getStartTime().clone();
					endTime.setSeconds(duration);
					observation.setEndTime(endTime);					
				}

				if (observation.getClockMode().equals("<<Clock160")) {
					observation.setClockFrequency(200000000d);
					observation.setSubbandWidth(195312.5d);
					observation.setChannelWidth(762.939453125d);
					observation.setSamplesPerSecond(196608);
				} else if (observation.getClockMode().equals("<<Clock2000")) {
					observation.setClockFrequency(160000000d);
					observation.setSubbandWidth(156250d);
					observation.setChannelWidth(610.3515625d);
					observation.setSamplesPerSecond(155648);
				}
				observation.setChannelsPerSubband(256);
				observation.setFileNameMask("/data/L${YEAR}_${MSNUMBER}/L${OBSERVATION}_B${BEAM}_SB${SUBBAND}.MS");
				results.add(observation);
			}
			if (Mom2OtdbConverter.OTDB_FINISHED_STATUS.equals(observation.getStatus())){
				observationToRemoved.add(entry.getKey());
			}

			
		}
		for (Integer mom2Id: observationToRemoved){
			observationsMap.remove(mom2Id);
		}

		return results;
	}

	@Override
	public synchronized void store(LofarObservation lofarObservation) throws RepositoryException {
		if (observationsMap.containsKey(lofarObservation.getMom2Id())){
			LofarObservation obs = observationsMap.get(lofarObservation.getMom2Id());
			obs.setStatus(lofarObservation.getStatus());
		}else {
			lofarObservation.setObservationId(lastObservationId);
			lastObservationId++;
			observationsMap.put(lofarObservation.getMom2Id(),lofarObservation);
		}

	}

	public String changeStatus(String status) {
		if (Mom2OtdbConverter.OTDB_BEING_SPECIFIED_STATUS.equals(status)) {
			return Mom2OtdbConverter.OTDB_SPECIFIED_STATUS;
		}
		// if (Mom2OtdbConverter.OTDB_SPECIFIED_STATUS.equals(status)){
		// return Mom2OtdbConverter.OTDB_APPROVED_STATUS;
		// }
		if (Mom2OtdbConverter.OTDB_APPROVED_STATUS.equals(status)) {
			return Mom2OtdbConverter.OTDB_ACTIVE_STATUS;
		}
		if (Mom2OtdbConverter.OTDB_ACTIVE_STATUS.equals(status)) {
			return Mom2OtdbConverter.OTDB_FINISHED_STATUS;
		}
		return null;

	}
}
