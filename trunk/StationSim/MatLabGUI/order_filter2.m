filter_order2=str2num(get(Order2,'string'));
Number_subbands2=str2num(get(Subband_Nb2,'string'));
Filter2Coef = DFTFilterBankInitialization(filter_order2,Number_subbands2);