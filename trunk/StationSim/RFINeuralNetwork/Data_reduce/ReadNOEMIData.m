
% purpose: read time sries NOEMI data files

clear all; close all;

% input of parameters
ch_table = [0:7];            % participating telescope channels
Nch = 1;
%length(ch_table);      % number of part. telescope channels 
Nread = 32768*1024/2;                  % no of samples to read per block (there are 2^24 samples
                             % for each of the 8 telescope channels)

file = ['c:\user\dromer\DataReaders\SpectrumDisk3\'];    % data file root 
% file = ['d:\disk1\ch2a'];    % data file root 
ADCrange = 2^11;             % ADC range, 12 bits total range (pos. and neg.)

% open the files as specified in ch_table
for ii = 1:Nch                                          % telescope channel counter
   filestr = [file '2wide103_108ch' int2str(ch_table(ii)) '.dat'];  % file name
   fid(ii) = fopen(filestr,'r');                        % file identifier
end

% reading data and spectral analysis
for ii = 1:Nch              % telescope channel counter
   [xtmp, L] = fread(fid(ii),Nread,'int16'); % read Nread data samples
   xtmp = xtmp ./ ADCrange; % (Nread.1) correct for ADC rang
   x(ii,:) = xtmp';
end 
% Closing data files
fclose('all');                            % close all open files


