function [sMap] = som_train(sMap, sData, epochs, radius, alpha, alpha_type, tracking)

%SOM_TRAIN Train a self-organizing map.
%
% sMap = som_train(sMap, sData, [epochs], [radius], ...
%                  [alpha_ini], [alpha_type], [tracking])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap          (struct) self-orgainzing map structure
%  sData         (struct or matrix) data structure or data 
%                 matrix used in the training
%  [epochs]      (scalar) length of the training in epochs (one epoch 
%                 equals the length of the training data)
%  [radius]      (scalar or vector) initial learning radius (radius_ini) 
%                 or initial and final learning radiuses 
%                 ([radius_ini radius_fin]) 
%  [alpha_ini]   (scalar) the initial learning rate (alpha_ini) 
%  [alpha_type]  (string) learning rate function, 'linear' or 'inv', 
%                 see below, 'linear' by default
%  [tracking]    (scalar) level of monitoring: 
%                 0 - estimate time 
%                 1 - track time and quantization error (the default)
%                 2 - plot quantization error
%                 3 - plot quantization error and two first components 
% RETURNS
%
%  sMap          (struct) trained self-organized map. In the struct, 
%                 a train sequence struct has been added to the 
%                 .train_seq field. The training sequence struct
%                 contains the following fields: 
%   .train_seq{i}
%    .algorithm  (string) the training algorithm, 'seq' or 'batch'
%    .radius_ini (scalar) the initial neighborhood radius
%    .radius_fin (scalar) the final neighborhood radius
%    .alpha_ini  (scalar) the initial learning coefficient
%    .alpha_type (string) alpha type, 'linear' or 'inv'
%    .trainlen   (scalar) the number of training steps
%    .time       (string) when the training was done
%
% This function is offered as a convenient interface for the actual 
% training functions som_seqtrain and som_batchtrain. It requires the
% very minimum number of parameters (the map and the data) and uses 
% *very* heuristical default values for the unspecified training 
% parameters. Training is performed in two phases (first rough
% ordering and then finetuning), if
%  1. the map has not been trained before
%  2. random algorithm has been used in initialization
%  3. epochs, radius and alpha_ini are left to their default values
%
% There are two alpha function types: 'linear' and 'inv' or inverse of time. 
% In the former the learning rate decreases linearly from the given 
% initial value to 0. In the latter the learning rate decreases
% in proportion to inverse of time from given initial value to 
% initial value / 100. 
%
% Information of the training is saved in the train_sequence 
% field of the trained map.
% NOTE: som_train is the only function that updates the train_sequence
% field of the map struct. If you use the lower level functions 
% to train the map, information of those trainings are not added
% to the train_sequence structure.
%
%  sMap = som_train(sMap,sData);                % default parameters
%  sMap = som_train(sMap,sData,[],[],[],'',[]); % likewise
%  sMap = som_train(sMap,sData,4); 
%  sMap = som_train(sMap,sData,4,[5 1],0.03,'inv'); 
%  sMap = som_train(sMap,sData,[],[],[],'',3);  % tracking=3
%
% See also SOM_SEQTRAIN, SOM_BATCHTRAIN, SOM_TRAINOPS.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 220997, 071197

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments and initialize

error(nargchk(2, 7, nargin));  % check the number of input arguments

% data struct or matrix
if isstruct(sData), D = sData.data; else D = sData; end
[dlen dim] = size(D);

% Typically the map is trained in two phases: rough ordering and fine-tuning
% However, if the map has already been trained, or the linear initialization 
% algorithm has been used there is no need for the first phase. The training
% will also be made in two phases only if the training parameters are left 
% to default values.

do_rough = isempty(sMap.train_sequence);

% epochs
if nargin < 3 | any(isnan(epochs)) | isempty(epochs),
  tlen_base = ceil(prod(sMap.msize) * 5 / dlen);
  if strcmp(sMap.train_type,'seq'), tlen_base = tlen_base*dlen; end
  rough_tlen = tlen_base;
  tlen = 3*tlen_base;
else 
  do_rough = 0;
  if strcmp(sMap.train_type,'seq'), tlen = epochs*dlen; 
  else tlen = epochs; end
end

% radius
if nargin < 4 | any(isnan(radius)) | isempty(radius),
  fine_radius = ceil(max(sMap.msize)/10); 
  rough_radius = [ceil(max(sMap.msize)/2), fine_radius];
  radius = [fine_radius 1];
else 
  do_rough = 0;
  if length(radius)==1 & tlen>1, radius = [radius 1]; end
end

% alpha
if nargin < 5 | any(isnan(alpha)) | isempty(alpha),
  rough_alpha = 0.5;
  alpha = 0.05; 
else 
  do_rough = 0;
  if strcmp(sMap.train_type,'batch'), 
    warning('Learning coefficient alpha is not used in batch algorithm.');
  end
end

% alpha_type
if nargin < 6 | any(isnan(alpha_type)) | isempty(alpha_type),
  alpha_type = 'linear';
end

% tracking
if nargin < 7 | any(isnan(tracking)) | isempty(tracking), tracking = 1; end
  
% handle 1/t learning rate type
if strcmp(alpha_type,'inv'), 
  % The inverse of time function is of the form
  %     alpha(t) = a / (t+b)
  % where a and b are suitably chosen constants. 
  % Below, they are chosen such that alpha_fin = alpha_ini/100.
  if do_rough, 
    trlen = rough_tlen - 1; b = trlen/99;
    rough_alpha = rough_alpha * b ./ (b + [0:trlen]);
  end
  trlen = tlen - 1; b = trlen/99;
  alpha = alpha * b ./ (b + [0:trlen]); 
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

% rough ordering
if do_rough, 
  fprintf(1,'Training 1st phase (rough ordering):\n');
  sMap = train(sMap, D, rough_tlen, rough_radius, rough_alpha, alpha_type, tracking);
  fprintf(1,'Training 2nd phase (fine-tuning):\n');
end  

% fine-tuning
sMap = train(sMap, D, tlen, radius, alpha, alpha_type, tracking);
             
return;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% subfunctions

function [sMap] = train(sMap,D,tlen,rad,alpha,alpha_type,track)

  algorithm = sMap.train_type;
  M = sMap.codebook;
  lattice = sMap.lattice;
  shape = sMap.shape;
  neigh = sMap.neigh;
  mask = sMap.mask;

  switch algorithm, 
  case 'seq', 
    [sMap.codebook qe] = som_seqtrain(M, lattice, shape, neigh, ...
                                      D, tlen, rad, alpha, mask, track);
  case 'batch',
    [sMap.codebook qe] = som_batchtrain(M, lattice, shape, neigh,...
                                        D, tlen, rad, mask, track);
  otherwise, 
    error(['illegal train type: ', algorithm]);
  end

  % create train sequence struct and insert it to map struct
  sTrain = struct('algorithm', algorithm, 'radius_ini', rad(1), 'radius_fin', rad(length(rad)), 'alpha_ini', alpha(1), 'alpha_type', alpha_type, 'trainlen', tlen, 'qerror', qe, 'time', datestr(now,0));
  l = length(sMap.train_sequence);  % no of trainings done before this one
  sMap.train_sequence{l+1} = sTrain; 
  
% end of trackplot
