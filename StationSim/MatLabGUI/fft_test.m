function sh = nband_siggen(nantl, nrfi, look_dir_phi, look_dir_theta, cfreq, bandwidth, Nfft, ...
    phi_RFI, theta_RFI, px, py)
%Nfft=512;
%Nfft = 2^9; % number of frequency bins in a block
tic
load_data;



% Declare some variables that define the signal
nsnsh=floor(length(data)/Nfft)*Nfft;

data=data(1:floor(length(data)/Nfft)*Nfft);

% introduce phase shift due to frequency drift
freq_res=bandwidth/Nfft;
freq_shift=exp(([-Nfft/2:1:Nfft/2-1].' * j * freq_res + cfreq) / cfreq);
freq_shift=repmat(freq_shift,1,floor(length(data)/Nfft));

% figure
% plot(data)
% title('Raw data')
data=reshape(data,Nfft,floor(length(data)/Nfft));
data=fft(data,Nfft);
% figure
% imagesc(20*log10(abs(datatest3)))
% title('FFT result')

data=data .* freq_shift;
% figure
% imagesc(20*log10(abs(datatest7)))
% title('Frequency shifted FFT result');

data=ifft(data,Nfft);
[li,co]=size(data);
data=reshape(data,1,li*co).';
% figure
% plot(real(datatest5))
% title('Data after iFFT')

% figure
% plot(datatest1-real(datatest5));
% title('Real part : problem after fft and ifft that is not reversible; Nfft=512 points,Hanning(window)')
% xlabel('time series')
% ylabel('\Delta (before FFT, after iFFT)')
% figure
% plot(imag(datatest5));
% title('Imaginary part : problem after fft and ifft that is not reversible; Nfft=512 points,Hanning(window)')
% xlabel('time series')
% ylabel('Imaginary part')


% Transform data to create an antenna_signal array of size (#antennas,#snapshots)

data = data.';


base=zeros(nantl,length(data));
for i=1:nantl
    base(i,:)=data(1,:);
end

sh=zeros(nantl,nsnsh);

l=1;
for l=1:nrfi  
    sv = steerv(px,py,phi_RFI(l),theta_RFI(l))/nantl;     
    sv = repmat(sv,1,nsnsh);
    sh = sh + sv .* base;
end
toc