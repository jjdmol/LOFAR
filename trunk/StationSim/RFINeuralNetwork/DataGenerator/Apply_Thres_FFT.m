I=abs(DSP_1inst);
if get(RadioThres1,'value')==1
    Threshold_modu=2*sum(sum(abs(DSP_1inst)))/(str2num(get(Freq_Res,'string'))*(str2num(get(Int_Time,'string')))^2)
    set(Noise_minValue,'String',num2str(20*log10(Threshold_modu)));
else
    Threshold_modu=str2num(get(Thres,'string'))
    Threshold_modu=10^(Threshold_modu/20);
    set(Noise_minValue,'String',num2str(20*log10(Threshold_modu)));
end
set(0,'CurrentFigure',HMainFig_gene)
reference=(I>Threshold_modu);
clear line;
subplot(HPerioMod)
cla
hold on
periodogram(Signal_modulation,hamming(length(Signal_modulation)),'onesided',16384,fs)
a=axis;
plot([a(1) a(2)],[20*log10(Threshold_modu) 20*log10(Threshold_modu)],'Color',[1 0 0])
hold off
subplot(PolyphasePlot)
imagesc(reference')
colormap(1-gray)
%clear I;
