%clear all
subband=65;
load('data\antenna_signals.mat');
data=AntennaSignals;
Nfft=128;
FFT_subband(Nfft,subband,subband,data);
%load('data\antenna_signals.mat');
load('data\antenna_signals_mod.mat');
%G=colormap(jet(16));
b=[];
g=[];
bandwidth=3000000;
freq_res=bandwidth/Nfft;
cfreq=bandwidth/2;
fr=[];
for i=1:size(AntennaSignals,1)
    f=angle(SelectedSubBandSignals(i,1));
    s=SelectedSubBandSignals(i,1);
    fr=[fr;s];
    b=[b;f];
end
fg=[];
load('data\antenna_config.mat')
load('data\signal_options.mat')
figure
a=[];
plot(px,py,'r+');
hold on
t=[-pi:0.01:pi];
freq_shift=(([-Nfft/2:1:Nfft/2-1].' * freq_res + cfreq) / cfreq);
freq_sub=freq_shift(subband);

for j=1:length(px)
    a(j,:) = b(1)+(freq_shift.' *-2*pi*(px(j) * sin(rfi_theta(1)) * cos(rfi_phi(1)) + py(j) * sin(rfi_theta(1)) * sin(rfi_phi(1)))); 
end
d=[];
for i=1:length(px)
    d(i)=b(i)-a(i,subband);
    x=0.2*cos(t)+px(i);
    y=0.2*sin(t)+py(i);
    plot(x,y,'b');
    [Xabs,Yabs]=pol2cart(a(i,subband),0.2);
    plot(px(i)+Xabs,py(i)+Yabs,'r*');
    [Xfeat,Yfeat]=pol2cart(b(i),0.2);
    plot([px(i) Xfeat+px(i)],[py(i) Yfeat+py(i)],'Color','g');
    hold on 
end
title('Matlab Datagenerator Verification (* is Theorical Phase) (- green) the extracted from the file)');
d=mod(abs(d),2*pi);
hold off
error_pct = 100 * (max(d)-min(d))/(2*pi)


