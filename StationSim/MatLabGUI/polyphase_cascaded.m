function [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2,option)
%
%   [y,z]=polyphase_cascaded(x,K1,L1,M1,K2,L2,M2,N2,option)
%
%
%   x   - input signal
%   K1  - number of channels after first filter bank
%   L1  - number of filter taps in first filter bank (compares to L*K filter taps in a classical FIR filter)
%   M1  - decimation factor (should be same as K1)
%   K2  - number of channels after second filter bank
%   L2  - number of filter taps in second filter bank (compares to L*K filter taps in a classical FIR filter)
%   M2  - decimation factor (should be same as K2)
%   N2  - number of channels that or actually used after the second filter bank
%
%   y   - output of the first filterbank
%   z   - output after the second filterbank for all input channels
%
%   z   is normalized w.r.t. the maximum
%
% (C) 2003, Astron. By M. van Veelen

  if nargin<9 ; option='keep' ; end ;
  
  y=polyphase(x,K1,L1,M1,'shift') ; y=y ./ max(max(abs(y))) ; %columns are subbands

  if strcmp(option,'cut') ;  dN2=floor((K2-N2)/2) ; else N2=K2; dN2=0; end;

  n_z = floor(length(y)/K2)-L2+1;
  z=zeros( n_z, N2*K1 );
  for k=1:K1
    z_range=N2*(k-1)+[1:N2];
    size(1+dN2:K2-dN2) ;
    size(z_range) ;
    r=polyphase(y(:,k),K2,L2,M2);
    z(:,z_range)=r(:,1+dN2:K2-dN2);
  end;
  z=z./max(max(abs(z))) ;
return ; 