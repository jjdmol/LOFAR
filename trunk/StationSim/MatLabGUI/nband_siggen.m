function sh = nband_siggen(nantl, nrfi, look_dir_phi, look_dir_theta, cfreq, bandwidth, Nfft, ...
    phi_RFI, theta_RFI, px, py)
%Nfft=512;
%Nfft = 2^9; % number of frequency bins in a block
tic
%load_data;
load('C:\users\dromer\StationGUI datagen branch\LofarSimDataGenerator\data\signal_output.txt')
file_name='G:\DATA\everyone\Gerdes\trajectory.txt';
[Phi_theta]=trajectory_reader(file_name);
number_data=size(Phi_theta,1)*Nfft;
data=signal_output(1:number_data);
freq_shift0=[];
data=reshape(data,Nfft,number_data/Nfft);
% introduce phase shift due to frequency drift
freq_res=bandwidth/Nfft;
% pre-allocate resulting signal
sh = zeros(nantl,number_data);
freq_shift=(([-Nfft/2:1:Nfft/2-1].' * freq_res + cfreq) / cfreq);
for k=1:nrfi
    % compute DOA vector for a single source
    a = DOA(px,py, Phi_theta,Nfft);
    for m=1:nantl
        freq_shift0=freq_shift*a(:,m).';
        freq_shift1=exp(i*freq_shift0);
        % figure
        % plot(data)
        % title('Raw data')
data_inter=fftshift(fft(data,Nfft),1);
        % figure
        % imagesc(20*log10(abs(datatest3)))
        % title('FFT result')
        data_inter=data_inter .* freq_shift1;
        % figure
        % imagesc(20*log10(abs(datatest7)))
        % title('Frequency shifted FFT result');
        data_inter=ifft(ifftshift(data_inter,1),Nfft);
        [li,co]=size(data_inter);
        data_inter0=reshape(data_inter,1,li*co);
        data_ant(m,:,k)=data_inter0;
    end
    sh=sh+data_ant(:,:,k);
end

toc
% 
% [li,co]=size(data);
% data=reshape(data,1,li*co).';
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

function a = DOA(px,py,Phi_theta,Nfft)
% computes the direction of arrival of a signal

N1 = size(px,2);
N2 = size(py,2);
%a = zeros(N1,N2);

for n=1:N1
   %for m=1:N2
   %a(n) = exp(-j*2*pi*(px(n)*sin(phi)*cos(theta)+py(n)*sin(theta))); 
   %a(n) = -j*2*pi*(px(n)*sin(phi)*cos(theta)+py(n)*sin(theta)); 
   a(:,n) = (-2*pi*(px(n).* sin(Phi_theta(:,2)).* cos(Phi_theta(:,1)) + py(n).* sin(Phi_theta(:,2)).* sin(Phi_theta(:,1))))/Nfft;
   %end
end 
return;