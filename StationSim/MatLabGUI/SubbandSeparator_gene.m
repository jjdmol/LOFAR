function [ SubbandAntennaSignals] = SubbandSeparator_gene(AntennaSignals, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                                   SubbandFilterLength, NumberOfAntennas,Filter_quant2,IS_Quant2,OS_Quant2)

quant_signal            = Filter_quant2;
quant_inputfft          = IS_Quant2;
quant_outputfft         = OS_Quant2;

% Split the antenna signals into subbands
for a = 1 : NumberOfAntennas
    SubbandAntennaSignals(:, :, a) = DFTFilterBankQuantised(AntennaSignals(a, :), quant_signal, quant_inputfft, quant_outputfft, ...
                                                            DFTSystemResponse, NumberOfSubbands, SubbandFilterLength, 100000000000000,2);
    
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
%for s = 1 : length(SubbandSelector)
 %   SelectedSubbandSignals(:, :, s) = SubbandAntennaSignals(:, :, SubbandSelector(s));
 %end

