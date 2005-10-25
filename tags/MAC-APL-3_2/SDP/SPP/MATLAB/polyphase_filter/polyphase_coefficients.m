function [p,h,p_test] = polyphase_coefficients(K,L,M,option,centre,h)
%
%  function [p,h] = polyphase_coefficients(K,L,M)
%
%   input:
%       K       - the number of channels
%       L       - filter length
%       M       - the downsampling rate (default M=K)
%       option  -   'coefficients'      returns the coefficients of the polyphase (default)
%                   'indices'           returns the indices into the prototype filter window
%       centre  - if centre='yes' the output is shifted to the centre (default centre='no')
%       h       - replaces the default fircls1 filter window with the one specified
%
%   output:
%       p   -   the coefficients of the polyphase in case option = 'coefficients'      
%               indices into the prototype filter window in case option = 'indices'
%       h   -   the prototype filter window
%
%   The filter coefficients can be applied directly to the data, however it is not necessary
%   to compute the filterbank coefficients p, as the following example will show. The filter
%   coefficients of the polyphase structure can be optained in two ways (e.g. take K=16,L=8,M=16):
%   
%       [p,h] = polyphase_coefficents(K,L,M) ; 
%       [i,h] = polyphase_coefficient(K,L,M,'indices') ; 
%       h(i) == p
%
%   The use of indices computation allows you to easily replace the prototype filter window.
%   The current implementation uses firls, in the following example we consider fir1:
%
%       [i,h] = polyphase_coefficient(K,L,M,'indices') ; 
%       alpha = 1 ; 
%       h=fir1(K-1,L/K * alpha) ;
%       h = resample(h, K * L, K);
%       h = h / sqrt(h * h'); % nomalisation
%       h = h / max(h);
%       p=h(i) ;     
%
%   The same result can be achieved by directly providing a prototype filter window, e.g.
%
%       hfir=fir1(K-1,L/K * alpha) ;
%       p=polyphase_coefficients(16,12,16,'coefficients','no',hfir) ;
%       [i,h]=polyphase_coefficients(16,12,16,'coefficients','no',hfir) ;
%
%   Note that the returned filter windows is resampled so h != hfir in the example above,
%   and hfir(i) is NOT a valid indexing of hfir, but h(i)==p. 
%
%  See also: reordering, partition, prefilter, polyphase, firls
%
% (C) 2002 Astron, by M.van Veelen, Version 1.2

p=zeros(K,L) ;

if nargin < 3 ; M=K ; end ;
if nargin < 4 ; option = 'coefficients' ; end ;
if nargin < 5 ; centre = 'no' ; end ;

if nargout > 2
    p_test=zeros(K,L) ;
end ;

% Quantisation settings (if any)
h_nq = 0 ;  % Number of quantisation level for the prototype filter windows h

% Compute the prototype filter window
if nargin < 6
    alpha =1 ; % alpha = M/(K); 
    h = fircls1(K-1, L / K * alpha, 0.01, 0.001);
end;

   h = resample(h, length(h) * L, K) ; % version 1.0
   % h = resample(h, length(h) * floor(L * K/M), K) ; % version 1.1
   
   if strcmp(centre, 'yes') ;
       h=exp(2*sqrt(-1)*pi*L/2*(1:1:length(h))/length(h)).*(K * L * h);
   else
       % tryout for the first phase
       h=exp(-2*sqrt(-1)*pi*L/2*(1:1:length(h))/length(h)).*(K * L * h);
   end;
   
   h = h / sqrt(h * h'); % nomalisation
   h = h / max(h);

if h_nq > 0
    % Quantization and normalization of the filter in order to avoid a bias
    h = double(uencode(h, quant_h, 1, 'signed'));
    h = h / max(h);                                 
end;


% p=partition(K*L,K)';  % version 1.0
% p=partition(K*floor(L*K/M),K)' ; % version 1.1
p=partition(K*L,K,K-M)';  % version 1.2

if ~strcmp(option, 'indices')    
    p = h(p) ;
end

if nargout > 2
    for n = 1 : K
        for i = 1 : L
            p_test(n, i) = h((i - 1) * K + n);
        end ;
    end
end

%fh = fopen('c:\temp\h.dat', 'w+');
%fwrite(fh, SubFilter, 'double');
%fclose(fh);



    
