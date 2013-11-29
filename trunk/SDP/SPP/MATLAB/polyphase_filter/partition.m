function [indices] = partition(n,m,l,option)
%
%   function [indices] = partition(n,m,l,option)
%
%   input
%       n       -   length of the signal
%       m       -   length of a block
%       l       -   length of overlapping section   (default=0)
%       option  -   'set'   [default]   returns the entire set of indices
%                   'range'             returns a list of ranges in pairs of [first,last]
%   output:
%
%       In case option='set' the returned value indices is a floor( (n-m) /(m-l))+1 by m array
%       each row contains the indices for one block of data, the rows are consequetive, possibly
%`      overlapping windows, e.g. partition(8,4,2) returns:
%           
%                1     2     3     4
%                3     4     5     6
%                5     6     7     8  
%
%       In cse option='range' the returned value indices is a  floor( (n-m) /(m-l))+1 by 2 matrix
%       with indices(:,1) are the first indices of the ranges and indices(:,2) are the last indices 
%       of the ranges, e.g. partition(8,4,2,'range') returns:
%   
%                    1     4
%                    3     6
%                    5     8
%
%       See also: reordering, 
%
% (C) 2002 Astron, by M.van Veelen

if nargin < 4
    option='set';
end ;

if nargin<3
    l=0;
end;


n_indices = floor( (n-m) /(m-l))+1;

if strcmp(option,'range')
    indices=zeros(n_indices,2) ;
    indices(:,1) = [1:(m-l):n-m+1   ]';
    indices(:,2) = indices(:,1)+m-1;
else
    indices=zeros(n_indices, floor(m) ) ;
    for i=0:n_indices-1
        indices(i+1,:)=i*(m-l)+[1:m] ;
    end;
end;

