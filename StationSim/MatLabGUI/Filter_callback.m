global HPerioFigresu;
global HPerioFilter;
global Filter_name;
global Bandwidth;
global Fsample;
%filtre={'Chebicheff1';'Chebicheff2';'Butterworth'};

BandWi=str2num(get(Bandwidth,'String'));
Select_filter=get(Filter_name,'Value');
S_Fr=str2num(get(Fsample,'String'));
n=16;

Wn=BandWi/S_Fr;


switch Select_filter
    
case 1
[n,Wn] = chebbord(Wp,Ws,3,100);
[b,a] = cheby1(n,Wn,-100);
subplot(HPerioFilter)
freqz(b,a,2048,S_Fr)

case 2
[n,Wn] = chebbord(Wp,Ws,3,100);
[b,a] = cheby2(n,Wn,-100);
subplot(HPerioFilter)
freqz(b,a,2048,S_Fr)

case 3
%[n,Wn] = buttord(Wp,Ws,3,200);
[b,a] = butter(n,Wn,'low');
[wn,h]=freqz(b,a,2048,S_Fr);
subplot(HPerioFilter)
plot(abs(h))
subplot(HPerioFigresu)
plot(wn)
end