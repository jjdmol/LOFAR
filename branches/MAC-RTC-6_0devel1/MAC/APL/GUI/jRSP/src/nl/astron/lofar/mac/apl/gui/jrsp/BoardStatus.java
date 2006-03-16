package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * This class keeps hold of the Board Status information. All member variables are public so there is no
 * need for getters and setters.
 * @author  balken
 */
public class BoardStatus
{
    /** Measured 1.2 V supply voltage. */
    public int voltage1V2;
    
    /** Measured 2.5 V supply voltage. */
    public int voltage2V5;
    
    /** Measured 3.3 V supply voltage. */
    public int voltage3V3;
    
    /** Current RSP board temperature. */
    public int pcbTemp;
    
    /** Current Board Processor temperature. */
    public int bpTemp;
    
    /** Current Antenna Processor 0 temperature. */
    public int ap0Temp;
    
    /** Current Antenna Processor 1 temperature. */
    public int ap1Temp;
    
    /** Current Antenna Processor 2 temperature. */
    public int ap2Temp;
    
    /** Current Antenna Processor 3 temperature. */
    public int ap3Temp;
    
    /** Current Board Processor system clock speed. */
    public int bpClock;
    
    /** Number of eth frames received. */
    public int nofFrames;
    
    /** Number of incorrect ethernet frames. */
    public int nofErrors;
    
    /** Error status of last received ethernet frame. */
    public int lastError;
    
    /** Sequence number of previous received message. */
    public int seqNr;
    
    /** Error status of previous received message. */
    public int error;
    
    /** Interface under test. */
    public int ifUnderTest;
    
    /** Test mode. */
    public int mode;
    
    /** Number of detected errors. */
    public int riErrors;
    
    /** Number of detected errors. */
    public int rcuxErrors;
    
    /** Number of detected errors. */
    public int lcuErrors;
    
    /** Number of detected errors. */
    public int cepErrors;
    
    /** Number of detected errors. */
    public int serdesErrors;
    
    /** Number of detected errors. */
    public int ap0RiErrors;
    
    /** Number of detected errors. */
    public int ap1RiErrors;
    
    /** Number of detected errors. */
    public int ap2RiErrors;
    
    /** Number of detected errors. */
    public int ap3RiErrors;
    
    /** Number of detected errors. */
    public int blp0Sync;
    
    /** Number of detected errors. */
    public int blp1Sync;
    
    /** Number of detected errors. */
    public int blp2Sync;
    
    /** Number of detected errors. */
    public int blp3Sync;
    
    /** BLP0 RCY status. */
    public int blp0Rcu;
    
    /** BLP1 RCY status. */
    public int blp1Rcu;
    
    /** BLP2 RCY status. */
    public int blp2Rcu;
    
    /** BLP3 RCY status. */
    public int blp3Rcu;
    
    /** Status information from CP. */
    public int cpStatus;
    
    /** BLP0 ADC offset values. */
    public int blp0AdcOffset;
    
    /** BLP1 ADC offset values. */
    public int blp1AdcOffset;
    
    /** BLP2 ADC offset values. */
    public int blp2AdcOffset;
    
    /** BLP3 ADC offset values. */
    public int blp3AdcOffset;    
}