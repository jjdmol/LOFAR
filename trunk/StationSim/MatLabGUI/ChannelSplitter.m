function ChannelSplitter(beamformer_enable)

  dirpath='data';
  load([dirpath '/antenna_signals.mat']);
  load([dirpath '/antenna_config.mat']);
  load([dirpath '/channel_options.mat']);
  load([dirpath '/signal_options.mat']);
  load([dirpath '/output_options.mat']);
  
  if (~beamformer_enable)
     load([dirpath '/subband_options.mat']);
     BFSignals(:,:)=SelectedSubBandSignals(:,:,round(NumberSubBands/2));
  end
  FlaggingCube=1;
  DFTSystemResponse = DFTFilterBankInitialization(ChannelFilterLength, NumberChannels);
  SelectedChannelSignals = SubbandSeparator(BFSignals, SelectedChannels, DFTSystemResponse, ...
      NumberChannels, ChannelFilterLength, NumberOfAntennas, ch_quant_signal, ch_quant_inputfft, ch_quant_outputfft);
  if (CH_RFIblanking)
    NewDataLength = size(SelectedChannelSignals);   
    FlaggingCube = TimeFrequencyAnalyzer(SelectedChannelSignals, NewDataLength(2), NumberOfAntennas, ...
        length(SelectedChannels), TFAavg, TFAfreq);
    % Cancel the RFI by blanking the antenna signals with use of the blanking vector
    % return the cleaned selected subband signals matrix instead of the non clean one.
    CleanSelectedChannelSignals = CSCS(SelectedChannelSignals, FlaggingCube);
    SelectedChannelSignals = CleanSelectedChannelSignals;
  end
  
  save([dirpath '/antenna_signals.mat'], 'AntennaSignals','SelectedSubBandSignals','SelectedChannelSignals','BFSignals',...
      'DFTSystemResponse');
