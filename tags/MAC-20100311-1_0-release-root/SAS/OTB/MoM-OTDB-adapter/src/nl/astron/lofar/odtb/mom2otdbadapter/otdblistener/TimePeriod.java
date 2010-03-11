package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.Serializable;
import java.util.Date;

/**
 * Time period
 * @author Bastiaan Verhoef
 *
 */
public class TimePeriod implements Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = -8282521565309700261L;
	protected Date startTime = null;
	protected Date endTime = null;
	public Date getEndTime() {
		return endTime;
	}
	public void setEndTime(Date endTime) {
		this.endTime = endTime;
	}
	public Date getStartTime() {
		return startTime;
	}
	public void setStartTime(Date startTime) {
		this.startTime = startTime;
	}
}
