function [FlaggingCube] = Thresholding(SelectedSubbandSignals, NumberOfAntennas, NumberOfSelectedSubbands, ...
                                   DataLength, Threshold)
                               
% ***************** Thresholding *************************
% calculate the power spectrum of antennas
for a = 1 : NumberOfAntennas
    for s = 1 : NumberOfSelectedSubbands
        PowerSpectrumSubbands(a, :, s) = SelectedSubbandSignals(a, :, s) .* conj(SelectedSubbandSignals(a, :, s));
    end
end

% threshold processing
for a = 1 : NumberOfAntennas   
    for s = 1 : NumberOfSelectedSubbands        
		for t = 1 : DataLength
            if (PowerSpectrumSubbands(a, t, s) > Threshold(mod(s - 1, NumberOfSelectedSubbands/2) + 1))
                FlaggingCube(a, t, s) = 0.0000000001; %Why does Peter a multiplication with a small figure?
            else 
                FlaggingCube(a, t, s) = 1;
			end 
        end  
    end
end
