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
count_signal=count_signal+1;

switch modu_choice;    
case 1
     y = modulate(k*mat,fc,fs,'am');
      feature_mod(count_signal).modulation_param=0;
      feature_mod(count_signal).modulation_Ampl=k;
case 2
     y = modulate(k*mat,fc,fs,'amdsb-tc',feature);
     feature_mod(count_signal).modulation_param=feature;
     feature_mod(count_signal).modulation_Ampl=k;
case 3
    y = modulate(k*mat,fc,fs,'amssb');
     feature_mod(count_signal).modulation_param=0;
     feature_mod(count_signal).modulation_Ampl=k;
case 4
    y = modulate(k*mat,fc,fs,'fm',feature);
    feature_mod(count_signal).modulation_param=feature;
    feature_mod(count_signal).modulation_Ampl=k;
case 5
    y = modulate(k*mat,fc,fs,'pm',feature);
    feature_mod(count_signal).modulation_param=feature;
    feature_mod(count_signal).modulation_Ampl=k;
case 6
    mat1=(mat-min(mat))/max(mat-min(mat))
    y = modulate(mat1,fc,fs,'pwm','centered');
    feature_mod(count_signal).modulation_param=0;
    feature_mod(count_signal).modulation_Ampl=k;
    clear mat1;
end    

feature_mod(count_signal).name=modulation_name{modu_choice};
feature_mod(count_signal).Carrier_freq=fc;

if (count_signal==1 & count_file==1)
  Signal_modulation=zeros(length(y),1); 
end

[Signal_modulation]=modulation_long(y,Signal_modulation);

subplot(HPerioMod)
periodogram(Signal_modulation,hamming(length(Signal_modulation)),'onesided',16384,fs);

Modulated_signal(count_file).Modula_signal=Filename_load;
Modulated_signal(count_file).feature_modulat=feature_mod;
clear y;
essai_list
end