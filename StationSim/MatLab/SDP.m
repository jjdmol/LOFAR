function SDP(DataLength, NumberOfAntennas, NumberOfSubbands, ...
             SubbandFilterLength, Averaging, Frequency)

% Initialize parameters
NumberOfPlot            = 1;

% Subbandselection vector
SubbandSelector         = [1:NumberOfSubbands];

% Calculate the system response for the filter banks
DFTSystemResponse = DFTFilterBankInitialization(SubbandFilterLength, NumberOfSubbands);

% start a timer
tic

% Generate representative data
[AntennaSignals, SOI, WithOut] = DataGenerator(DataLength, NumberOfAntennas, 1);

% Separate the antenna signals into subbands and select a number of them
SelectedSubbandSignals = SubbandSeparator(AntennaSignals, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                           SubbandFilterLength, NumberOfAntennas);
                                             
% Do a time frequency analysis on the generated data, and generate a blanking vector / matrix
NewDataLength = size(SelectedSubbandSignals);
FlaggingCube = TimeFrequencyAnalyzer(SelectedSubbandSignals, NewDataLength(2), NumberOfAntennas, length(SubbandSelector), Averaging, Frequency);

% Cancel the RFI by blanking the antenna signals with use of the blanking vector
CleanSelectedSubbandSignals = CSCS(SelectedSubbandSignals, FlaggingCube);

Results(SOI, WithOut, SubbandSelector, DFTSystemResponse, NumberOfSubbands, SubbandFilterLength, ...
        SelectedSubbandSignals, CleanSelectedSubbandSignals, NumberOfPlot, NumberOfAntennas,...
        NewDataLength, FlaggingCube);
             
% Stop timer
toc