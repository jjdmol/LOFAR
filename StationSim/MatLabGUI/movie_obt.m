clear all
tic
disp('Load the files corresponding to each sourcesgui');
disp('source : 1...');
%load('data2/signal_mod1.txt');
load('Matlab_Dat_Files/am_data.dat');
disp('source : 2...');
load('Matlab_Dat_Files/FM_data.dat');
%load('data2/signal_mod2.txt');
% disp('source : 3...');
% load('data2/signal_mod3.txt');
% disp('source : 4...');
% load('data2/signal_mod4.txt');
[px,py]=create_lofar;
Nfft=64;
bandwidth=100000;
long_file=floor(length(Matlab_Dat_Files_am_data)/Nfft);
total_time=long_file/bandwidth*Nfft;
signal_mod=[Matlab_Dat_Files_am_data.';Matlab_Dat_Files_FM_data.'];%;data2_signal_mod3.';data2_signal_mod4.'];
clear signal_mod1;
clear signal_mod2;
clear signal_mod3;
clear signal_mod4;
signal_mod=signal_mod(:,1:long_file*Nfft);

matrice(:,:,1)=[0,0, 0,0;...  
   1,1,2,1]

matrice(:,:,2)=[1,1, 0,0;...  
   1,1,0,1];


matrice(:,:,3)=[-1 , 1, 0,0;...  
      1.2 , 1.2, 3,1];  
% 
% matrice(:,:,4)=[1.2 , 1.3, 0,0;...  
%    1.2,1.3,0,1];
%  

%load('data/antenna_config.mat');
Phi_theta=[];
NumberSubBands=64;
disp('Trajectory Computation...');
for i=1:size(matrice,3);
phi_theta=interpolation(squeeze(matrice(:,:,i)),Nfft,bandwidth,total_time);
%interpolation(mat,Nfft,frequence,Total_Time);
if size(phi_theta,1)<size(Phi_theta,1)
Phi_theta=Phi_theta(1:size(phi_theta,1),:,:);
end
if (i~=1)&(size(Phi_theta,1)<size(phi_theta,1))
phi_theta=phi_theta(1:size(Phi_theta,1),:,:);
end
Phi_theta(:,:,i)=phi_theta;
end
save('data/trajectory_Phi_theta.mat','Phi_theta');
%Phasing and addition
disp('Phasing and addition Computation...');
AntennaSignals=zeros(length(px),min([size(Phi_theta,1)*Nfft,size(signal_mod,2)]));
for i=1:size(signal_mod)
sh = nband_siggen4(length(px), bandwidth, Nfft, px, py,signal_mod(i,:),Phi_theta(:,:,i).');
AntennaSignals = sh + AntennaSignals;
end
clear signal_mod;
clear Phi_theta;
SubbandFilterLength=12;
option_polyphase='shift';
SelectedSubBands=1:NumberSubBands;
%fft_subband(NumberSubBands,SelectedSubBands,AntennaSignals);
disp('Polyphase filter...');
[Result]=polyphase_antenna(AntennaSignals,NumberSubBands,SubbandFilterLength,NumberSubBands,option_polyphase,SelectedSubBands);
%polyphase_Antenna(AntennaSignals,NumberSubBands,SubbandFilterLength,NumberSubBands,option_polyphase,SelectedSubBands);
toc

