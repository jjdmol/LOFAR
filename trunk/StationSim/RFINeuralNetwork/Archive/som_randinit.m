function M = som_randinit(D, msize)

%SOM_RANDINIT Initialize self-organizing map (randomly).
%
% M = som_randinit(data, msize)
% 
% ARGUMENTS 
% 
%  data   (matrix) Q x dim data matrix
%  msize  (vector) self-organizing map grid size, k x 1 vector
%          NOTE: the matrix notation of indexes is used
% 
% RETURNS   
%
%  M      (matrix) randomly initialized self-organizing map 
%          codebook matrix, size n1 x ... x nk x dim, where
%          [n1 ... nk] = msize and dim is the dimension of the data 
%          vectors
%
% See also  SOM_LININIT, SOM_INIT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta kimmo & ecco 150797, ecco 100997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 2, nargin));  % check no. of input args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initalize variables

dim    = size(D, 2);           
munits = prod(msize);
M      = rand([msize dim]); 
M      = reshape(M, [munits dim]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize codebook vectors

% set interval of each component to correct value

for i = 1:dim,
  M(:,i) = (max(D(:,i)) - min(D(:,i))) * M(:,i) + min(D(:,i)); 
end

% reshape the codebook matrix

M = reshape(M, [msize dim]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
