close all;
clear all
load('data\antenna_config.mat');
load('data\Extraction_2source.mat');
% Nfft=128;
% nrsb_begin=65;
% nrsb_end=65;
% load('data/antenna_signals.mat');
% data=AntennaSignals;
% number_antenna=size(data,1);
%fft_subband(Nfft,nrsb_begin, nrsb_end,data);
%load('data/antenna_signals_mod.mat');
A=[px' py']
Phi_theta=[];
for int=1:2
%phi1
%theta1
% Phi1=Phi_theta(int:100,1,1);
% Theta1=Phi_theta(int:100,2,1);
% Phi2=Phi_theta(int:100,1,2);
% Theta2=Phi_theta(int:100,2,2);
% Phi3=Phi_theta(int:100,1,3);
% Theta3=Phi_theta(int:100,2,3);
% Phi4=Phi_theta(int:100,1,4);
% Theta4=Phi_theta(int:100,2,4);
% Phi=[Phi1 Phi2 Phi3 Phi4];
% Theta=[Theta1 Theta2 Theta3 Theta4];
vec=fliplr(EvectorEVD);
number_antenna=length(px);
B=angle(vec(:,int));%(ones(number_antenna,1)*unwrap(angle(vec(1,int)))+unwrap()/(2*pi)
vec_check(:,1)=exp(i*2*pi*(px*cos(phi1)*sin(theta1)+py*sin(phi1)*sin(theta1))).';
vec_check(:,2)=exp(2*pi*i*(px*cos(phi2)*sin(theta2)+py*sin(phi2)*sin(theta2))).';
X=(B.')/A.';
phi=atan(X(2)/X(1));
theta=asin(X(1)/cos(phi));
Phi_theta(int,:)=[phi theta];
%[x,y,z]=sph2cart(phi,pi/2-theta,5);
% figure(1)
% hold on
% plot3(mean(px),mean(py),0.2,'*g')
% plot3(x,y,z,'*b')
% plot3(px,py,0.2*ones(length(px)),('r*'))
% line([mean(px) x], [mean(py) y],[0.2 z])
% hold off
%axis([-max(px)*5 max(px)*5 -max(py)*5 max(py)*5 0 10])
% switch array_type
%    case 1
%     title(' Compact THEA');
%     disp(' Compact THEA');  
%    case 2
%     title(' Compact THEA, but now with triangular lattice structure');
%     disp(' Compact THEA, but now with triangular lattice structure');
%    case 3
%     title(' 16 element array in one ring '); 
%     disp(' 16 element array in one ring ')
%    case 4  
%     title('16 element array with two rings of 11 and 5');
%     disp(' 16 element array with two rings of 11 and 5');
%    case 8
%     title(' 16 element array with three rings of 11, 4, 1');  
%     disp(' 16 element array with three rings of 11, 4, 1');
%    case 9
%      title(' Y shaped array (VLA)');  
%     disp(' Y shaped array (VLA)');
%    case 10
%      title(' Spiral shaped array (3 arms)');  
%     disp(' Spiral shaped array (3 arms)');
%    case 11
%       title(' Spiral shaped array (4 arms)');
%     disp(' Spiral shaped array (4 arms)');
%    case 12
%            title('logarithmic array (4x4)');
%     disp('logarithmic array (4x4)');
%    case 13
%         title('logarithmic circle');   
%     disp('logarithmic circle');
%    case 14
%       title('logarithmic spiral');
%     disp('logarithmic spiral');
%    case 15
%        title(' two logarithmic circle ');
%     disp(' two logarithmic circle ');
%    case 16
%        title(' three logarithmic circle');
%     disp(' three logarithmic circle');
%    case 17
%       title(' three pentagones'); 
%     disp(' three pentagones');
%    case 18
%        title(' 8*8 THEA Tile ')
%     disp(' 8*8 THEA Tile ')
%    case 19
%         title(' LOFAR array of 5 circles with logarithmic radius ');
%     disp(' LOFAR array of 5 circles with logarithmic radius ');
%    case 20
%         disp('LOFAR array of 7 circles with logarithmic radius'); 
%     disp('LOFAR array of 7 circles with logarithmic radius'); 
%    case 21
%         title('logarithmic circle');
%     disp('logarithmic circle');
%    case 22
%         title('Triangular lattice in middle');
%     disp('Triangular lattice in middle');
%    case 23 
%        title('single antenna, slightly offset from center')
%     disp('single antenna, slightly offset from center')
%    case 24
%          title('regular array with two antennas, spaced one lambda apart')
%     disp('regular array with two antennas, spaced one lambda apart')
%    case 25 
%         title('regular array with six antennas, spaces two lambda apart')
%     disp('regular array with six antennas, spaces two lambda apart')
%    case 26 
%        disp('regular array with nine antennas, spaces four lambda apart')
%     disp('regular array with nine antennas, spaces four lambda apart')
%    case 27 
%         title('regular array with six antennas, spaced six lambda apart')
%     disp('regular array with six antennas, spaced six lambda apart')
%    case 28 
%         title('square array, four elements, spaced four lambda apart')
%     disp('square array, four elements, spaced four lambda apart')
%    case 29 % 
%         title('rectangular array 4x8 elements, spaced 2 lambda apart');
%     disp('rectangular array 4x8 elements, spaced 2 lambda apart');
%    case 30 
%         title('rectangular array 4x8 elements, spaced 8 lambda apart');
%     disp('rectangular array 4x8 elements, spaced 8 lambda apart');
%    case 31
%         title(' 16 element array with two rings of 11 and 5');
%       disp(' 16 element array with two rings of 11 and 5');
%   case 32
%         title('regular array two antennas');
%       disp('regular array two antennas');
%    end
end


figure
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*r')
% plot(cos(phi3)*sin(theta3)/Nfft,sin(phi3)*sin(theta3)/Nfft,'*r')
% plot(cos(phi4)*sin(theta4)/Nfft,sin(phi4)*sin(theta4)/Nfft,'*r')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
for i=1:1
plot(cos(Phi_theta(i,1))*sin(Phi_theta(i,2))/Nfft,sin(Phi_theta(i,1))*sin(Phi_theta(i,2))/Nfft,'*m')
end
hold off
title('EVD using the first hundred data') 
caxis([-70 0]);
colorbar 
Input=[phi1 theta1; phi2 theta2]
Output=Phi_theta



