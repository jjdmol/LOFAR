function fft_subband(Nfft,SelectedSubBands,data,dirpath)
data_ant=data;
data_ant=data_ant(:,1:floor(length(data_ant)/Nfft)*Nfft);
tic
SelectedSubBandSignals=zeros(size(data_ant,1),floor(size(data_ant,2)/Nfft),length(SelectedSubBands));
for i=1:size(data_ant,1)
    data_s=data_ant(i,:);      
    datatest2=reshape(data_s,Nfft,floor(length(data_s)/Nfft));
    datatest3=fft(datatest2,Nfft);
    SelectedSubBandSignals(i,:,:)=datatest3(SelectedSubBands,:).';
end
toc
AntennaSignals=data;
figure(21)
imagesc(log10(abs(squeeze(SelectedSubBandSignals(1,:,:)))));
%  load([dirpath 'data/signal_options.mat'])
%  snapshot_number=size(SelectedSubBandSignals,3);
 save([dirpath 'data/antenna_signals.mat'],'AntennaSignals','SelectedSubBandSignals');
 
 %save(['data/signal_options.mat'],'look_dir_phi','look_dir_theta','signal_freq','signal_type','array_type', ...
  %r   'rfi_number','rfi_phi','rfi_theta','rfi_freq','rfi_ampl','snapshot_number','NumberOfAntennas','SnapToGrid','NullGrid');
 
