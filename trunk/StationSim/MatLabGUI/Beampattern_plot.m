function [BeamPattern]=Beampattern_plot(EvectorSVD)
rfi_sources_SVD=size(EvectorSVD,2)
load('data/antenna_config.mat');
phi_look=0.5;
theta_look=0.5;
LookingDirection=steerv(px,py,phi_look,theta_look);
WeightVector = awe(EvectorSVD, 1, LookingDirection,rfi_sources_SVD, length(px),LookingDirection,1);
%Beampattern Obtention
N=100;
phi_theta=[];
phi_theta=[];
pat_steered=zeros(N,N);
pat_nulled=zeros(N,N);
pat_nulled_SVD=zeros(N,N);
relfreq=1;
i=0;
fin=N*N;
 time = 0
  for u = 0:N
     u1 = -1 + 2*u/N;
     for v = 0:N
         v1 = -1 + 2*v/N;
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi = atan2(u1,v1);
             phi_theta=[phi_theta; phi theta];
             S{u+1,v+1}=[phi,theta];
             if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end
    
              pat_nulled(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector.')';

         end;
     end;
 end


BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));

%Plotting of the trajectory for EVD during snap_length
figure
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
