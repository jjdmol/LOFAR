function StationSimVerification(DataLength, NumberOfAntennas, NumberOfSubbands, ...
             SubbandFilterLength)

% Initialize parameters
NumberOfPlot            = 1;

% Subbandselection vector
SubbandSelector         = [1:NumberOfSubbands];

% Calculate the system response for the filter banks
DFTSystemResponse = DFTFilterBankInitialization(SubbandFilterLength, NumberOfSubbands);

% Generate representative data
AntennaSignals = DataGenerator(DataLength, NumberOfAntennas, 2, NumberOfSubbands, SubbandFilterLength);

% Separate the antenna signals into subbands and select a number of them
SelectedSubbandSignals = SubbandSeparator(AntennaSignals, SubbandSelector, DFTSystemResponse, NumberOfSubbands, ...
                                           SubbandFilterLength, NumberOfAntennas);

SelectedSubbandSignalsStationSim = ReadSelectedSubbandSignals('f:\bandsep.out', NumberOfSubbands, DataLength, ...
                                                   SubbandFilterLength, NumberOfAntennas);

%%%%%%%%%%%%%%%%%%%%%%%%% Temp quick result view %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure(NumberOfPlot)
Tn = DataLength / NumberOfSubbands - SubbandFilterLength + 1;
for t = 1 : Tn
    for s = 1 : NumberOfSubbands
        PowSpec(t, s) = SelectedSubbandSignals(1, t, s) .* conj(SelectedSubbandSignals(1, t, s));
%        PowSpec(t, s) = SelectedSubbandSignals(1, t, s);
    end
end

for t = 1 : DataLength / NumberOfSubbands - SubbandFilterLength + 1;
    for s = 1 : NumberOfSubbands
        PowSpecStationSim(t, s) = SelectedSubbandSignalsStationSim(1, t, s) .* conj(SelectedSubbandSignalsStationSim(1, t, s));
%        PowSpecStationSim(t, s) = SelectedSubbandSignalsStationSim(1, t, s);        
    end
end

% Normalize
% PowSpecStationSim = PowSpecStationSim / max(max(PowSpecStationSim));
% PowSpec = PowSpec / max(max(PowSpec));

% PowDifference = PowSpec((1 : 19), :) - PowSpecStationSim((2 : 20), :);
PowDifference = PowSpec(:, :) - PowSpecStationSim(:, :);
% PowDifference = PowSpec((1 : 10), :) - PowSpecStationSim((12 : 21), :);
% PowDifference = PowSpec((1 : 9), :) - PowSpecStationSim((13 : 21), :);

subplot(3,1,1),surf(PowSpec)
colormap('default');
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Subband separation by Matlab')
view(20,45)
%view(0,90)

subplot(3,1,2),surf(PowSpecStationSim)
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Subband separation by StationSim')
view(20,45)
%view(0,90)

subplot(3,1,3),surf(PowDifference)
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Difference between the two impl.')
view(20,45)
%view(0,90)

NumberOfPlot = NumberOfPlot + 1;
figure(NumberOfPlot)
subplot(3,1,1),surf(PowSpec)
colormap('default');
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Subband separation by Matlab')
view(0,90)

subplot(3,1,2),surf(PowSpecStationSim)
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Subband separation by StationSim')
view(0,90)

subplot(3,1,3),surf(PowDifference)
shading interp;
xlabel('frequency')
ylabel('time')
%axis([1 NumberOfSubbands/2  1 Tn])
zlabel('Difference between the two impl.')
view(0,90)

NumberOfPlot = NumberOfPlot + 1;
figure(NumberOfPlot)
subplot(3, 1, 1), plot([1 : Tn], SelectedSubbandSignals(1, :, 1))
subplot(3, 1, 2), plot([1 : Tn], SelectedSubbandSignalsStationSim(1, :, 1))
subplot(3, 1, 3), plot([1 : Tn], SelectedSubbandSignals(1, :, 1) - SelectedSubbandSignalsStationSim(1, :, 1))

% SelectedSubbandSignals(1, :, 1)
% SelectedSubbandSignalsStationSim(1, :, 1)
% SelectedSubbandSignals(1, :, 1) - SelectedSubbandSignalsStationSim(1, :, 1)