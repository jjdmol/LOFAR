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

fc= str2num(get(Carrier,'String'));
if (modu_choice==3 | modu_choice==4 | modu_choice==5)
feature=str2num(get(feature_modulation,'String'))
end


switch modu_choice;    
case 1
     y = modulate(x,fc,fs,'am')
case 2
     y = modulate(x,fc,fs,'amdsb-tc',feature)
case 3
    y = modulate(x,fc,fs,'amssb')
case 4
    y = modulate(x,fc,fs,'fm',feature)
case 5
    y = modulate(x,fc,fs,'pm',feature)
case 6
    y = modulate(x,fc,fs,'pwm','centered')
end    

subplot(HPerioMod)
periodogram(y,hamming(length(y)),'onesided',16384,fs)

