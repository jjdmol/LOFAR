% Generate input data for a Phased array
% nantl     Number of antennas in a 1D direction
% nsnsh     Number of snapshot
% nrfi      Number of RFI
% phi       Phi direction of looking
% theta     Theta direction of looking
% freqsource Frequency of the source
% phiRFI    Phi RFIs
% thetaRFI  Theta RFIs
% px        Placement of the antenna elements along the x axis
% py        Placement of the antenna elements along the y axis
% inr       Amplitude of the RFI signal 
% freq      Frequency of the RFI signal if 0<f<1 else Wide Band RFI

function sh = snapsh(nantl, nsnsh, nrfi, phi, theta, freqsource, phiRFI, thetaRFI, px, py, inr, freq)

sh=zeros(nantl,nsnsh);

for k=1:nsnsh
   for l=1:nrfi  
      sv = steerv(px,py,phiRFI(l),thetaRFI(l))/nantl;
      
      if (freq(l)>0)&(freq(l)<1)  % C(w) signal
          
        sh(:,k) = sh(:,k) + sqrt(inr(l)) * sv .* exp(2*pi*(freq(l)/2)*i*k);
      else                  % Wide band RFI
        inR(1:1:nantl)=randn;
        inI(1:1:nantl)=randn;
        in=inR+i*inI;
        sh(:,k) = sh(:,k) + 0.5*sqrt(2)*sqrt(inr(l))* sv .*in.';       
      end    
   end
   % Point source Cw signal
   sv = steerv(px,py,phi,theta)/nantl;
   if (freqsource>0)&(freqsource<1)  % C(w) signal
      sh(:,k) =  sh(:,k) + sv .* exp(2*pi*(freqsource/2)*i*k);
   else   
      inR(1:1:nantl)=randn;
      inI(1:1:nantl)=randn;
      in=inR+i*inI;
      sh(:,k) = sh(:,k) + 0.5*sqrt(2)* sv .*in.';  
   end
   %antenna noise
   sh(:,k) =  sh(:,k) + 0.5*sqrt(2)*(randn(nantl,1) + randn(nantl,1) * i);
end
