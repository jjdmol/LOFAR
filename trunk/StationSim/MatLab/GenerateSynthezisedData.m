function [AntennaSignals, SOI, WithOut] = GenerateSynthezisedData(DataLength, NumberOfAntennas)

%NumberOfRFI             = 134;                          % number of RFI frequency
AmplSOI                 = 0.4;                          % Ampl of coherent signal
AmplSystemNoise         = 1.0;                          % Amplitude of system noise at antenna
AmplRFI                 = 2.0;                          % Ampl of RFI 
NumberOfPlot            = 0;

%  RFI chirp parameters (0.0001)
AmplChirp               = 1.1; 
FreqChirp               = 0.33;
PhaseChirp              = 0.0001;

% chirp RFI
for t = 1 : DataLength
    RFIChirp(t) = AmplChirp * sin(2 * pi * t * FreqChirp +  PhaseChirp * t * t);  
end

% make a filter for the signal of interest and system noise and plot the system response
SOIFilterOrder          = 7;
SOIFilterCutOffFreq     = [0.1 0.9];
[SOIFilterWeightsB, SOIFilterWeightsA] = butter(SOIFilterOrder, SOIFilterCutOffFreq);
% [H_SOI, w] = freqz(SOIFilterWeightsB, SOIFilterWeightsA, 100);
% f = w / pi;
% H_SOI_magn = abs(H_SOI);

% make another filter for the RFI and plot the system response
RFIFilterOrder          = 7;
RFIFilterCutOffFreq     = [0.2 0.25];
[RFIFilterWeightsB, RFIFilterWeightsA] = butter(RFIFilterOrder, RFIFilterCutOffFreq);
% [H_RFI, w] = freqz(RFIFilterWeightsB, RFIFilterWeightsA, 100);
% f = w / pi;
% H_RFI_magn = abs(H_RFI);

for t = 1 : DataLength
    SOI(t) = randn;                     % this is the signal of interest (SOI)
    RFI(t) = randn;                     % this is the RFI at both antenna
    
    for a = 1 : NumberOfAntennas        % this is the system noise of the antenna  
        SystemNoiseAntennas(a, t) = randn; 
        %
        % *----------*
        % |A1......Tn|  How to index: (An, Tn)
        % |.         |
        % |.         |
        % |An        |
        % *----------*
    end
end

% main signals low-pass filtering, the signals are filtered with the filters that are constructed above
SOI = filter(SOIFilterWeightsB, SOIFilterWeightsA, SOI);
RFI = filter(RFIFilterWeightsB, RFIFilterWeightsA, RFI);           % line imitation

for a = 1 : NumberOfAntennas
    SystemNoiseAntennas(a, :) = filter(SOIFilterWeightsB, SOIFilterWeightsA, SystemNoiseAntennas(a, :));
end
	

% Write the generated antenna signals to a file
fdata = fopen('f:\generated_data.dat', 'w+');

% Here the signals of the antennas are constructed by the following formula: X(t) = Xsig(t) + Xsys(t) + Xrfi(t)
% there are two pairs, one with RFI and one pair without
for t = 1 : DataLength
    for a = 1 : NumberOfAntennas
        AntennaSignals(a, t) = SOI(t) * AmplSOI + SystemNoiseAntennas(a, t) * AmplSystemNoise + RFI(t) * AmplRFI + RFIChirp(t); % total signal 1
        fprintf(fdata, '%20.16e ', AntennaSignals(a, t));
        WithOut(a, t) = SOI(t) * AmplSOI + SystemNoiseAntennas(a, t) * AmplSystemNoise;
    end
    fprintf(fdata, '\n');
end

%%%%%%%%%%%%% Calculate the performance indicators %%%%%%%%%%%%
RFItotal = RFI + RFIChirp;
VarRFI = var(RFItotal).^2;
VarSys = var(SystemNoiseAntennas(1,:)).^2;
Qint = VarSys / VarRFI

% Qband = SOI bandwidth / RFI bandwidth

fclose(fdata);