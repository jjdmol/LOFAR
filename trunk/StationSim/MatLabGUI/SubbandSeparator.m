function [SelectedSubbandSignals] = SubbandSeparator(AntennaSignals, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                                   SubbandFilterLength, NumberOfAntennas, quant_signal, quant_inputfft,...
                                                   quant_outputfft)
% Split the antenna signals into subbands
for a = 1 : NumberOfAntennas
    SubbandAntennaSignals(a, :, :) = DFTFilterBankQuantised(AntennaSignals(a, :), quant_signal, quant_inputfft, quant_outputfft, ...
                                                            DFTSystemResponse, NumberOfSubbands, SubbandFilterLength, 100000000000000);
    
    %     *----------*
    %    /Sn        /|
    %   /.         / |      Tn = n-th time sample
    %  /.         /  |
    % *----------*   |      An = n-th Antenna
    % |A1......Tn|   *
    % |.         |  /       Sn = n-th Subband
    % |.         | /
    % |An        |/         How to index: (An, Tn, Sn)
    % *----------*
end


% Select a number of subbands 
for s = 1 : length(SubbandSelector)
    % This could fail. SubbandSelector(s) is not guaranteed to fit within the range of 
    % SubbandAntennaSignals().. Is this correct? If so, we should implement a range check
    % before entering the loop. If not, we need to correct the algorithm.
    %
    SelectedSubbandSignals(:, :, s) = SubbandAntennaSignals(:, :, s); 
end

