function [s]=Addition(signal_choice);
%global signal_choice;
global Time;
global Fsample;
global Band;
global Ampl;
global Frequency;
global signal_name;
global count_sig;
global s;
global Width;
global listbox_signal;
global Time_signal;
global list;
global phase;
global indice;
global res;
global offset_time;
global rad1;
global rad2;
global rad3;
global rad4;
global freq2;
global Power_noise;


if str2num(get(Time,'String'))>str2num(get(Time_signal,'String'));
    message='You must generate a  signal time equal or less than Total time ';
    title='Warning';
    msgbox(message,title,'Warn')
else
count_sig=count_sig+1;
%fill the structure

s(count_sig).Signal_type=signal_name{signal_choice};
s(count_sig).Time = str2num(get(Time,'String'));
    
    
s(count_sig).Fs= str2num(get(Fsample,'String'));
%s(count_sig).Bandwidth = str2num(get(Band,'String'));
if signal_choice < 8
s(count_sig).Ampl= str2num(get(Ampl,'String'));
s(count_sig).Frequency=str2num(get(Frequency,'String'));
end
step_time=1/s(count_sig).Fs; 
t=0:step_time:s(count_sig).Time;



if (Width~=0)
    s(count_sig).Timewidth=str2num(get(Width,'String'));
else
    s(count_sig).Timewidth=0;
end

if (freq2~=0)
    freqfin=str2num(get(freq2,'String'));
end

if indice~=1
    phase=rand(1)*2*pi;
end

switch signal_choice    
case 1
    s(count_sig).signal_value=s(count_sig).Ampl*sin(2*pi*s(count_sig).Frequency*t+phase);
    s(count_sig).phase=phase;
    s(count_sig).Type='None';
    s(count_sig).FreqFin=0;
case 2
   s(count_sig).signal_value=s(count_sig).Ampl*cos(2*pi*s(count_sig).Frequency*t+phase);
   s(count_sig).phase=phase;
   s(count_sig).Type='None';
   s(count_sig).FreqFin=0;
case 3
    s(count_sig).phase=0;
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'rectpuls',s(count_sig).Timewidth);
case 4
    s(count_sig).phase=0;
    s(count_sig).Type='None';
    s(count_sig).FreqFin=0;
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'tripuls',s(count_sig).Timewidth);
case 5
    s(count_sig).phase=0;
    s(count_sig).FreqFin=0;
    s(count_sig).Type='None';
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'gauspuls',s(count_sig).Frequency,s(count_sig).Timewidth);
case 6
    s(count_sig).phase=0;
    s(count_sig).FreqFin=0;
    s(count_sig).signal_value=s(count_sig).Ampl*square(t,s(count_sig).Timewidth);
    s(count_sig).Type='None';
case 7
    s(count_sig).phase=phase;
    if get(rad1,'Value')==1
    s(count_sig).Type='linear'; 
    s(count_sig).signal_value=s(count_sig).Ampl*chirp(t,s(count_sig).Frequency,s(count_sig).Time,freqfin,'li',phase);
    elseif get(rad2,'Value')==1
    s(count_sig).Type='Exponentiel';  
    s(count_sig).signal_value=s(count_sig).Ampl*chirp(t,s(count_sig).Frequency,s(count_sig).Time,freqfin,'lo',phase);
    elseif get(rad3,'Value')==1
    s(count_sig).signal_value=s(count_sig).Ampl*chirp(t,s(count_sig).Frequency,s(count_sig).Time,freqfin,'q',phase,'convex');
    s(count_sig).Type='Quadra Convex';
    elseif get(rad4,'Value')==1
    s(count_sig).signal_value=s(count_sig).Ampl*chirp(t,s(count_sig).Frequency,s(count_sig).Time,freqfin,'q',phase,'concave');
    s(count_sig).Type='Quadra Concave';
    end
    s(count_sig).FreqFin=freqfin;
case 8
    s(count_sig).phase=0;
    power_noise=str2num(get(Power_noise,'String'));
    s(count_sig).FreqFin=0;
    s(count_sig).Type='None';
    s(count_sig).Frequency='None';
    var_noise=sqrt(10^(1/10*(power_noise+10*log10(Fsample))));%one sided-band
    s(count_sig).Ampl = var_noise;
    s(count_sig).signal_value=normrnd(0,var_noise,length(t),1).';
end   

s(count_sig).signal_value=s(count_sig).signal_value(1:length(s(count_sig).signal_value)-1);
Done_2
if offset_time==[];
   offset_time=0;
else
s(count_sig).OffsetTime=offset_time/S_Fs;
end
list{count_sig}=[' ' s(count_sig).Signal_type '   -   ' num2str(s(count_sig).Frequency) '     -     ' num2str(s(count_sig).Ampl) '    -    'num2str(s(count_sig).Time) '    -     ' num2str(s(count_sig).Timewidth) '    -     ' num2str(s(count_sig).OffsetTime)  '    -    ' num2str(s(count_sig).phase) ' - ' num2str(s(count_sig).FreqFin) ' - 's(count_sig).Type ''];
set(listbox_signal,'string',list)
end

