% plot the beam in the direction of looking 
% the RFI are nulls after detection with the eigenvector deconposition
% level is a treshold for detection of the eigenvalues more than > LEVEL 

function genebeampattern(px,py,dirpath)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001

% Adapted by Chris Broekema july 2002

      
    load([dirpath 'data/antenna_signals.mat']);
    load([dirpath 'data/rfi_eigen.mat']);
    %
    signal_freq=0.5;
    digwindow=1;
    relfreq=signal_freq*2;
    
    %
    % Generate Digital beamforming window 
    %
    digwin = taper(px, py, digwindow, 128);
    i = 0;
    U = 0;
    V = 0;
    N=91;
    time = 0
    
% Generate beam pattern of the array
disp(['Array pattern for the center_frequency :' num2str(signal_freq)]);
BeamSignals =[];

  for u = 0:N
     u1 = (-1 + 2*u/N);
     for v = 0:N
         v1 = (-1 + 2*v/N);
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             theta = asin(sqrt(u1^2+v1^2));
             phi= atan2(u1,v1);
             U(i) = u;
             V(i) = v;
             pat(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(phi)*sin(theta)+py*sin(phi)*sin(theta)))*WeightVector;
         end;
     end;
end;
BeamSignals = abs(pat)/max(max(abs(pat)));



%Plot the beampattern in the angle coordinates
patstep=4;
patend=90;
a=[-patend:patstep/2:patend]*pi/180;
b=[-patend:patstep/2:patend]*pi/180;
for k=1:length(a)
    for j=1:length(b)
        pattern(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(a(j))*sin(b(k))+py*sin(a(j))*sin(b(k))))*digwin;
    end;
end;
Rect_pattern_nulled=abs(pattern)/max(max(abs(pattern)));


save([dirpath 'data/beam_pattern.mat'],'digwin','BeamSignals','a','b','Rect_pattern_nulled','patend');
