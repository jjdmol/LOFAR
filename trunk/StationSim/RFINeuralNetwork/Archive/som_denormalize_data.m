function sData = som_denormalize_data(sData, varargin)

%SOM_DENORMALIZE_DATA Denormalize data matrix of a data struct.
%
% sData = som_denormalize_data(sData, [normalization])
%
% ARGUMENTS
%
%  sData           (struct or matrix) data struct with normalized data 
%                   matrix or a normalized data matrix
%  [normalization] (struct) normalization struct
%
% RETURNS
% 
%  sData           (struct) data struct with denormalized data matrix
%
% If the first argument is a matrix, the second argument (normalization
% struct) MUST BE specified.
% If the first argument is a struct, the second argument need not to be
% specified; if it is, it overrides the normalization in the data struct.
%
% See also SOM_DATA_STRUCT, SOM_NORMALIZE_DATA.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% kk 161097, ecco 231097

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 2, nargin))  % check no. of input args is correct

if isstruct(sData)
  if isempty(sData.normalization.name)|isempty(sData.normalization.inv_params)
    warning('Did not denormalize: the data are not in normalized form');
    return;
  else 
    name       = sData.normalization.name;
    inv_params = sData.normalization.inv_params;
    data       = sData.data;
  end
else
  if nargin == 1
    error('Missing second argument (normalization struct)')
  end
  data = sData;
end

if nargin == 2
  if ~isstruct(varargin{1})
    error('The second argument is not a normalization struct')
  end
  name       = varargin{1}.name;
  inv_params = varargin{1}.inv_params;
end  

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% perform denormalization

switch name
  case 'som_var_norm'
    data_mean = inv_params(1, :);
    data_sdev = inv_params(2, :);
    data      = data * sparse(diag(data_sdev)) ...
	+ ones(size(data)) * sparse(diag(data_mean));
  
  case 'som_lin_norm'
    data_min  = inv_params(1,:);
    data_max  = inv_params(2,:);
    data      = data * sparse(diag(data_max - data_min)) ...
	+ ones(size(data)) * sparse(diag(data_min));
    
  case 'som_hist_norm'
    % find number of bins (on the last row of sData.normalization.inv_params)

    n_bins = inv_params(size(inv_params, 1),:);
    
    % expand data from unit interval to integer values 1,...,n_bins

    data   = round(data * sparse(diag(n_bins)));
    for j = 1:size(data, 2)                    % process each column separately
      if any(isnan(data(:, j)))
	data(isnan(data(:, j)), j) = n_bins(j); % NaN's can't be indices
      end
      data(:, j) = inv_params(data(:, j), j);
    end
  
  case 'som_unit_norm'
    data = (data' * sparse(diag(inv_params)))';    
    
  otherwise
    error(['Unknown normalization method: ' name])
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% return result in correct form

if isstruct(sData)
  sData.data                     = data;
  sData.normalization.inv_params = [];  % to indicate the data are unnormalized
else
  sData = data;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
