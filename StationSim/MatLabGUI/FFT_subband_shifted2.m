function fft_subband_shifted2(Nfft,Nfft2,nrsb_begin, nrsb_end,data,stage)


if (length(data)<Nfft*Nfft2) & (stage==1)
   % message=['Your signal is not long enough to apply 2 FFT stages.'];
    %title='Warning';
    %msgbox(message,title,'Warn') 
else

    figure_plot_pos = [ 360 110 800 600];
shh = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on')
%Figure principale
figure_plot = figure(...
   'Name','Analyser', ...
   'handlevisibility','callback',...
   'IntegerHandle','off',...
   'NumberTitle','off',...
   'Tag','Plot',...
   'position',figure_plot_pos);



data_ant=data;
nrsb=nrsb_end-nrsb_begin;
data_ant=data_ant(1:floor(length(data_ant)/Nfft)*Nfft);
SelectedSubBands=[nrsb_begin:nrsb_end];
tic
datatest=reshape(data_ant,Nfft,floor(length(data_ant)/Nfft));
datatest2=fftshift(fft(datatest,Nfft),1);
SelectedSubBandSignals=datatest2(SelectedSubBands,:).';
toc

if (stage==0)&(nrsb_begin==nrsb_end)
%First FFT stage 
plot(20*log10(abs(SelectedSubBandSignals)))  
title(['FFT of the signal : ' num2str(Nfft) ' subbands, selected subband : ' num2str(SelectedSubBands)])
else if (stage==1)&(nrsb_begin==nrsb_end)
%Second FFT stage 
SelectedSubBandSignals2=SelectedSubBandSignals(1:floor(length(SelectedSubBandSignals)/Nfft2)*Nfft2);
tic
SelectedSubBandSignals3=reshape(SelectedSubBandSignals2,Nfft2,floor(length(SelectedSubBandSignals2)/Nfft2));
SelectedSubBandSignals4=fftshift(fft(SelectedSubBandSignals3,Nfft2),1);
surf(20*log10(abs(SelectedSubBandSignals4.')))
title(['FFT of the signal : ' num2str(Nfft2) ' subbands, selected subband : ' num2str(nrsb_end)])
shading interp
view(0,90)
toc

else
surf(20*log10(abs(SelectedSubBandSignals)))
title(['FFT of the signal : ' num2str(Nfft) ' subbands, selected subband : ' num2str(nrsb_begin) ' - ' num2str(nrsb_end)])
shading interp
view(0,90)
end
end
end




%AntennaSignals=data;
%  load('data/signal_options.mat')
%  snapshot_number=size(SelectedSubBandSignals,3);
%  save('data/antenna_signals_mod.mat','AntennaSignals','SelectedSubBandSignals');
%  
%  save(['data/signal_options.mat'],'look_dir_phi','look_dir_theta','signal_freq','signal_type','array_type', ...
%      'rfi_number','rfi_phi','rfi_theta','rfi_freq','rfi_ampl','snapshot_number','NumberOfAntennas','SnapToGrid','NullGrid');
%  
