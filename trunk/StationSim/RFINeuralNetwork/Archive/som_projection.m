function P = som_projection(D, P, algorithm, epochs)

%SOM_PROJECTION Computes the projection of map weight vectors.
%
% P = som_projection(D, P, algorithm, [epochs])
%
% ARGUMENTS ([]'s are optional) 
%
%  D          (matrix or data struct or map struct) original data set.
%              If D is an n-dim matrix, each D(i1, ... , in, :) is a 
%              data vector. In 2-dim case this means each row of D.
%  P          (scalar or matrix) output dimension, or the initial
%              projection matrix, which must have equal number of
%              vectors as the data set
%  algorithm  (string) the projection algorithm, either 'pca' or
%              'sammon' or 'cca', default is 'pca'
%  [epochs]   (scalar) training epochs (1 epoch = dlen training steps), 
%              dummy parameter for 'pca'
%
% RETURNS   
%
%  P            (matrix) projected coordinates of each data vector; 
%                barring output dimension, the size is same as that of D
%
% NOTE: the output dimension must be lower or equal to the data dimension
% NOTE: if projection type is 'pca', the epochs argument is obsolute
%
% EXAMPLES
%
% P = som_projection(sMap,3,'pca')
%  projects the codebook vectors of the map to 3-dimensions using the 
%  PCA. The projection can be visualized with som_showgrid like this: 
%  som_showgrid(sMap,P)
%
% P = som_projection(sMap,som_projection(sMap,3,'pca'),'sammon',20)
%  same as above, but now the PCA is used for initialization and 
%  the projection is further refined with Sammon's projection
%
% See also SOM_SAMMON, SOM_CCA, SOM_PCA, SOM_SHOWGRID, SOM_CREATE, 
% SOM_DATA_STRUCT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 191297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(3, 4, nargin));  % check no. of input arguments is correct

% check the type of the input argument(data matrix, data struct or map struct)

% data set
if isstruct(D)
  if isfield(D, 'data'), D = D.data;             % data struct
  elseif isfield(D, 'codebook'), D = D.codebook; % map struct
  else
    error('Invalid structure');
  end
end
orig_size = size(D); 
dim = orig_size(end);
noc = prod(orig_size)/dim;
if length(orig_size)>2, D = reshape(D,[noc dim]); end

% output dimension / initial projection matrix
if prod(size(P))==1, 
  odim = P; 
else 
  si = size(P);
  odim = si(end);
  if prod(si) ~= noc*odim, 
    error('Initial projection matrix size does not match data size');
  end
  if length(si)>2, P = reshape(P,[noc odim]); end
end
if odim > dim, error('Output dimension greater than data dimension'); end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

switch algorithm, 
case 'pca',    P = som_pca(D,odim);
case 'sammon', P = som_sammon(D,P,epochs,'steps');
case 'cca',    P = som_cca(D,P,epochs);
otherwise,     error(['Unknown projection algorithm: ' algorithm]);
end

out_size = orig_size;
out_size(end) = odim;
P = reshape(P, out_size);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%