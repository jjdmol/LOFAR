function [P] = som_pca(D,odim)

%SOM_PCA Projects data vectors using Principal Component Analysis.
%
% P = som_pca(D, odim)
%
% ARGUMENTS ([]'s are optional)
%
%  D      (matrix) the data matrix, size dlen x dim
%  odim   (scalar) how many principal vectors are used
%
% RETURNS
%  
%  P      (matrix) the projections, size dlen x odim
%
% See also SOM_PROJECTION, SOM_SAMMON, SOM_CCA, SOM_SHOWGRID.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 191297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments 

error(nargchk(2, 2, nargin)); % check the number of input arguments

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

% the data
[dlen dim] = size(D);

% full samples (no missing components)
ind = find(~isnan(sum(D'))); % indices of samples not having NaNs 
n = length(ind);
if n<odim, 
  error('Do not have enough full vectors for PCA');
end
if odim > dim, 
  error('Output dimension greater than data dimension')
end

% compute principal components using singular value decomposition
[U, S, V] = svds(D(ind,:), odim);

% normalize eigenvectors to unit length
for i = 1:odim, Eigvec(:,i) = V(:,i) / norm(V(:,i)); end

% project the data
P = zeros(dlen,odim);
for i=1:odim, P(:,i) = D*Eigvec(:,i); end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
