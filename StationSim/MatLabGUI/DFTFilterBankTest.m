function DFTFilterBankTest

dirpath='data\';

load([dirpath 'dft_filterbank_testset.mat']);

NumberOfAntennas=1;
SubbandFilterLength=16;
NumberSubBands=128;

%%%%
% Sub band splitter
%%%%

sb_step=size(AntennaSignals,2) / NumberSubBands;
SelectedSubBands=zeros(1,NumberSubBands);
for i=1:NumberSubBands
    SelectedSubBands(1,i) = i;
end

sb_quant_signal=32;
sb_quant_inputfft=32;
sb_quant_outputfft=32;

DFTSystemResponse = DFTFilterBankInitialization(SubbandFilterLength, NumberSubBands);
SelectedSubBandSignals = SubbandSeparator(AntennaSignals, SelectedSubBands, DFTSystemResponse, ...
    NumberSubBands, SubbandFilterLength, NumberOfAntennas, sb_quant_signal, sb_quant_inputfft, sb_quant_outputfft);

answer1(1,:)=SelectedSubBandSignals(1,8,:);

figure(1);
plot(abs(answer1));


%%%%
% Channel Splitter
%%%%

ChannelFilterLength=16;
NumberChannels=128;

ch_quant_signal=32;
ch_quant_inputfft=32;
ch_quant_outputfft=32;

% variables for the time frequency analyzer.
TFAavg=1;
TFAfreq=5;

ch_step=size(SelectedSubBandSignals,2);
SelectedChannels=zeros(1,NumberChannels);
for i=1:NumberSubBands
    % only the first x channels are selected
    SelectedChannels(1,i) = i;
end

DFTSystemResponse = DFTFilterBankInitialization(ChannelFilterLength, NumberChannels);
SelectedChannelSignals = SubbandSeparator(SelectedSubBandSignals, SelectedChannels, DFTSystemResponse, ...
   NumberChannels, ChannelFilterLength, NumberOfAntennas, ch_quant_signal, ch_quant_inputfft, ch_quant_outputfft);

% Now do a time frequency analysis
NewDataLength = size(SelectedChannelSignals);   
FlaggingCube = TimeFrequencyAnalyzer(SelectedChannelSignals, NewDataLength(2), NumberOfAntennas, ...
    length(SelectedChannels), TFAavg, TFAfreq);
    % Cancel the RFI by blanking the antenna signals with use of the blanking vector
    % return the cleaned selected subband signals matrix instead of the non clean one.
CleanSelectedChannelSignals = CSCS(SelectedChannelSignals, FlaggingCube);
SelectedChannelSignals = CleanSelectedChannelSignals;

answer2(1,:)=SelectedChannelSignals(1,8,:);

figure(2)
plot(abs(answer2))

% It may be a good idea to print a succes or failure message depending on the answers found.


% Alex's code to visualize the results from the DFT filterbank.
%
% Note that this requires 3 data objects: SOI, which is only the signal of interest,
% WithOut, SOI with system noise, but without RFI's and the normal AntennaSignals.
% 
% NumberOfPlot=25;
%    
% SOI=AntennaSignals;
% WithOut=AntennaSignals;
% Results(SOI, WithOut, SelectedChannels, DFTSystemResponse, NumberChannels, ChannelFilterLength, ...
%     SelectedChannelSignals, CleanSelectedChannelSignals, NumberOfPlot, NumberOfAntennas,...
%     NewDataLength, FlaggingCube);   
