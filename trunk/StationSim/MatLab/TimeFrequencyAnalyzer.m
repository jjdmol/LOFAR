function [FlaggingCube] = TimeFrequencyAnalyzer(SelectedSubbandSignals, DataLength, NumberOfAntennas, NumberOfSelectedSubbands, Averaging, Frequency)

% go to the nearest possible averaging value
while mod(DataLength, Averaging) > 0
    Averaging = Averaging - 1;
end

% control the averaging en frequency.

for avr = 1 : DataLength / Averaging
    
    % Make a cube of signals of the length Averaging
    IntermediateSubbandSignals = SelectedSubbandSignals(:, (avr-1)*Averaging+1:(avr-1)*Averaging+Averaging, :);
        
    if mod(avr, Frequency) == 0 | avr == 1
        % Statistics that might be needed by calibration
		MeanPower = zeros(1, NumberOfSelectedSubbands / 2);
		MedianPower = zeros(1, NumberOfSelectedSubbands / 2);
		MaxPower = zeros(1, NumberOfSelectedSubbands / 2);
		MinPower = zeros(1, NumberOfSelectedSubbands / 2);
		VariancePower = zeros(1, NumberOfSelectedSubbands / 2);
		
		for a = 1 : NumberOfAntennas
            for s = 1 : NumberOfSelectedSubbands / 2
                Power = IntermediateSubbandSignals(a, :, s) .* conj(IntermediateSubbandSignals(a, :, s));
                MeanPower(s) = MeanPower(s) + mean(Power) / NumberOfAntennas;
                VariancePower(s) = VariancePower(s) + var(Power) / NumberOfAntennas;
                MedianPower(s) = MedianPower(s) + median(Power) / NumberOfAntennas;
                MaxPower(s) = MaxPower(s) + max(Power) / NumberOfAntennas;
                MinPower(s) = MinPower(s) + min(Power) / NumberOfAntennas;
            end
		end
	
        % Let calibration calculate a threshold
        Threshold = Calibration(MeanPower, MedianPower, NumberOfSelectedSubbands);
    end

    % Do a detection of RFI with the calculated threshold
    IntermediateFlaggingCube = Detector(IntermediateSubbandSignals, NumberOfAntennas, ...
                                        NumberOfSelectedSubbands, Averaging, Threshold);

    % Concatenate the intermediate flaggingcube to the total one
    if avr == 1
        FlaggingCube = IntermediateFlaggingCube;
    else
        FlaggingCube = [FlaggingCube IntermediateFlaggingCube];
    end
end
    