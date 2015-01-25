function [y] = reordering( x,M,L,option ) 
%
%  function [Y] = reordering( X,M,L,OPTION ) 
%
%  input:
%   X       -   input vector
%   M       -   number of channels and downsample rate
%   L       -   upsample rate [default 1]
%   OPTION  -   'block'
%               'streaming'
%               'demo'
%               'test'
%
%  output:
%   Y    - reordered data matrix in case X is a vector:
%
%       Y   has M rows and L*length(x)/M columns 
%
%   The input signal is decimated by a factor M, thus obtaining M different channels
%   with a sampling frequency of fs/M when fs is the sampling frequency of the input.
%   In the default case L=1 the samples are distributed uniformly, for higher values
%   of L each channel is upsampled by L, thus within each channel the samples are 
%   M/L apart, e.g. with reordering([1:30],8,1.5) the returned matrix will be:
%
%            1     6    11    16    21 
%            2     7    12    17    22 
%            3     8    13    18    23 
%            4     9    14    19    24 
%            5    10    15    20    25 
%            6    11    16    21    26 
%            7    12    17    22    27 
%            8    13    18    23    28  
%
%   This funtion is implemented using partition. The indices of the input matrix at the 
%   location of the output matrix of the reordering function can be obtained with:
%   partition(length(x),M,floor(M*(L-1))) , this indices can also be used to obtain Y from X:
%
%       reordering(x,M,L)   ==   x(partition(length(x),M,floor(M*(L-1))))'
%
%   See also: partition
%
% (C) 2002 Astron, by M.van Veelen

if nargin < 4 ; 
    option = 'block' ;
end ;

if nargin < 3
    L=1;
end;

if strcmp(option,'streaming')
    error('option streaming is not implemented yet');
end ;
    
if strcmp(option,'demo')
    display( 'x=1:100;y=reordering(x,1.3,1);mesh(y)' ) ;
    x=1:100;y=reordering(x,16,1.25);mesh(y);
    return ;
end ;

if strcmp(option,'test')
    if reordering(x,M,L)   ==   x(partition(length(x),M,floor(M*(L-1))))'
        display('TEST SUCCESSFUL : ASSERT{reordering(x,M,L)== x(partition(length(x),M,floor(M*(L-1))))''}') ;
    end ;
    return ;
end ;

n_x = length(x) ;
indices=partition( n_x, M, floor(M*(L-1)),'range' );
n_y=length(indices');
y=zeros(M,n_y) ;

for i=1:n_y
  y(1:M,i) = x(indices(i,1):indices(i,2))';
% DEBUG   t=x(indices(i,1):indices(i,2))
% DEBUG   y(1:M,i) = t';
end ;

