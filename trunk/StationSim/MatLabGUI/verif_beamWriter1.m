clear all
subband=65;
load('C:\user\dromer\SDP_travail\StationGUI datagen branch\LofarSimDataGenerator\data\signal_output.txt')
number_data=128;
Nfft=128;
number_antenna=16;
data=signal_output(1:number_data);
data=repmat(data,1,number_antenna).';
FFT_subband(128,subband,subband,data);
load('data/antenna_signals_mod.mat');
fid=fopen('C:\user\dromer\SDP_travail\StationGUI datagen branch\LofarSimDataGenerator\data\verification.txt','w');
fprintf(fid,[num2str(number_antenna) '\n']);
fprintf(fid,'[');
fprintf(fid,'%6f\t',SelectedSubBandSignals.');
fprintf(fid,']');
fclose(fid);