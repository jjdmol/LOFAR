global HPerioFigresu;
global HPerioFilter;
global Filter_name;
global Bandwidth;
global Fsample;
global Fresample;
global filter_box;
global res;


filter_Selected=get(filter_box,'Value');
Select_filter=get(Filter_name,'Value');

if filter_Selected==0
S_Fr=str2num(get(Fsample,'String'));
elseif filter_Selected==1
S_Fr=Res_Fr;
end

%filtre={'Chebicheff1';'Chebicheff2';'Butterworth'};
BandWi=str2num(get(Bandwidth,'String'));
if (BandWi>=S_Fr/2)
    message='The bandwith hqs tobe less than Fsampling/2';
    title='Warning';
    msgbox(message,title,'warn')
else
Wp=99/100*BandWi/S_Fr; %we assume to have our cut frequency at 9/10 off the needed bandwidth 
Ws=BandWi/S_Fr;
n=1000;
step2=S_Fr/2048;


switch Select_filter
    
case 1
     message='Y''a rien a voir';
    title='Attention';
    msgbox(message,title,'warn')
    
case 2  
f = [0 2*Ws 2*Ws 1]; m = [1 1 0 0];
b = fir2(n,f,m);
[h,w] = freqz(b,1,1024,S_Fr);
grid
%title('Normalised Phase','Fontsize',8)
subplot(HPerioFilter)
plot(w,20*log10(abs(h)))
grid
%title('Bode diagram (log Gfilter))','Fontsize',8)
subplot(HPerioFigresu)
pd=unwrap(angle(h))*360/(2*pi);
plot(w,pd);
grid
%title('Phase(degrees)/frequence','Fontsize',8)
case 3
f = [0 2*Wp 2*Ws 1];
amp = [1  0 0];
up = [1.02 0.01 0.01];
lo = [0.98 -0.01 -0.01];
b = fircls(n,f,amp,up,lo);
[h,w] = freqz(b,1,1024,S_Fr);
grid
%title('Normalised Phase','Fontsize',8)
subplot(HPerioFilter)
plot(w,20*log10(abs(h)))
grid
%title('Bode diagram (log Gfilter))','Fontsize',8)
subplot(HPerioFigresu)
pd=unwrap(angle(h))*360/(2*pi);
plot(w,pd);
%title('Phase(degrees)/frequence','Fontsize',8)

case 4
fcuts = [BandWi*99/100 BandWi];
mags = [1 0];
devs = [0.05 0.001];%-60db -> 0.00 attenuation for BandWi
[n,Wn,beta,ftype] = kaiserord(fcuts,mags,devs,S_Fr);
if n>(length(res)/3)-1;
    n=ceil((length(res)/3))-1;
end
b = fir1(n,Wn,ftype,kaiser(n+1,beta),'noscale');
[h,w] = freqz(b,1,1024,S_Fr);
subplot(HPerioFilter)
plot(w,20*log10(abs(h)))
grid
%title('Bode diagram (log Gfilter))/frequence','Fontsize',8)
subplot(HPerioFigresu)
pd=unwrap(angle(h))*360/(2*pi);
plot(w,pd);
grid
%title('Phase(degrees)/frequence','Fontsize',8)
end

clear tim2;
end