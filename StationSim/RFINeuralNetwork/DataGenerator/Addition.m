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
count_sig=count_sig+1;
%fill the structure

s(count_sig).Signal_type=signal_name{signal_choice};
s(count_sig).Time = str2num(get(Time,'String'));
if s(count_sig).Time>str2num(get(Time_signal,'String'));
    message='You must generate a  signal time equal or less than Total time ';
    title='Warning';
    msgbox(message,title,'Warn')
else
    
s(count_sig).Fs= str2num(get(Fsample,'String'));
%s(count_sig).Bandwidth = str2num(get(Band,'String'));
s(count_sig).Ampl= str2num(get(Ampl,'String'));
step_time=1/s(count_sig).Fs; 
t=0:step_time:s(count_sig).Time;
s(count_sig).Frequency=str2num(get(Frequency,'String'));

if (Width~=0)
    s(count_sig).Timewidth=str2num(get(Width,'String'));
else
    s(count_sig).Timewidth=0;
end

if indice~=1
    phase=rand(1)*2*pi;
    
end

switch signal_choice    
case 1
    s(count_sig).signal_value=s(count_sig).Ampl*sin(2*pi*s(count_sig).Frequency*t+phase);
    s(count_sig).phase=phase;
case 2
   s(count_sig).signal_value=s(count_sig).Ampl*cos(2*pi*s(count_sig).Frequency*t+phase);
   s(count_sig).phase=phase;
case 3
    s(count_sig).phase=0;
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'rectpuls',s(count_sig).Timewidth);
case 4
    s(count_sig).phase=0;
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'tripuls',s(count_sig).Timewidth);
case 5
    s(count_sig).phase=0;
    s(count_sig).signal_value=s(count_sig).Ampl*pulstran(t,0:1/s(count_sig).Frequency:s(count_sig).Time,'gauspuls',s(count_sig).Frequency,s(count_sig).Timewidth);
case 6
    s(count_sig).phase=0;
    s(count_sig).signal_value=s(count_sig).Ampl*square(t,s(count_sig).Timewidth);
end    
s(count_sig).signal_value=s(count_sig).signal_value(1:length(s(count_sig).signal_value)-1);
Done_2
if offset_time==[];
    offset_time=0;
else
s(count_sig).OffsetTime=offset_time/S_Fs;
end
list{count_sig}=['   ' s(count_sig).Signal_type '   -    ' num2str(s(count_sig).Frequency) '     -     ' num2str(s(count_sig).Ampl) '    -    'num2str(s(count_sig).Time) '    -     ' num2str(s(count_sig).Timewidth) '    -     ' num2str(s(count_sig).OffsetTime)  '    -    ' num2str(s(count_sig).phase) ''];
set(listbox_signal,'string',list)
end

