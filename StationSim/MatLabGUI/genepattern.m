% Generate a beam pattern according to the selected antenna configuration 
% and antenna signals

function genepattern(px,py,Beam_Phi_theta,dirpath,N)
 
      
%Taper window 

      digwin = taper(px, py, 1, 128);

     % Generate beam pattern in the cartesian coordinates
      relfreq=1;
      i = 0;
      U = 0;
      V = 0;
      pat=[];
      fin=N*N;
      
      for u = 0:N
         u1 = (-1 + 2*u/N);
         for v = 0:N
         v1 = (-1 + 2*v/N);
             if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi= atan2(u1,v1);
             pat(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(phi)*sin(theta)+py*sin(phi)*sin(theta)))*digwin;
             if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end
             end;
         end;
       end;
       BeamPattern=[];
       BeamPattern = abs(pat)/max(max(abs(pat)));
      
       %built the beampattern in angle coordinates    
       patend=90;
       patstep=4;
       a=[-patend:patstep/2:patend]*pi/180;
       b=[-patend:patstep/2:patend]*pi/180;
       for k=1:length(a)
             for j=1:length(b)
                   pattern(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(a(j))*sin(b(k))+py*sin(a(j))*sin(b(k))))*digwin;
             end;
       end;
       Rect_pattern=abs(pattern)/max(max(abs(pattern)));

       save([dirpath 'data/' 'beam_pattern.mat'],'digwin','N','BeamPattern','Rect_pattern','a','b','patend');