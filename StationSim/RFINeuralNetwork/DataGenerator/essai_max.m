I=abs(OutputSignal3_N);
Threshold_modu=5*sum(sum(I))/(str2num(get(Subband_Nb,'string'))*str2num(get(Subband_Nb2,'string')))
reference=(I>Threshold_modu)