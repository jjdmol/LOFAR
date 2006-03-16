package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * This class keeps hold of the Board Status information.
 * @author  balken
 */
public class BoardStatus
{
    /** Measured 1.2 V supply voltage. */
    private int voltage_1V2;
    
    /** Measured 2.5 V supply voltage. */
    private int voltage_2V5;
    
    /** Measured 3.3 V supply voltage. */
    private int voltage_3V3;
    
    /** Current RSP board temperature. */
    private int pcb_temp;
    
    /** Current Board Processor temperature. */
    private int bp_temp;
    
    /** Current Antenna Processor 0 temperature. */
    private int ap0_temp;
    
    /** Current Antenna Processor 1 temperature. */
    private int ap1_temp;
    
    /** Current Antenna Processor 2 temperature. */
    private int ap2_temp;
    
    /** Current Antenna Processor 3 temperature. */
    private int ap3_temp;
    
    /** Current Board Processor system clock speed. */
    private int bp_clock;
    
    /** Number of eth frames received. */
    private int nof_frames;
    
    /** Number of incorrect ethernet frames. */
    private int nof_errors;
    
    /** Error status of last received ethernet frame. */
    private int last_error;
    
    /** Sequence number of previous received message. */
    private int seqnr;
    
    /** Error status of previous received message. */
    private int error;
    
    /** Interface under test. */
    private int iinterface; // @TODO change name
    
    /** Test mode. */
    private int mode;
    
    /** Number of detected errors. */
    private int ri_errors;
    
    /** Number of detected errors. */
    private int rcux_errors;
    
    /** Number of detected errors. */
    private int lcu_errors;
    
    /** Number of detected errors. */
    private int cep_errors;
    
    /** Number of detected errors. */
    private int serdes_errors;
    
    /** Number of detected errors. */
    private int ap0_ri_errors;
    
    /** Number of detected errors. */
    private int ap1_ri_errors;
    
    /** Number of detected errors. */
    private int ap2_ri_errors;
    
    /** Number of detected errors. */
    private int ap3_ri_errors;
    
    /** Number of detected errors. */
    private int blp0_sync;
    
    /** Number of detected errors. */
    private int blp1_sync;
    
    /** Number of detected errors. */
    private int blp2_sync;
    
    /** Number of detected errors. */
    private int blp3_sync;
    
    /** BLP0 RCY status. */
    private int blp0_rcu;
    
    /** BLP1 RCY status. */
    private int blp1_rcu;
    
    /** BLP2 RCY status. */
    private int blp2_rcu;
    
    /** BLP3 RCY status. */
    private int blp3_rcu;
    
    /** Status information from CP. */
    private int cp_status;
    
    /** BLP0 ADC offset values. */
    private int blp0_adc_offset;
    
    /** BLP1 ADC offset values. */
    private int blp1_adc_offset;
    
    /** BLP2 ADC offset values. */
    private int blp2_adc_offset;
    
    /** BLP3 ADC offset values. */
    private int blp3_adc_offset;

    // getters and setters we're automatically added. 
    // TODO add comments to the getters and setters.
    
    public int getVoltage1V2() 
    {
        return voltage_1V2;
    }

    public void setVoltage1V2(int voltage_1V2) 
    {
        this.voltage_1V2 = voltage_1V2;
    }

    public int getVoltage2V5() 
    {
        return voltage_2V5;
    }

    public void setVoltage2V5(int voltage_2V5) 
    {
        this.voltage_2V5 = voltage_2V5;
    }

    public int getVoltage3V3() 
    {
        return voltage_3V3;
    }

    public void setVoltage3V3(int voltage_3V3) 
    {
        this.voltage_3V3 = voltage_3V3;
    }

    public int getPcbTemp() 
    {
        return pcb_temp;
    }

    public void setPcbTemp(int pcb_temp) 
    {
        this.pcb_temp = pcb_temp;
    }

    public int getBpTemp() 
    {
        return bp_temp;
    }

    public void setBpTemp(int bp_temp) 
    {
        this.bp_temp = bp_temp;
    }

    public int getAp0Temp() 
    {
        return ap0_temp;
    }

    public void setAp0Temp(int ap0_temp) 
    {
        this.ap0_temp = ap0_temp;
    }

    public int getAp1Temp() 
    {
        return ap1_temp;
    }

    public void setAp1Temp(int ap1_temp) 
    {
        this.ap1_temp = ap1_temp;
    }

    public int getAp2Temp() 
    {
        return ap2_temp;
    }

    public void setAp2Temp(int ap2_temp)
    {
        this.ap2_temp = ap2_temp;
    }

    public int getAp3Temp() 
    {
        return ap3_temp;
    }

    public void setAp3Temp(int ap3_temp) 
    {
        this.ap3_temp = ap3_temp;
    }

    public int getBpClock() 
    {
        return bp_clock;
    }

    public void setBpClock(int bp_clock) 
    {
        this.bp_clock = bp_clock;
    }

    public int getNofFrames() 
    {
        return nof_frames;
    }

    public void setNofFrames(int nof_frames) 
    {
        this.nof_frames = nof_frames;
    }

    public int getNofErrors() 
    {
        return nof_errors;
    }

    public void setNofErrors(int nof_errors) 
    {
        this.nof_errors = nof_errors;
    }

    public int getLastError() 
    {
        return last_error;
    }

    public void setLastError(int last_error) 
    {
        this.last_error = last_error;
    }

    public int getSeqnr() 
    {
        return seqnr;
    }

    public void setSeqnr(int seqnr) 
    {
        this.seqnr = seqnr;
    }

    public int getError() 
    {
        return error;
    }

    public void setError(int error) 
    {
        this.error = error;
    }

    public int getInterface() 
    {
        return iinterface;
    }

    public void setInterface(int iinterface) 
    {
        this.iinterface = iinterface;
    }

    public int getMode() 
    {
        return mode;
    }

    public void setMode(int mode) 
    {
        this.mode = mode;
    }

    public int getRiErrors() 
    {
        return ri_errors;
    }

    public void setRiErrors(int ri_errors) 
    {
        this.ri_errors = ri_errors;
    }

    public int getRcuxErrors() 
    {
        return rcux_errors;
    }

    public void setRcuxErrors(int rcux_errors) 
    {
        this.rcux_errors = rcux_errors;
    }

    public int getLcuErrors() 
    {
        return lcu_errors;
    }

    public void setLcuErrors(int lcu_errors) 
    {
        this.lcu_errors = lcu_errors;
    }

    public int getCepErrors() 
    {
        return cep_errors;
    }

    public void setCepErrors(int cep_errors) 
    {
        this.cep_errors = cep_errors;
    }

    public int getSerdesErrors() 
    {
        return serdes_errors;
    }

    public void setSerdesErrors(int serdes_errors) 
    {
        this.serdes_errors = serdes_errors;
    }

    public int getAp0RiErrors() 
    {
        return ap0_ri_errors;
    }

    public void setAp0RiErrors(int ap0_ri_errors) 
    {
        this.ap0_ri_errors = ap0_ri_errors;
    }

    public int getAp1RiErrors() 
    {
        return ap1_ri_errors;
    }

    public void setAp1RiErrors(int ap1_ri_errors) 
    {
        this.ap1_ri_errors = ap1_ri_errors;
    }

    public int getAp2RiErrors() 
    {
        return ap2_ri_errors;
    }

    public void setAp2RiErrors(int ap2_ri_errors) 
    {
        this.ap2_ri_errors = ap2_ri_errors;
    }

    public int getAp3RiErrors() 
    {
        return ap3_ri_errors;
    }

    public void setAp3RiErrors(int ap3_ri_errors) 
    {
        this.ap3_ri_errors = ap3_ri_errors;
    }

    public int getBlp0Sync() 
    {
        return blp0_sync;
    }

    public void setBlp0Sync(int blp0_sync) 
    {
        this.blp0_sync = blp0_sync;
    }

    public int getBlp1Sync() 
    {
        return blp1_sync;
    }

    public void setBlp1Sync(int blp1_sync) 
    {
        this.blp1_sync = blp1_sync;
    }

    public int getBlp2Sync() 
    {
        return blp2_sync;
    }

    public void setBlp2Sync(int blp2_sync) 
    {
        this.blp2_sync = blp2_sync;
    }

    public int getBlp3Sync() 
    {
        return blp3_sync;
    }

    public void setBlp3Sync(int blp3_sync) 
    {
        this.blp3_sync = blp3_sync;
    }

    public int getBlp0Rcu() 
    {
        return blp0_rcu;
    }

    public void setBlp0Rcu(int blp0_rcu) 
    {
        this.blp0_rcu = blp0_rcu;
    }

    public int getBlp1Rcu() 
    {
        return blp1_rcu;
    }

    public void setBlp1Rcu(int blp1_rcu) 
    {
        this.blp1_rcu = blp1_rcu;
    }

    public int getBlp2Rcu() 
    {        
        return blp2_rcu;
    }

    public void setBlp2Rcu(int blp2_rcu) 
    {
        this.blp2_rcu = blp2_rcu;
    }

    public int getBlp3Rcu() 
    {
        return blp3_rcu;
    }

    public void setBlp3Rcu(int blp3_rcu) 
    {
        this.blp3_rcu = blp3_rcu;
    }

    public int getCpStatus() 
    {
        return cp_status;
    }

    public void setCpStatus(int cp_status) 
    {
        this.cp_status = cp_status;
    }

    public int getBlp0AdcOffset() 
    {
        return blp0_adc_offset;
    }

    public void setBlp0AdcOffset(int blp0_adc_offset) 
    {
        this.blp0_adc_offset = blp0_adc_offset;
    }

    public int getBlp1AdcOffset() 
    {
        return blp1_adc_offset;
    }

    public void setBlp1AdcOffset(int blp1_adc_offset) 
    {
        this.blp1_adc_offset = blp1_adc_offset;
    }

    public int getBlp2AdcOffset() 
    {
        return blp2_adc_offset;
    }

    public void setBlp2AdcOffset(int blp2_adc_offset) 
    {
        this.blp2_adc_offset = blp2_adc_offset;
    }

    public int getBlp3AdcOffset() 
    {
        return blp3_adc_offset;
    }

    public void setBlp3AdcOffset(int blp3_adc_offset) 
    {
        this.blp3_adc_offset = blp3_adc_offset;
    }  
    
}