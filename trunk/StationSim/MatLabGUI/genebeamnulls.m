% plot the beam in the direction of looking 
% the RFI are nulls after detection with the eigenvector deconposition
% level is a treshold for detection of the eigenvalues more than > LEVEL 
function genebeamnull(dirpath,px,py,N)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001

      
  load([dirpath 'data/antenna_signals.mat']);
  load([dirpath 'data/rfi_eigen.mat']);
  load([dirpath 'data/beam_pattern.mat']);
  
  disp('Building steered and adapted beampattern...') 
  kl=size(WeightVector,2);
  pat_steered=[];
  pat_nulled=zeros(N,N,size(WeightVector,2)); 
  signal_freq=0.5;
  digwindow=1;
  relfreq=signal_freq+0.5;
  i=0;

    
  % Generate Digital beamforming window 
  digwin = taper(px, py, digwindow, 128);

  %Number of RFI sources    
  kl=size(WeightVector,2);
  fin=N*N;
  %for kl=1:size(WeightVector,2)

 
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
             pat_steered(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(LookingDirection.')';
           
             pat_nulled(u+1,v+1,:) = exp(-1*sqrt(-1)*2*pi* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*((squeeze(WeightVector(:,:,kl))).')';
              if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end
         end;
     end;
 end

 for kl=1:size(WeightVector,2)
 BeamPattern(:,:,kl) = abs(pat_nulled(:,:,kl))/max(max(abs(pat_nulled(:,:,kl))));
 end
 
 %Plot the beampattern in the angle coordinates
 patstep=4;
 a=[-patend:patstep/2:patend]*pi/180;
 b=[-patend:patstep/2:patend]*pi/180;

 for k=1:length(a)
      for j=1:length(b)
             pattern(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(a(j))*sin(b(k))+py*sin(a(j))*sin(b(k))))*(LookingDirection.')';
             pattern_null(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*cos(a(j))*sin(b(k))+py*sin(a(j))*sin(b(k))))*(WeightVector.')';
      end;
 end;
 Rect_pattern_steered=abs(pattern)/max(max(abs(pattern)));
 Rect_pattern_null=abs(pattern_null)/max(max(abs(pattern_null)));
                
    
 OldPattern=BeamPattern;
 SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
 BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));
 save([dirpath 'data/beam_pattern.mat'],'digwin','BeamPattern','N','OldPattern','SteeredPattern','patend','Rect_pattern_steered','Rect_pattern_null','a','b');