filter_order=str2num(get(Order,'string'));
Number_subbands=str2num(get(Subband_Nb,'string'));
Filter1Coef = DFTFilterBankInitialization(filter_order,Number_subbands);
 
 