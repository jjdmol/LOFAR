function [AntennaSignals, SOI, WithOut] = DataGenerator(DataLength, NumberOfAntennas, Type, NumberOfSubbands, SubbandFilterLength)

switch Type
case 1
	% Generate synthezised data with RFI, SOI is the signal of interest, and WithOut is SOI and system noise.
	[AntennaSignals, SOI, WithOut] = GenerateSynthezisedData(DataLength, NumberOfAntennas);

case 2
    % Read in the data form a file
	AntennaSignals = load('f:\generated_data.dat');
	AntennaSignals = AntennaSignals';
    
case 3
    % put a pulse on the input
    AntennaSignals = zeros(NumberOfAntennas, DataLength);
    pulse_offset = (SubbandFilterLength + 1) * NumberOfSubbands;
    AntennaSignals(:, pulse_offset + [1 : NumberOfSubbands]) = ones(NumberOfAntennas, NumberOfSubbands);
    %AntennaSignals(:, 4 + NumberOfSubbands * 7) = 1;
    
    % Write the generated antenna signals to a file
    fdata = fopen('f:\generated_data.dat', 'w+');
    for t = 1 : DataLength
        for a = 1 : NumberOfAntennas
            fprintf(fdata, '%20.16e ', AntennaSignals(a, t));
        end
        fprintf(fdata, '\n');
    end
    fclose(fdata);
    
case 4
    % put a sine on the input
    freq = 0.1;
    ampl = 1;
    
    AntennaSignals = zeros(NumberOfAntennas, DataLength);
    
    for a = 1 : NumberOfAntennas
        for t = 1 : DataLength
            AntennaSignals(a, t) = ampl * sin(2 * pi * freq * t);
        end
    end
    
    % Write the generated antenna signals to a file
    fdata = fopen('f:\generated_data.dat', 'w+');
    for t = 1 : DataLength
        for a = 1 : NumberOfAntennas
            fprintf(fdata, '%20.16e ', AntennaSignals(a, t));
        end
        fprintf(fdata, '\n');
    end
    fclose(fdata);    
end