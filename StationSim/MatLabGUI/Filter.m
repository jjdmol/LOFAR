global HPerioFigresu;
global HPerioFilter;
global Filter_name;
global Bandwidth;
%filtre={'Chebicheff1';'Chebicheff2';'Butterworth'};

BandWi=str2num(get(Bandwidth),'String');
Select_filter=str2num(get(Bandwidth),'Selected');
S_Fr=str2num(get(Frequency,'String'));

Wp=0;
Ws=BandWi/Fr_s;

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
[n,Wn] = buttord(Wp,Ws,3,100);
[b,a] = butter(n,Wn,-100);
freqz(b,a,2048,S_Fr)
subplot(HPerioFilter)
end