def rcu_status(values = [0]):
    """returns a dictionary of RCU settings (see 'rspctl --help')"""

    if (type(values) != type([0])):
        values = [values]
        
    settings = []

    # print "in %s values = "%(__name__),  [("%x"%v) for v in values]
    for val in values:
        a = {}
        
        a['INPUT_DELAY']  =  val & 0x0000007F         #     Sample delay for the data from the RCU.
        a['INPUT_ENABLE'] = (val & 0x00000080)/0x80   #     Enable RCU input.
        
        a['LBL_EN']       = (val & 0x00000100)/0x100  #     supply LBL antenna on (1) or off (0)
        a['LBH_EN']       = (val & 0x00000200)/0x200  #     sypply LBH antenna on (1) or off (0)
        a['HB_EN']        = (val & 0x00000400)/0x400  #     supply HB on (1) or off (0)
        a['BANDSEL']      = (val & 0x00000800)/0x800  #     low band (1) or high band (0)
        a['HB_SEL_0']     = (val & 0x00001000)/0x1000 #     HBA filter selection
        a['HB_SEL_1']     = (val & 0x00002000)/0x2000 #     HBA filter selection
        #Options : HBA_SEL_0 HBA_SEL_1 Function
        #             0          0      210-270 MHz
        #             0          1      170-230 MHz
        #             1          0      110-190 MHz
        #             1          1      all off

        a['VL_EN']        = (val & 0x00004000)/0x00004000 #     low band supply on (1) or off (0)
        a['VH_EN']        = (val & 0x00008000)/0x00008000 #     high band supply on (1) or off (0)
        a['VDIG_EN']      = (val & 0x00010000)/0x00010000 #     ADC supply on (1) or off (0)
        # a['LB_SEL_0']     = (val & 0x00020000)/0x00020000 #     LBA input selection
        # a['LB_SEL_1']     = (val & 0x00040000)/0x00040000 #     HP filter selection
        #Options : LB_SEL_0 LB_SEL_1 Function
        #             0        0    10-90 MHz + 10 MHz HPF
        #             0        1    30-80 MHz + 10 MHz HPF
        #             1        0    10-90 MHz + 30 MHz HPF
        #             1        1    30-80 MHz + 30 MHz HPF
        # Ruud changed the masks some time ago. I made this change on 24apr08
        # shouldn't matter, really
        a['LBL_LBH_SEL']     = (val & 0x00020000)/0x00020000 #     LBA input selection
        a['LB_FILTER']     = (val & 0x00040000)/0x00040000 #     HP filter selection
        #Options:   LB_FILTER Selection
        #            0  10-90 MHz
        #            1  30-80 MHz
        a['ATT_CNT_4']    = (val & 0x00080000)/0x00080000 # on (1) is  1dB attenuation
        a['ATT_CNT_3']    = (val & 0x00100000)/0x00100000 # on (1) is  2dB attenuation
        a['ATT_CNT_2']    = (val & 0x00200000)/0x00200000 # on (1) is  4dB attenuation
        a['ATT_CNT_1']    = (val & 0x00300000)/0x00300000 # on (1) is  8dB attenuation
        a['ATT_CNT_0']    = (val & 0x00800000)/0x00800000 # on (1) is 16dB attenuation
           
        a['PRSG']         = (val & 0x01000000)/0x01000000 # pseudo random sequence generator on (1), off (0)
        a['RESET']        = (val & 0x02000000)/0x02000000 # on (1) hold board in reset
        a['SPEC_INV']     = (val & 0x04000080)/0x04000080 # Enable spectral inversion (1) if needed.
        #                                                   **SPEC_INV not set reliably, DO NOT USE**
        a['RCU_VERSION']  = ((val & 0xF0000000) >> 28) & 0xF      # RCU version 
        a['TBD']          = ((val & 0x08000000) >> 24) & 0xFF     # reserved
            
        settings.append(a)
        
    return settings
