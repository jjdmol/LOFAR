global HPerioFilFig;
global HPerioFig;
global signal_choice;
global Time;
global Fsample;
global Band;
global Ampl;
global Frequency;
global Width;
global phase;

S_time = str2num(get(Time,'String'));
S_Fs= str2num(get(Fsample,'String'));
%S_Bandwidth = str2num(get(Band,'String'));
S_Ampl= str2num(get(Ampl,'String'));
step_time1=1/S_Fs; 
t=0:step_time1:S_time;
S_Frequency=str2num(get(Frequency,'String'));

if (Width~=0)
    S_width=str2num(get(Width,'String'));
end
indice=1;
phase=rand(1)*2*pi;
switch signal_choice    
case 1
    signal_value=S_Ampl*sin(2*pi*S_Frequency*t+phase);
case 2
    signal_value=S_Ampl*cos(2*pi*S_Frequency*t+phase);
case 3
    signal_value=S_Ampl*pulstran(t,0:1/S_Frequency:S_time,'rectpuls',S_width);
case 4
    signal_value=S_Ampl*pulstran(t,0:1/S_Frequency:S_time,'tripuls',S_width);
case 5
    signal_value=S_Ampl*pulstran(t,0:1/S_Frequency:S_time,'gauspuls',S_Frequency,S_width);
case 6
    signal_value=S_Ampl*square(t,S_width);
end    

subplot(HPerioFig)
plot(t,signal_value)
subplot(HPerioFilFig)
periodogram(signal_value,hamming(length(signal_value)),'twosided',2048,S_Fs)

clear ans;
%clear signal_value;
clear t;
