function [Threshold] = Calibration(MeanPower, MedianPower, NumberOfSelectedSubbands)

% Plot the Mean power before calculation
figure(1);
subplot(3,1,1), plot(MeanPower);
ylabel('Intermittend and stationary RFI')
axis([1 NumberOfSelectedSubbands/2 0 0.00001]);

% Get rid of the intermittend RFI, make use of the fact that for Intermittend RFI the mean and the median 
% are different
IntermittendRFI = MeanPower - MedianPower;
for s = 1 : NumberOfSelectedSubbands / 2 
    if IntermittendRFI(s) > MedianPower(s)
        MeanPower(s) = MedianPower(s);
    end
end

% plot the results of the disposal of intermittend RFI
subplot(3,1,2), plot(MeanPower);
ylabel('Stationary RFI')
axis([1 NumberOfSelectedSubbands/2 0 0.00001]);

% Get rid of the stationar RFI by asuming that this isn't much in the singal so the median will give a go
%  measure as a threshold
MedianStationaryRFI = median(MeanPower) * 1.3;
for s = 1 : NumberOfSelectedSubbands / 2
    if MeanPower(s) > MedianStationaryRFI
        MeanPower(s) = MedianStationaryRFI;
    end
end

% plot the results of the mean calculation without RFI
subplot(3,1,3), plot(MeanPower);
ylabel('Without RFI')
axis([1 NumberOfSelectedSubbands/2 0 0.00001]);

% Calculate the threshold, this is done by adding  a safe offset to the mean. The offset in this 
% calculation is 6 times sigma (the standard deviation)
Variance = var(MeanPower);
Threshold = MeanPower + 6 * sqrt(Variance);
