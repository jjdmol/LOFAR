function [M, qe] = som_batchtrain(M, lattice, shape, neigh, D, train_len, rad, mask, tracking)

%SOM_BATCHTRAIN  Use batch algorithm to train the self-organizing map.
%
% [M qe] = som_batchtrain(M, lattice, shape, neigh, D, train_len, rad, ...
%                         [mask], [tracking])
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
%  [mask]      (vector) component weights learning, size dim x 1, 
%               default value is a vector of ones
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
% NOTE: batchtrain does not work correctly for a map with a single unit. 
% This is because of the way 'min'-function works. If such a map is 
% given, the function returns the average of the data set.
%
%  M = som_batchtrain(M, 'hexa', 'rect', 'bubble', D, 10, 5);
%   below, mask is default, tracking type=3 and neighborhood 
%   radius goes from 5 to 1.5
%  M = som_batchtrain(M, 'hexa', 'rect', 'ep', D, 10, [5 1.5], [], 3);
%   below, five training steps are used and the neighborhood 
%   radius specified for each step separately
%  M = som_batchtrain(M, 'hexa', 'rect', 'ep', D, 5, [5 4 3 2 1]);
%   below, data is 4-dimensional data, and in BMU search 
%   the third component is ignored
%  M = som_batchtrain(M, 'rect', 'rect', 'ep', ...
%                     D, 10, [5 1.5], [1 1 0 1], 3);
%
% See also  SOM_SEQTRAIN, SOM_TRAINOPS, SOM_TRAIN.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 071197 041297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments 

error(nargchk(7, 9, nargin));  % check the number of input arguments

% dimensions

msize = size(M);
map_vector_dim = msize(length(msize));
[dlen dim] = size(D);
if dim ~= map_vector_dim, 
  fprintf(2, 'Map vector dimension:  %d\nData vector dimension: %d\n', map_vector_dim, dim);
  error('Data set dimension not equal to map dimension.');
end
if prod(msize)/map_vector_dim == 1,
  fprintf(2, 'Map contains only one unit. Setting unit location to mean value of data.\n');
  M = som_nanops(D);
  M = reshape(M,msize);
  return;
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

% neighborhood radius 

if any(isnan(rad)) | isempty(rad),
  error('NaN/empty vector is unsuitable for neighborhood radius.');
elseif length(rad) == 1, 
  rad = [rad 1]; 
elseif length(rad) > 2 & length(rad) ~= train_len,
  error('The length of neighborhood radius vector (rad) should be either 1, 2 or train_len.');
end

if length(rad) == 2 & train_len > 1, 
  rad = rad(2) + fliplr(0:(train_len-1))/(train_len-1) * (rad(1)-rad(2));
end

rad(find(rad==0)) = sqrt(realmin); % zero neighborhood radius causes 
                                   % div-by-zero error

% weights

if nargin < 8 | isempty(mask) | any(isnan(mask)), mask = ones(dim,1); end

% tracking

if nargin < 9 | isempty(tracking) | isnan(tracking), tracking = 1; end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization

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
% it doesn't matter which is used:
Unitdist = Unitdist.^2;
rad = rad.^2;

% reserve space for the distance matrix
Dist = zeros(munits, dlen);

% The training algorithm involves calculating weighted Euclidian distances 
% to all map units for each data vector. Basically this is done as
%   for i=1:dlen, 
%     for j=1:munits, 
%       for k=1:dim
%         Dist(j,i) = Dist(j,i) + mask(k) * (D(i,k) - M(j,k))^2;
%       end
%     end
%   end
% where mask is the weighting vector for distance calculation. However, taking 
% into account that distance between vectors m and v can be expressed as
%   |m - v|^2 = sum_i ((m_i - v_i)^2) = sum_i (m_i^2 + v_i^2 - 2*m_i*v_i)
% this can be made much faster by transforming it to a matrix operation:
%   Dist = (M.^2)*mask*ones(1,d) + ones(m,1)*mask'*(D'.^2) - 2*M*diag(mask)*D'
% Of the involved matrices, several are constant, as the mask and data do 
% not change during training. Therefore they are calculated beforehand.
%
% In the case where there are unknown components in the data, each data
% vector will have an individual mask vector so that for that unit, the 
% unknown components are not taken into account in distance calculation.
% In addition all NaN's are changed to zeros so that they don't screw up 
% the matrix multiplications and behave correctly in updating step.

% handle unknown components
Known = ~isnan(D);
Wmap = (mask*ones(1,dlen)) .* Known'; 
D(find(~Known)) = 0;  

% calculate the constant matrices
D2 = ones(munits,1)*mask'*(D'.^2);
WD = 2*diag(mask)*D';
  
% tracking
if train_len > 0, % estimate the training time
  % perform the heaviest operation once
  start = clock;
  Dist = (M.^2)*Wmap + D2 - M*WD; 
  % multiply by training length times correction term
  fprintf(1,'Estimated time: %5.3f seconds ', ceil(etime(clock,start)*train_len*3.5));
end
qe = 0;
if tracking >  0, % initialize tracking
  qe = zeros(train_len+1,1);  
  start = clock;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

for t = 1:train_len,

  % find winners
  Dist = (M.^2)*Wmap + D2 - M*WD;
  [ddists, winners] = min(Dist);
  
  % tracking
  if tracking > 0,
    qe(t) = mean(sqrt(ddists)); 
    trackplot(M,D,tracking,start,t,qe);
  end

  % neighborhood function 
  % notice that the elements Unitdist and rad have been squared!
  switch neighf, 
    case 0, % bubble - this matches the original "Batch Map" algorithm
      H = (Unitdist(winners,:) <= rad(t));
    case 1, % gaussian
      H = exp( - (Unitdist(winners,:))/(2*rad(t)));
    case 2, % cut gaussian
      H = exp( - (Unitdist(winners,:))/(2*rad(t))) .* (Unitdist(winners,:) <= rad(t));
    case 3, % epanechnikov
      H = (1 - Unitdist(winners,:)/rad(t)) .* (Unitdist(winners,:) <= rad(t));
  end

  % update

  % In principle the updating step goes like this: replace each map unit 
  % by the average of the data vectors that were in its neighborhood.
  % The contribution, or activation, of data vectors in the mean can 
  % be varied with the neighborhood function. This activation is given by matrix H.
  % So, for each map unit 
  %   m = sum_i (h_i * d_i) / sum_i (h_i), where i denotes the index of data vector
  % This is basically what the line "M = 1./A(...) .* H' * D" is about.

  % The fact that there might be unknown components in the data 
  % complicates matters a bit since we don't want the map to be updated by them.
  % Therefore we want to set activation for these data components to zero
  % i.e. for the component j of a map unit the above equation can be rewritten as
  %   m_j = sum_i (h_ij * d_ij) / sum_i (h_ij), 
  % and we want to set h_ij=0 if d_ij is unknown. However, in fact the matrix H
  % does not have an index j at all since it is not component-specific, so we can't
  % handle the matter by just modifying the H matrix. The way this is handled
  % is by setting the unknown components themselves to zero (done in the 
  % initialization phase) and removing their contribution to the total activation
  % (done on the line "A = H' * Known"). 

  A = H' * Known;        % a(ij) is total activation of component j for map unit i over
                         % all neighborhood functions, or the same as "sum_i (h_ij)"
                         % above with unknown components taken into account
  nonzero = find(A > 0); % indexes of non-zero activations: only these are updated 
  A(find(A==0)) = 1;     % since the inverse of A is used, zeros must be taken care of
  S = H' * D;            % the "sum_i (h_i * d_i)" above
  M(nonzero) = S(nonzero) ./ A(nonzero); 
    
end; % for t = 1:train_len

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Build / clean up the return arguments

% final quantization error
Dist = (M.^2)*Wmap + D2 - M*WD;
qe(length(qe)) = mean(sqrt(min(Dist)));

% tracking
if tracking > 0, trackplot(M,D,tracking,start,length(qe),qe); end
if tracking | train_len, fprintf(1,'\n'); end

M = reshape(M,[msize dim]);
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
      title('Quantization error after each epoch');
      drawnow
    otherwise,
      subplot(2,1,1), plot(1:n,qe(1:n),(n+1):l,qe((n+1):l))
      title('Quantization error after each epoch');
      subplot(2,1,2), plot(M(:,1),M(:,2),'ro',D(:,1),D(:,2),'b+'); 
      title('First two components of map units (o) and data vectors (+)');
      drawnow
  end

% end of trackplot
