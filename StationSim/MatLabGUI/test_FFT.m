
Nfft=128;
nrsb_begin=64;
nrsb_end=64;
load('data/antenna_signals.mat');
data_ant=AntennaSignals;
SelectedSubBands=24;
nrsb=nrsb_end-nrsb_begin;

data_ant=data_ant(:,1:floor(length(data_ant)/Nfft)*Nfft);
SelectedSubBands=[nrsb_begin:nrsb_end];
win=hanning(Nfft);
win=repmat(win,1,floor(length(data_ant)/Nfft));
data=data_ant(1,:); 
%Apply FFT
datatest=reshape(data,Nfft,floor(length(data)/Nfft));
datatest1=fft(datatest,Nfft);
result1=datatest1(SelectedSubBands,:);
%Plot the result
figure(1)
imagesc(20*(log10(abs(datatest1))))
figure(2)
plot(20*log10(abs(result1)))
%Apply inverse FFT
data_inter=ifft(datatest1,Nfft);
[li,co]=size(data_inter);
data_inter=reshape(data_inter,1,li*co).';
figure(3)
plot(20*log10(abs(data_inter)))
%Apply FFT
data_inter=reshape(data_inter,Nfft,floor(length(data_inter)/Nfft));
datatest2=data_inter;%.*win;
datatest3=fft(datatest2,Nfft);
result2=datatest3(SelectedSubBands,:);
%Plot the result
figure(4)
imagesc(20*(log10(abs(datatest3)))) 
figure(5)
plot(20*log10(abs(result2)))
figure(6)
diff=abs(result2)-abs(result1);
plot(diff)
title('Gain between the two result*(FFT<->FFT_iFFT_FFT)')
figure(7)
plot(angle(result2)-angle(result1))
title('Angle between the two result*(FFT<->FFT_iFFT_FFT)')