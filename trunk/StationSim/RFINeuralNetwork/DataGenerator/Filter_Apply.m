trace=0
if (length(Signal_modulation_noisy)~=0)
  mat=Signal_modulation_noisy;
  Freqsample=fs;
  clear Signal_modulation_noisy;
  trace=1
else
    Freqsample=fs_2;
end
if (length(Filter1Coef)==0)
    message='You must compute the filter 1 coefficients';
    title='Warning';
    msgbox(message,title,'warn')
elseif (get(Checkbox2,'Value')==1&length(Filter2Coef)==0)
    message='You must compute the filter 2 coefficients';
    title='Warning';
    msgbox(message,title,'warn')
else 
if (get(Checkbox1,'Value')==1)
OutputSignal = DFTFilterBankQuantised(mat,str2num(get(Filter_quant1,'string')), str2num(get(IS_Quant1,'string')), str2num(get(OS_Quant1,'string')), Filter1Coef, str2num(get(Subband_Nb,'string')),str2num(get(Order,'string')), 10000000000,1);
[lig_Out,col_Out]=size(OutputSignal);
OutputSignal3=OutputSignal(:,1:col_Out/2)';
else 
OutputSignal = DFTFilterBankQuantised(mat,str2num(get(Filter_quant2,'string')), str2num(get(IS_Quant1,'string')), str2num(get(OS_Quant1,'string')), Filter1Coef, str2num(get(Subband_Nb,'string')),str2num(get(Order,'string')), 10000000000,1);
[lig_Out,col_Out]=size(OutputSignal);
[OutputSignal2] = SubbandSeparator_gene(OutputSignal(:,1:col_Out/2)',[1:str2num(get(Subband_Nb,'string'))],Filter2Coef,str2num(get(Subband_Nb2,'string')),str2num(get(Order2,'string')),str2num(get(Subband_Nb,'string'))/2, str2num(get(Filter_quant2,'string')), str2num(get(IS_Quant2,'string')), str2num(get(OS_Quant2,'string')));
[OutputSignal3]=my_reshape(OutputSignal2);
end  
set(0,'CurrentFigure',HMainFig_gene)
if (get(Checkbox1,'Value')==1)
    freq_real=Freqsample/str2num(get(Subband_Nb,'string'));
set(real_fr,'String',freq_real)
elseif (get(Checkbox1,'Value')==0)
     freq_real=Freqsample/(str2num(get(Subband_Nb,'string'))*str2num(get(Subband_Nb2,'string')));
set(real_fr,'String',freq_real)
end
indice=2;
set(real_time,'string',1/freq_real);
set(Total_time,'String',1/Freqsample*length(mat));
subplot(PolyphasePlot)
Factor=10^((str2num(get(Noise_Var,'String'))-20*log10(mean(mean(abs(OutputSignal3)))))/20);
OutputSignal3=Factor*OutputSignal3;
imagesc(20*log10(abs(OutputSignal3)))
colormap(1-gray)
end 
if trace==1
Signal_modulation_noisy=mat;
clear mat;
end