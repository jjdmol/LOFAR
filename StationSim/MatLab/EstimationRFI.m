function [FlaggingCube] = EstimationRFI(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
                                   DataLength, Threshold)

% **************** Estimation ****************************
Threshold = 0.0001;
% calculate the power spectrum of antennas
for a = 1 : NumberOfAntennas
    for s = 1 : NumberOfSelectedSubbands
        PowerSpectrumSubbands(a, :, s) = SelectedSubbandSignals(a, :, s) .* conj(SelectedSubbandSignals(a, :, s));
    end
end

% Calculate the RFI estimate
for a = 1 : NumberOfAntennas
    for s = 1 : NumberOfSelectedSubbands
        RFIestimate(a, :, s) = PowerSpectrumSubbands(a, :, s) ./ (PowerSpectrumSubbands(a, :, s) + Threshold);
    end
end

% threshold processing
for a = 1 : NumberOfAntennas   
    for s = 1 : NumberOfSelectedSubbands        
		for t = 1 : DataLength
            FlaggingCube(a, t, s) = 1 - RFIestimate(a, t, s);
        end  
    end
end
