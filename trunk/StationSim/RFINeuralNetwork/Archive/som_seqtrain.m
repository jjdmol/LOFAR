function [M, qe] = som_seqtrain(M, lattice, shape, neigh, D, train_len, rad, alpha, mask, tracking)

%SOM_SEQTRAIN Use sequential algorithm to train the self-organizing map.
%
% [M, qe] = som_seqtrain(M, lattice, shape, neigh, D, ...
%                        train_len, rad, alpha, [mask], [tracking])
% 
% ARGUMENTS ([]'s are optional)
% 
%  M           (matrix) map unit weight vectors, size n1 x ... x nk x dim
%  lattice     (string) map lattice, 'rect' or 'hexa'
%  shape       (string) map shape, 'rect' or 'cyl' or 'toroid'
%  neigh       (string) neighborhood function, 'bubble' or 'gaussian' 
%               or 'ep'
%  D           (matrix) data matrix, size dlen x dim
%  train_len   (scalar) training length
%  rad         (scalar or vector) initial learning radius (rad_ini) or
%               initial and final learning radiuses ([rad_ini rad_fin]) 
%               or neighborhood radius width for each step (vector of
%               length train_len)
%  alpha       (scalar or vector) the initial learning rate (alpha_ini) 
%               or the learning rate for each step, size train_len x 1
%  [mask]      (vector) component masks, size dim x 1 
%               default value a vector of ones
%  [tracking]  (scalar) level of monitoring: 
%               0 - estimate time (the default)
%               1 - track time and quantization error
%               2 - plot quantization error
%               3 - plot quantization error and two first components 
% RETURNS
%
%  M           (matrix) batch-trained map codebook matrix, 
%               size n1 x ... x nk x dim
%  qe          (scalar) average quantization error
%
% EXAMPLES
%
%  M = som_seqtrain(M, 'hexa', 'rect', 'bubble', D, 1000, 5, 0.3);
%  M = som_seqtrain(M, 'rect', 'rect', 'gaussian', D, 2000, 3, 0.03);
%   below, default mask is used, tracking type=3 and 
%   neighborhood radius goes from 5 to 1.5
%  M = som_seqtrain(M, 'hexa', 'rect', 'ep', ...
%                   D, 1000, [5 1.5], 0.3, [], 3);
%   below, 4-dimensional data used and in the BMU search
%   the third component is ignored
%  M = som_seqtrain(M, 'rect', 'rect', 'ep', ...
%                   D, 1000, [5 1.5], 0.3, [1 1 0 1], 3);
%   below, rads and alphas are vectors of length 1000 giving 
%   the neighborhood radius and learning coefficient for 
%   each step separately
%  M = som_seqtrain(M, 'rect', 'rect', 'ep', D, 1000, rads, alphas);
%
% See also  SOM_BATCHTRAIN, SOM_TRAINOPS, SOM_TRAIN.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 220997
 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments

error(nargchk(8, 10, nargin));  % check the number of input arguments

% dimensions

msize = size(M);
map_vector_dim = msize(length(msize));
[dlen dim] = size(D);
if dim ~= map_vector_dim, 
  fprintf(2, 'Map vector dimension:  %d\nData vector dimension: %d\n', map_vector_dim, dim);
  error('Data set dimension not equal to map dimension.');
end

% neighborhood function

switch neigh
  case 'bubble',   neighf = 0;
  case 'gaussian', neighf = 1;
  case 'cutgauss', neighf = 2;
  case 'ep',       neighf = 3;
  otherwise, 
    error ('Unknown neighborhood function.'); 
end

% learning rate

if any(isnan(alpha)) | isempty(alpha),
  error('NaN/empty vector is unsuitable for learning rate.');
elseif length(alpha) > 1 & length(alpha) ~= train_len,
  error('The length of learning rate vector (alpha) should be either 1 or train_len.');
end

if length(alpha) == 1 & train_len > 0, 
  % use linear training coefficient function
  alpha = fliplr(1:train_len)/train_len * alpha;
  % another option would be inverse function: 
  % this is implemented in function som_train
end

% neighborhood radius

if any(isnan(rad)) | isempty(rad),
  error('NaN/empty vector is unsuitable for neighborhood radius.');
elseif length(rad) == 1, 
  rad = [rad 1]; 
elseif length(rad) > 2 & length(rad) ~= train_len,
  error('The length of neighborhood radius vector (rad) should be either 1, 2 or train_len.');
end

if length(rad) == 2 & train_len > 1, 
  rad = rad(2) + fliplr((0:(train_len-1))/(train_len-1)) * (rad(1)-rad(2));
end

rad(find(rad==0)) = sqrt(realmin); % zero neighborhood radius causes 
                                   % div-by-zero error

% weights

if nargin < 9 | isempty(mask) | any(isnan(mask)), mask = ones(dim,1); end

% tracking

if nargin < 10 | isempty(tracking) | isnan(tracking), tracking = 1; end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization

% remove empty vectors from the data
empty_vecs = (sum(isnan(D')) == dim);
D = D(find(~empty_vecs),:);
[dlen dummy] = size(D);

% random order of samples
rand('state',sum(100*clock));
sample_inds = ceil(dlen*rand(train_len,1));
% or if you want the samples in order, uncomment the below
%tmp = repmat(1:dlen,[1 ceil(train_len/dlen)]);
%sample_inds = tmp(1:train_len);

% the map
msize = msize(1:(length(msize)-1));   % msize = [n1, ...,  nk]
mdim = length(msize);                 % grid dimension
munits = prod(msize);                 % number of map units
M = reshape(M,munits,dim);  % change the shape of the matrix
if any(any(isnan(M))), 
  error ('Map weight matrix must not have missing components.');
end

% distance between map units in the grid space (_not_ in the vector space)
Unitdist = som_unit_distances(msize, lattice, shape); 
% since in the case of gaussian and ep neighborhood functions, the 
% equations utilize squares of the unit distances and in bubble case
% it doesn't matter which is used, the unitdistances and neighborhood
% radiuses are squared
Unitdist = Unitdist.^2;
rad = rad.^2;

% tracking
if train_len > 0, % estimate the training time 
  start = clock;
  % perform the heaviest operation once                  
  x = D(1,:); known = ~isnan(x); Dx = M(:,known) - x(ones(munits,1),known);   
  [qerr bmu] = min(mask(known)'*(Dx'.^2)); 
  % multiply by training length times correction term
  fprintf(1,'Estimated time: %5.3f seconds ', ceil(etime(clock,start)*train_len*6));
end
qe = 0;
if tracking >  0, % initialize tracking
  track_step = min(100,dlen);
  track_table = zeros(track_step,1);
  qe = zeros(floor(train_len/track_step)+1,1);  
  start = clock;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

for t = 1:train_len, 

  % find BMU
  x = D(sample_inds(t),:);                     % sample vector
  known = ~isnan(x);                           % its known components
  Dx = M(:,known) - x(ones(munits,1),known);   % difference between vector and all map units
  [qerr bmu] = min(mask(known)'*(Dx'.^2)); % minimum distance(^2) and the BMU

  % tracking
  if tracking > 0, 
    i = rem(t,track_step); 
    if i>0, track_table(i) = sqrt(qerr);
    else 
      n = ceil(t/track_step); 
      track_table(track_step) = sqrt(qerr);
      qe(n) = mean(track_table);
      trackplot(M,D,tracking,start,n,qe);
    end
  end
  
  % neighborhood function 
  % notice that the elements Unitdist and rad have been squared!
  switch neighf, 
    case 0, % bubble 
      h = (Unitdist(bmu,:) <= rad(t));
    case 1, % gaussian
      h = exp( - (Unitdist(bmu,:))/(2*rad(t))); 
    case 2, % cut gaussian
      h = exp( - (Unitdist(bmu,:))/(2*rad(t))) .* (Unitdist(bmu,:) <= rad(t));
    case 3, % epanechnikov
      h = (1 - Unitdist(bmu,:)/rad(t)) .* (Unitdist(bmu,:) <= rad(t));
  end  

  % update
  M(:,known) = M(:,known) - alpha(t)*diag(h)*Dx;

end; % for t = 1:train_len

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Build / clean up the return arguments

% final quantization error
Wmap = (mask*ones(1,dlen)) .* (~isnan(D))'; % mask NaN's out
D(find(isnan(D))) = 0;                      % set NaN's to zeros 
Dist = (M.^2)*Wmap + ones(munits,1)*mask'*(D'.^2) - 2*M*diag(mask)*D';
qe(length(qe)) = mean(sqrt(min(Dist)));

% tracking
if tracking > 0, trackplot(M,D,tracking,start,length(qe),qe); end
if tracking | train_len, fprintf(1,'\n'); end

M = reshape(M, [msize dim]);
qe = qe(length(qe));

return;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% subfunctions

function [] = trackplot(M,D,tracking,start,n,qe)

  l = length(qe);
  elap_t = etime(clock,start); tot_t = elap_t*l/n;
  fprintf(1,'\rTraining: %3.0f/ %3.0f s [qerror: %f]\n',elap_t,tot_t,qe(n))  
  switch tracking
    case {0, 1}, %nil
    case 2,       
      plot(1:n,qe(1:n),(n+1):l,qe((n+1):l))
      title('Quantization errors for each min(dlen,100) samples')    
      drawnow
    otherwise,
      subplot(2,1,1), plot(1:n,qe(1:n),(n+1):l,qe((n+1):l))
      title('Quantization error for each min(dlen,100) samples');
      subplot(2,1,2), plot(M(:,1),M(:,2),'ro',D(:,1),D(:,2),'b+'); 
      title('First two components of map units (o) and data vectors (+)');
      drawnow
  end
  
% end of trackplot

