mat1=Signal_modulation;
Freqsample=fs;
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
OutputSignal = DFTFilterBankQuantised(mat1,str2num(get(Filter_quant2,'string')), str2num(get(IS_Quant1,'string')), str2num(get(OS_Quant1,'string')), Filter1Coef, str2num(get(Subband_Nb,'string')),str2num(get(Order,'string')), 10000000000,1);
[lig_Out,col_Out]=size(OutputSignal);
OutputSignal=OutputSignal(:,1:col_Out/2)';
else 
OutputSignal = DFTFilterBankQuantised(mat1,str2num(get(Filter_quant2,'string')), str2num(get(IS_Quant1,'string')), str2num(get(OS_Quant1,'string')), Filter1Coef, str2num(get(Subband_Nb,'string')),str2num(get(Order,'string')), 10000000000,1);
[lig_Out,col_Out]=size(OutputSignal);
[OutputSignal] = SubbandSeparator_gene(OutputSignal(:,1:col_Out/2)',[1:str2num(get(Subband_Nb,'string'))],Filter2Coef,str2num(get(Subband_Nb2,'string')),str2num(get(Order2,'string')),str2num(get(Subband_Nb,'string'))/2, str2num(get(Filter_quant2,'string')), str2num(get(IS_Quant2,'string')), str2num(get(OS_Quant2,'string')));
[OutputSignal]=my_reshape(OutputSignal);
end  
set(0,'CurrentFigure',HMainFig_gene)
if (get(Checkbox1,'Value')==1)
    freq_real=Freqsample/str2num(get(Subband_Nb,'string'));
set(real_fr,'String',freq_real)
elseif (get(Checkbox1,'Value')==0)
     freq_real=Freqsample/(str2num(get(Subband_Nb,'string'))*str2num(get(Subband_Nb2,'string')));
set(real_fr,'String',freq_real)
end
set(real_time,'string',1/freq_real);
set(Total_time,'String',1/Freqsample*length(mat1));
subplot(PolyphasePlot)
imagesc(20*log10(abs(OutputSignal)))
colormap(1-gray)
indice_thres=1;
Thresholding;
%save_reference;
clear I;
%clear OutputSignal;
end 


