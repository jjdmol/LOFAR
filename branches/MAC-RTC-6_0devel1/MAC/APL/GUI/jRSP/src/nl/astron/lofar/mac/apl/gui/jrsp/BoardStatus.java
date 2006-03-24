package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * This class keeps hold of the Board Status information. All member variables are public so there is no
 * need for getters and setters.
 * @author  balken
 */
public class BoardStatus
{
    /** Measured 1.2 V supply voltage. */
    public double voltage1V2;
    
    /** Measured 2.5 V supply voltage. */
    public double voltage2V5;
    
    /** Measured 3.3 V supply voltage. */
    public double voltage3V3;
    
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
    public int rcuyErrors;
    
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
    public SyncStatus blp0Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp1Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp2Sync;
    
    /** Number of detected errors. */
    public SyncStatus blp3Sync;
    
    /** BLP0 RCU status. */
    public RCUStatus blp0Rcu;
    
    /** BLP1 RCU status. */
    public RCUStatus blp1Rcu;
    
    /** BLP2 RCU status. */
    public RCUStatus blp2Rcu;
    
    /** BLP3 RCU status. */
    public RCUStatus blp3Rcu;
    
    /** Status information from CP. */
    /** RDY - Type of image loaded in FPGA. */
    public int cpRdy;
    
    /** ERR - Configuration result. */
    public int cpErr;
    
    /** APnBP - Type of FPGA that was configured previously. */
    public int cpFpga;
    
    /** IM - Type of image loaded in FPGA. */
    public int cpIm;
    
    /** TRIG - Cause of previous reconfiguration. */
    public int cpTrig;
    
    /** BLP0 ADC offset values. */
    public ADOStatus blp0AdcOffset;
    
    /** BLP1 ADC offset values. */
    public ADOStatus blp1AdcOffset;
    
    /** BLP2 ADC offset values. */
    public ADOStatus blp2AdcOffset;
    
    /** BLP3 ADC offset values. */
    public ADOStatus blp3AdcOffset;    
    
    /**
     * Default Constructor.
     */
    public BoardStatus()
    {
        blp0Sync = new SyncStatus();
        blp1Sync = new SyncStatus();
        blp2Sync = new SyncStatus();
        blp3Sync = new SyncStatus();
        blp0Rcu = new RCUStatus();
        blp1Rcu = new RCUStatus();
        blp2Rcu = new RCUStatus();
        blp3Rcu = new RCUStatus();
        blp0AdcOffset = new ADOStatus();
        blp1AdcOffset = new ADOStatus();
        blp2AdcOffset = new ADOStatus();
        blp3AdcOffset = new ADOStatus();
    }
}