if length(Signal_modulation_noisy)~=0
  freqsample=fs;
  mat=Signal_modulation_noisy;
  clear Signal_modulation_noisy;
  trace=1;
else
freqsample=fs_2; 
frequency_res=str2num(get(Freq_Res,'string'));
puissance=round(log2(fs_2/frequency_res));
real_res=fs_2/2^puissance;
set(real_fr,'String',real_res);
set(0,'CurrentFigure',HMainFig_gene)
set(Total_time,'String',1/fs_2*length(mat));
subplot(PolyphasePlot)
periodogram(mat,hamming(length(mat)),'onesided',2^puissance,freqsample)
end