%function [res,s]=Done_2(s,list,count_sig);
global signal_name;
global count_sig;
global s;
global Fsample;
global res;
global features;
global Time_signal;
global HMainFig_gene1;
global HPerioFig;
global HPerioFilFig;
global S_Fs;
global offset_time;

S_Fs=str2num(get(Fsample,'String'));
Time_sig=str2num(get(Time_signal,'String'));


if count_sig==1
   res=zeros(S_Fs*Time_sig,1); 
end

if length(s(count_sig).signal_value)==length(res);
res=res+s(count_sig).signal_value';
offset_time=0;
else
maximum_time=length(res)-length(s(count_sig).signal_value);
offset_time=ceil(rand(1)*maximum_time);
before_time=zeros(offset_time,1);
after_time=zeros(length(res)-(length(before_time)+length(s(count_sig).signal_value)),1);
s(count_sig).signal_value=[before_time' s(count_sig).signal_value after_time'];
res=res+s(count_sig).signal_value';
end

tim=0:1/S_Fs:Time_sig;
set(0,'CurrentFigure',HMainFig_gene1)
subplot(HPerioFig)
plot(tim(1:(length(tim)-1)),res)
subplot(HPerioFilFig)
periodogram(res,hamming(length(res)),'twosided',2048,S_Fs)
features(1).Sampling_Freq_signal=S_Fs;
features(1).total_time=1/S_Fs*length(res);

clear tim;
clear R;
clear signal;
clear after_time;
clear before_time;
clear ans;