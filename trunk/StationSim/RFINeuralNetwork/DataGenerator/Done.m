global signal_name;
global count_sig;
global s;
global Fsample;
global res;
global features;

S_Fs= str2num(get(Fsample,'String'));
long=[];

if (length(s)>1)
long=[];
for i=1:length(s)
a=length(s(i).signal_value);
long=[long a];
end
A=[];
[A,index]=sort(long);
A=flipud(A');
index=flipud(index');
res=zeros(A(1),1);

R=[];
signal=s(index(1)).signal_value;
for i=1:length(s)-1
    diff=0;
diff=A(1)-A(i+1);
if (diff~=0)
R=[s(index(i+1)).signal_value res(1:diff)'];
else
R=[s(index(i+1)).signal_value]; 
end
signal=[signal;R];
end


for i=1:length(s)
res=res+signal(i,:)';
end
else 
    res=s(1).signal_value;
end


tim=0:1/S_Fs:length(res)/S_Fs;
subplot(HPerioFig)
plot(tim(1:(length(tim)-1)),res)
subplot(HPerioFilFig)
periodogram(res,hamming(length(res)),'twosided',2048,S_Fs)
res=res(1:length(res)-1);

features(1).Sampling_Freq_signal=S_Fs;
features(1).total_time=1/S_Fs*length(res);

clear tim;
clear R;
clear signal;
clear ans;