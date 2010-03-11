function [ym] = prefilter(x,K,L,M,centre,h)
%
%  function [Y] = prefilter(x,K,L,M,centre,h)
%
%       K       - the number of channels
%       L       - filter length
%       M       - the downsampling rate
%       centre  - if centre='yes' the output is shifted to the centre (default centre='no')
%       h       - replaces the default fircls1 filter window with the one specified
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
%  The first stage is implemented in the function. The output is an N X K matrix with a
%  channel in each columm, providing K columns; the length N, is floor((length(x))/K)-L+1,
%  in case no case the data is critically sampled (K=M).
%
%  See also: reordering, partition, prefilter, polyphase, polyphase_coefficients
%
% (C) 2002 Astron, by M.van Veelen
   
% Settings for finite word-length coding

% INITIALIZE: compute the filter prototype coefficients of the 

if nargin < 5 ; centre='no' ; end ;

if nargin == 6
    [i,h]=polyphase_coefficients(K,L,M,'indices',centre,h); 
else
    [i,h]=polyphase_coefficients(K,L,M,'indices',centre); 
end;

n_y = floor((length(x)) / K )-L+1;
parts=partition(length(x),K,0)';
xd=x(parts) ;
y = zeros(1, K);
ym= zeros(K, n_y);

% STAGE 1: apply the pre-filter
for m = 1 : n_y
     y=1/L * sum(( xd(i+(m-1)*K)  .* h(i)  )' );  
        % When p=h(i) this is equivalent to y=1/L * sum(( xd(:, m : m + L - 1)  .* p  )' ); 
        % y = double(uencode(y, quant_inputfft, 1, 'signed')) / 2^(quant_inputfft - 1);   % Quantise the input of the FFT
     ym(:,m)=y';
end ;