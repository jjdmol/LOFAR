function data_ant_int = nband_siggen4(nantl, bandwidth, Nfft, px, py,signal_mod,Phi_theta)

%Nfft=512;
%Nfft = 2^9; % number of frequency bins in a block
tic
signal_mod=signal_mod(1:floor(length(signal_mod)/Nfft)*Nfft);
%load_data;
cfreq=bandwidth/2;
data=signal_mod;
freq_shift0=[];
% introduce phase shift due to frequency drift
freq_res=bandwidth/Nfft;

% pre-allocate resulting signal
if Nfft==1
 freq_shift=1;  
else
freq_shift=(([-Nfft/2:1:Nfft/2-1].' * freq_res + cfreq) / cfreq);
end 
    % compute DOA vector for a single source
    a = DOA(px,py,Phi_theta(1:2,:).',Nfft);
    data=reshape(data,Nfft,length(signal_mod)/Nfft);
    if size(a,1)>size(data,2)
    a=a(1:size(data,2),:);
    else
    data=data(:,1:size(a,1));
    end
    
    for m=1:nantl
        freq_shift0=freq_shift*a(:,m).';
        freq_shift1=exp(i*freq_shift0);
        % figure
        % plot(data)
        % title('Raw data')
        if Nfft==1
        data_ant_int(m,:)=data.* freq_shift1;
        else
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
        data_ant_int(m,:)=reshape(data_inter,1,li*co);
        end
        %data_ant(m,:,k)=reshape(data_inter,1,li*co);
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

