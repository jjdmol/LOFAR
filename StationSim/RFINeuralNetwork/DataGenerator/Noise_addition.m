if length(mat)==0
    message='You must load a file to specify the noise time length';
    title='Attention';
    msgbox(message,title,'warn')
else
power=str2num(get(Noise_Var,'String'));
var=sqrt(10^(1/10*(power+10*log10(fs))));%one sided-band
if length(Signal_modulation)==0 
Signal_modulation_noisy=zeros(length(mat),1)+normrnd(0,var,length(mat),1);
else     
Signal_modulation_noisy=Signal_modulation+normrnd(0,var,length(Signal_modulation),1);
end
subplot(PolyphasePlot)
periodogram(Signal_modulation_noisy,hamming(length(Signal_modulation_noisy)),'twosided',16384,fs);
indice_reference=2;
end