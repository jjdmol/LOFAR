global HPerioFilFig;
global HPerioFig;
global signal_choice;
global Time;
global Fsample;
global Band;
global Ampl;
global Frequency;
global Width;
global modu_choice;
global feature_modulation;
global Carrier;
global fs;
global Amplitude;

fc= str2num(get(Carrier,'String'));
if fc >=fs/2
    message='Fc must be less Fs/2 : Nyquist Frequency';
    title='Attention';
    msgbox(message,title,'warn')
else
if (modu_choice==2 | modu_choice==4 | modu_choice==5)
feature=str2num(get(feature_modulation,'String'))
end
if (modu_choice~=6)
k=str2num(get(Amplitude,'String'));
end
switch modu_choice;    
case 1
     y = modulate(k*mat,fc,fs,'am');
case 2
     y = modulate(k*mat,fc,fs,'amdsb-tc',feature);
case 3
    y = modulate(k*mat,fc,fs,'amssb');
case 4
    y = modulate(mat,fc,fs,'fm',feature);
    y=k*y;
case 5
    y = modulate(mat,fc,fs,'pm',feature);
    y=k*y;
case 6
    mat1=(mat-min(mat))/max(mat-min(mat));
    y = modulate(mat1,fc,fs,'pwm','centered');
    clear mat1;
end    

subplot(HPerioMod)
periodogram(y,hamming(length(y)),'onesided',16384,fs)
end
