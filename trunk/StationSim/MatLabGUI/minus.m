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

%if count_sig==0
 %   message='There is no generated signal';
  %  title='Warning';
   % msgbox(message,title,'Warn')
   %else
%Selection=get(listbox_signal,'Value');
%res=res-s(Selection).signal_value';

%if Selection~=length(list)
%for i=Selection+1:length(list) 
 %   list{i}=list{i-1}
 %   s(i)=s(i-1);
 %end
 %else 
%    list{Selection}=[];
 %   s(Selection)=[];
 %end


%tim=0:1/S_Fs:Time_sig;
%set(0,'CurrentFigure',HMainFig_gene1)
%subplot(HPerioFig)
%plot(tim(1:(length(tim)-1)),res)
%subplot(HPerioFilFig)
%periodogram(res,hamming(length(res)),'twosided',2048,S_Fs)


%clear tim;
%clear R;
%clear signal;
%clear ans;
end