function [ SubbandAntennaSignals] = SubbandSeparator_gene(AntennaSignals, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                                   SubbandFilterLength, NumberOfAntennas)

quant_signal            = str2num(get(Filter_quant2,'string'));
quant_inputfft          = str2num(get(IS_Quant2,'string'));
quant_outputfft         = str2num(get(OS_Quant2,'string'));

% Split the antenna signals into subbands
for a = 1 : NumberOfAntennas
    SubbandAntennaSignals(:, :, a) = DFTFilterBank(AntennaSignals(a, :), quant_signal, quant_inputfft, quant_outputfft, ...
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
%for s = 1 : length(SubbandSelector)
 %   SelectedSubbandSignals(:, :, s) = SubbandAntennaSignals(:, :, SubbandSelector(s));
 %end

