function fft_subband_shifted(Nfft,nrsb_begin, nrsb_end,data)
data_ant=data;
nrsb=nrsb_end-nrsb_begin;
data_ant=data_ant(:,1:floor(length(data_ant)/Nfft)*Nfft);
SelectedSubBands=[nrsb_begin:nrsb_end];
tic
SelectedSubBandSignals=zeros(size(data_ant,1),floor(size(data_ant,2)/Nfft),length(SelectedSubBands));
for i=1:size(data_ant,1)
    data_s=data_ant(i,:);      
    datatest2=reshape(data_s,Nfft,floor(length(data_s)/Nfft));
    datatest3=fftshift(fft(datatest2,Nfft),1);
    SelectedSubBandSignals(i,:,:)=datatest3(SelectedSubBands,:).';
end
toc
AntennaSignals=data;
 load('data/signal_options.mat')
 snapshot_number=size(SelectedSubBandSignals,3);
 save('data/antenna_signals_mod.mat','AntennaSignals','SelectedSubBandSignals');
 
 save(['data/signal_options.mat'],'look_dir_phi','look_dir_theta','signal_freq','signal_type','array_type', ...
     'rfi_number','rfi_phi','rfi_theta','rfi_freq','rfi_ampl','snapshot_number','NumberOfAntennas','SnapToGrid','NullGrid');
 
