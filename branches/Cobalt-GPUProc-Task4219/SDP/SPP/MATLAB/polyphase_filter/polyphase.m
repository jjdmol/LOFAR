function [Yall,Yall_test] = polyphase(x,K,L,M,option)
%
%  function [Y] = polyphase(x,K,L,M,option)
%
%       K       - the number of channels
%       L       - filter length
%       M       - the downsampling rate
%       option  -   'normal'    [default]
%                   'shift'     applies fftshift on the output of the filterbank
%                   'centre'    centers the signals and recombines the output accordingly
%
%  Compute the polyphase filter coefficients p and applies them the the data x, i.e. the 
%  data is decimated by a factor M and divided into K channels. The prototype filter can
%  be supplied through h, by default fircls1 is used to derive the prototype filter window.
%  The coefficients care derived from this prototype windows and convolved with x, hence
%  a number of output vectors are computed which can be transformed into the real filter
%  outputs using an DFT. The polyphase filter structure comprises of two stages:
%
%       STAGE 1: Convolution
%       STAGE 2: Fourier Transform
%   
%  The both stages are performed. The output is an N X K matrix with a %  channel in each columm, 
%  providing K columns; the length N, is floor((length(x))/K)-L+1, in case the data is critically 
%  sampled (K=M). Theoretical background can be found in
%
%       [1] Multirate Digital Signal Processing. Ronald. E. Crochiere & Lawrence R.Rabiner,1983.
%           ISBM 0-13-605162-6, Prentice Hall Inc., Englewood Cliffs, New Jersey 07632.
%
%  See also: reordering, partition, prefilter, polyphase_coefficients
%
% (C) 2002 Astron, by M.van Veelen
%

if nargin < 5 ; option='normal' ; end ;

% Settings for finite word-length coding

% INITIALIZE: compute the filter prototype coefficients of the 
centre='no' ; if strcmp(option,'centre') ; centre='yes' ; end ;
   
% STAGE 1: Pre-filter by convolution with polyphase filter coefficients
ym=prefilter(x,K,L,M,centre); 

% STAGE 2: DFT the output of the pre-filter to obtain the channels
Yall=1/K * fft(ym)' ;

if strcmp(option,'shift') | strcmp(option,'centre') ;
    Yall= [ Yall(:,floor(K/2):-1:1) , Yall(:,K:-1:floor(K/2)+1)  ];
end ;
                                               
% Normalization of the input signals in order to replace the signal in the range [-1,+1]                                                   
% x = x / max(x);                                                  
% x = double(uencode(x, quant_signal, 1, 'signed'));
% x = x / max(x);




 
