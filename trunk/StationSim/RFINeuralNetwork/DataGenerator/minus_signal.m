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
global listbox_signal;
global list;
global res;

Time_sig=str2num(get(Time_signal,'String'));
S_Fs=str2num(get(Fsample,'String'));

if count_sig==0
    message='There is no generated signal';
    title='Warning';
    msgbox(message,title,'Warn')
else
Selection=get(listbox_signal,'Value');
res=res-s(Selection).signal_value';

if Selection~=length(s)
    
for i=Selection:length(s)-1;
    list{i}=list{i+1};
    list{i+1}='';
    
end
for i=Selection+1:length(s)
    s(i-1)=s(i);
end
s(length(s))=[];
else 
    list{Selection}='';
    s(Selection)=[];
end
count_sig=count_sig-1;

set(listbox_signal,'String',list);
tim=0:1/S_Fs:Time_sig;
set(0,'CurrentFigure',HMainFig_gene1)
subplot(HPerioFig)
plot(tim(1:(length(tim)-1)),res)
subplot(HPerioFilFig)
periodogram(res,hamming(length(res)),'twosided',2048,S_Fs)


clear tim;
clear R;
clear signal;
clear ans;
end