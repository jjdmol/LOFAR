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
% (C) 2002 Astron, by M.van Veelen. Version 1.2
   
% Settings for finite word-length coding (TODO)
%

% INITIALIZE: compute the filter prototype coefficients of the 
%
%   $Log$
%
%   $Id$
%
%   h(i))'  changed to h(i)).'
%
%   NOTE: M' =DEF= complex conjugate ; M.' =DEF= transpose

if nargin < 4 ; M=K ; end ;
if nargin < 5 ; centre='none' ; end ;


% if (K>M) ;  floor(L*K/M) , end ; % faking it!!!!

if nargin == 6
    [i,h]=polyphase_coefficients(K,L,M,'indices',centre,h); 
else
    [i,h]=polyphase_coefficients(K,L,M,'indices',centre); 
end;

%size(i)
% if (K>M) ; i=[i , ones(K,1)]; L=floor(L*K/M) ; end ; % faking it!!!!

parts=partition(length(x),K,K-M)';
% n_y = floor((length(x)) / M )-L+1; % This should work ... but it doesn't for K>M

% L=floor(L*K/M);

n_y=size(parts,2)-L+1 ;

if n_y ~= size(parts,2)-L+1 ; display('n_y ~= length(parts)-L+1') ; end;

% xd=x(parts) ; % obsolete since version 1.1
% y = zeros(1, K); % obsolete since version 1.4
ym= zeros(K, n_y);

% display( ['debug: length(x)==',num2str(length(x)), ' length(parts)-L+1==', num2str(length(parts')-L+1),' K==',num2str(K),' size(i)==[' , num2str(size(i)) , '] ' ] ) ;
% display( ['size(i)==[',num2str(size(i)),'] size(parts(:,(1:L)))==[', num2str(size(parts(:,(1:L)))), '] n_y==', num2str(floor(length(x) ./ M))] ) ; 

% STAGE 1: apply the pre-filter
for m = 1 : n_y
     % y=1/L * sum(( xd(i+(m-1)*K)  .* h(i)  )' );              % version 1.0.0
     % y=1/L * sum(( x( parts(i+(m-1)*K) )  .* h(i)  )' );      % version 1.1.1
     % ASSERTION parts(:,(1:L)+(m-1))   == i + (m-1)*M  WARNING : only holds for M=K        
     % y=1/L * sum(( x( parts(:,(1:L)+(m-1)) ) .* h(i)  )' );   % version 1.1.2
     % y=1/L * sum(( x( i+(m-1)*M ) .* h(i)  )' );              % version 1.1.3, line obsolete since version 1.1.4
        % When p=h(i) this is equivalent to y=1/L * sum(( xd(:, m : m + L - 1)  .* p  )' ); 
        % y = double(uencode(y, quant_inputfft, 1, 'signed')) / 2^(quant_inputfft - 1);   % Quantise the input of the FFT
     % ym(:,m)=y'; % obsolete since version 1.1.4
     % ym(:,m)=(1/L * sum(( x( i+(m-1)*K ) .* h(i))' ))';       % version 1.1.4
     % ym(:,m)=(1/L * sum(( x( parts(:,(1:L)+(m-1))  ) .* h(i)).' )).';       % version 1.1.5
     ym(:,m)=(sum(( x( parts(:,(1:L)+(m-1))  ) .* h(i)).' )).';       % version 1.2

end ;