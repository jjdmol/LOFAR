function SubbandSplitter

  dirpath='data';
  load([dirpath '/antenna_signals.mat']);
  load([dirpath '/antenna_config.mat']);
  load([dirpath '/subband_options.mat']);
  load([dirpath '/signal_options.mat']);

  DFTSystemResponse = DFTFilterBankInitialization(SubbandFilterLength, NumberSubBands);
  SelectedSubBandSignals = SubbandSeparator(AntennaSignals, SelectedSubBands, DFTSystemResponse, ...
      NumberSubBands, SubbandFilterLength, NumberOfAntennas, sb_quant_signal, sb_quant_inputfft, sb_quant_outputfft);

  FlaggingCube=1;
  if (RFIblanking)
    NewDataLength = size(SelectedSubBandSignals);   
           
     
    FlaggingCube = TimeFrequencyAnalyzer(SelectedSubBandSignals, NewDataLength(2), NumberOfAntennas, ...
        length(SelectedSubBands), TFAavg, TFAfreq);

    % Cancel the RFI by blanking the antenna signals with use of the blanking vector
    % return the cleaned selected subband signals matrix instead of the non clean one.
    CleanSelectedSubbandSignals = CSCS(SelectedSubBandSignals, FlaggingCube);
    SelectedSubBandSignals = CleanSelectedSubbandSignals;
  end
  
  save([dirpath '/antenna_signals.mat'], 'AntennaSignals','SelectedSubBandSignals','DFTSystemResponse','FlaggingCube');