function [FlaggingCube] = Detector(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
                                   DataLength, Threshold)
                             
FlaggingCube = Thresholding(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
                                   DataLength, Threshold);
                               
%FlaggingCube] = CumulativeSum(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
%                                   DataLength, Threshold);                               
                               
%FlaggingCube] = EstimationRFI(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
%                                   DataLength, Threshold)