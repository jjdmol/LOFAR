global HPerioFigresu;
global HPerioFilter;
global Filter_name;
global Bandwidth;
global Fsample;
%filtre={'Chebicheff1';'Chebicheff2';'Butterworth'};
global Fresample;
global filter_box;
global res;
global S_Fs;
global voila;

filter_Selected=get(filter_box,'Value');
Select_filter=get(Filter_name,'Value');

if filter_Selected==0
S_Fr=str2num(get(Fsample,'String'));
elseif filter_Selected==1
S_Fr=Res_Fr;
end


BandWi=str2num(get(Bandwidth,'String'));
if (BandWi>=S_Fr/2)
    message='The bandwith hqs tobe less than Fsampling/2';
    title='Warning';
    msgbox(message,title,'warn')
else
Select_filter=get(Filter_name,'Value');
S_Fr=str2num(get(Fsample,'String'));
Wp=99/100*BandWi/S_Fr; %we assume to have our cut frequency at 9/10 off the needed bandwidth 
Ws=BandWi/S_Fr;
n=1000;
Wn=BandWi/S_Fr;
voila=[];

switch Select_filter
case 1
voila=res;    
case 2  
f = [0 2*Ws 2*Ws 1]; m = [1 1 0 0];
b = fir2(n,f,m);
voila=filtfilt(b(1:length(b)-1),1,res);
case 3
f = [0 2*Wp 2*Ws 1];
amp = [1 1 0];
up = [1.02 1.02 0.01];
lo = [0.98 0.98 -0.01];
b = fircls(n,f,amp,up,lo);
voila=filtfilt(b(1:length(b)-1),1,res);
case 4
fcuts = [BandWi*99/100 BandWi];
mags = [1 0];
devs = [0.05 0.001];%-60db -> 0.001 attenuation for BandWi
[n,Wn,beta,ftype] = kaiserord(fcuts,mags,devs,S_Fr);
if n>round((length(res)/3))-1;
     n=ceil((length(res)/3))-1;
end
b = fir1(n,Wn,ftype,kaiser(n+1,beta),'noscale');
voila=filtfilt(b,1,res);
end
tim=0:1/S_Fs:length(res)/S_Fs;
subplot(HPerioFilter)
plot(tim(1:(length(tim)-1)),voila);
subplot(HPerioFigresu)
periodogram(voila,hamming(length(voila)),'twosided',2048,S_Fs)
end