close all;
%clear all;
%load('matlab.mat');
load('data/antenna_config.mat');
switch array_type
   case 1
    chain=' Compact THEA';
    disp(' Compact THEA');  
   case 2
    chain=(' Compact THEA, but now with triangular lattice structure');
    disp(' Compact THEA, but now with triangular lattice structure');
   case 3
    chain=(' 16 element array in one ring '); 
    disp(' 16 element array in one ring ')
   case 4  
    chain=('16 element array with two rings of 11 and 5');
    disp(' 16 element array with two rings of 11 and 5');
   case 8
    chain=(' 16 element array with three rings of 11, 4, 1');  
    disp(' 16 element array with three rings of 11, 4, 1');
   case 9
     chain=(' Y shaped array (VLA)');  
    disp(' Y shaped array (VLA)');
   case 10
     chain=(' Spiral shaped array (3 arms)');  
    disp(' Spiral shaped array (3 arms)');
   case 11
      chain=(' Spiral shaped array (4 arms)');
    disp(' Spiral shaped array (4 arms)');
   case 12
           chain=('logarithmic array (4x4)');
    disp('logarithmic array (4x4)');
   case 13
        chain=('logarithmic circle');   
    disp('logarithmic circle');
   case 14
      chain=('logarithmic spiral');
    disp('logarithmic spiral');
   case 15
       chain=(' two logarithmic circle ');
    disp(' two logarithmic circle ');
   case 16
       chain=(' three logarithmic circle');
    disp(' three logarithmic circle');
   case 17
      chain=(' three pentagones'); 
    disp(' three pentagones');
   case 18
       chain=(' 8*8 THEA Tile ')
    disp(' 8*8 THEA Tile ')
   case 19
        chain=(' LOFAR array of 5 circles with logarithmic radius ');
    disp(' LOFAR array of 5 circles with logarithmic radius ');
   case 20
        chain=('LOFAR array of 7 circles with logarithmic radius'); 
    disp('LOFAR array of 7 circles with logarithmic radius'); 
   case 21
        chain=('logarithmic circle');
    disp('logarithmic circle');
   case 22
        chain=('Triangular lattice in middle');
    disp('Triangular lattice in middle');
   case 23 
       chain=('single antenna, slightly offset from center')
    disp('single antenna, slightly offset from center')
   case 24
         chain=('regular array with two antennas, spaced one lambda apart')
    disp('regular array with two antennas, spaced one lambda apart')
   case 25 
        chain=('regular array with six antennas, spaces two lambda apart')
    disp('regular array with six antennas, spaces two lambda apart')
   case 26 
       chain=('regular array with nine antennas, spaces four lambda apart')
    disp('regular array with nine antennas, spaces four lambda apart')
   case 27 
        chain=('regular array with six antennas, spaced six lambda apart')
    disp('regular array with six antennas, spaced six lambda apart')
   case 28 
        chain=('square array, four elements, spaced four lambda apart')
    disp('square array, four elements, spaced four lambda apart')
   case 29 % 
        chain=('rectangular array 4x8 elements, spaced 2 lambda apart');
    disp('rectangular array 4x8 elements, spaced 2 lambda apart');
   case 30 
        chain=('rectangular array 4x8 elements, spaced 8 lambda apart');
    disp('rectangular array 4x8 elements, spaced 8 lambda apart');
   case 31
        chain=(' 16 element array with two rings of 11 and 5');
      disp(' 16 element array with two rings of 11 and 5');
  case 32
        chain=('regular array two antennas');
      disp('regular array two antennas');
   end

Nfft=128;
nrsb_begin=65;
nrsb_end=65;
%data=ss_dg_output(1:size(ss_dg_output,1),:).';
%load('data/antenna_signals.mat');
%data=AntennaSignals;
%fft_subband(Nfft,nrsb_begin, nrsb_end,data);
load('data/antenna_signals_mod.mat');
A=[px' py']
phi_end=[];
theta_end=[];
compteur=1;
angle_final=[];
number_antenna=size(data,1);
number_data=floor(size(data,2)/Nfft);
trajectory=[];
X_fin=[];
for int=1:number_data
B=[];
B=(ones(number_antenna,1)*unwrap(angle(SelectedSubBandSignals(1,int)))-unwrap(angle(SelectedSubBandSignals(:,int))))/(2*pi);
angle_final=[angle_final,B];
X=(Nfft*B.')/A.';
phi=atan(X(2)/X(1));
phi_end=[phi_end phi];
 theta=asin(X(1)/cos(phi));
 theta_end=[theta_end theta];
 [x,y,z]=sph2cart(theta,(pi/2-phi),5);
 figure
 hold on
 plot3(0,0,0,'*g')
 plot3(x,y,z,'*b')
 plot3(px,py,0*ones(length(px)),('r*'))
 line([0 x], [0 y],[0 z])
 trajectory(:,:,int)=[x,y,z];
 if (int > 1)
 for i=2:int
 line([trajectory(1,1,i-1) trajectory(1,1,i)],[trajectory(1,2,i-1) trajectory(1,2,i)],[trajectory(1,3,i-1) trajectory(1,3,i)])
 end
 end
 hold off
 axis([-20 20 -20 20 0 8])
 title(chain)
 mov(int) = getframe(gcf);
end
movie2avi(mov,'moving_trajectory3')
solution=[phi_end.' theta_end.']
%close all