function  sData = som_normalize_data(sData, arg)

%SOM_NORMALIZE_DATA Normalize data matrix of a data struct.
%
% sData = som_normalize_data(sData, [arg])
%
% ARGUMENTS ([]'s are optional)
%
%  sData    (struct or matrix) data struct with unnormalized data matrix
%            or data matrix
%  [arg]    (string or struct) If 'arg' is a string, it defines 
%            the normalization method; if this differs from the one 
%            defined in data struct, AND the data have already been  
%            normalized, an error results.
%            If 'arg' is a normalization struct, it is used to
%            normalize the data.
%            The available normalization methods are: 
%             'som_var_norm' : normalize variance of each component to 1
%                              and mean to 0
%             'som_lin_norm' : normalize each component between [0,1]
%             'som_hits_norm': histogram equalization for each component
%             'som_unit_norm': normalizes each vector to unit length
%
% RETURNS
% 
%  sData    (struct or matrix) data struct or matrix with normalized 
%            data matrix
%
% If the first argument is a data struct and normalization parameters 
% do not exist (normalization.inv_params field of data struct is empty), 
% they are determined from data. 
%
% If the first argument is a matrix, the second argument MUST BE specified
% (and it must be a normalization matrix).
%
% See also SOM_DATA_STRUCT, SOM_DENORMALIZE_DATA.

% In the following, the contents of the field 'normalization' of 
% the data struct is discussed in detail (for more information about data 
% struct, try 'help som_data_struct').
%
% The contents of the matrix normalization.inv_params depend on the value of 
% normalization.name in the following way:
%
% - normalization.name == 'som_var_norm': 
%   * inv_params(1,:) contains the means of data components
%   * inv_params(2,:) contains the standard dev. of data components
%
% - normalization.name == 'som_lin_norm': 
%   * inv_params(1,:) contains the minimum values of data components
%   * inv_params(2,:) contains the maximum values of data components
%
% - normalization.name == 'som_hist_norm': 
%    * inv_params(size(inv_params,1),:) contains n_bins, the number of 
%      different values for each data component
%    * inv_params(1:n_bins(j),j) contains the (maximum) values of 
%      data component j that go to each bin
%
% - normalization.name == 'som_unit_norm': 
%   * inv_params is a vector which consists of lengths of data vectors

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta kk 161097, ecco 231097, ecco 041197

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 2, nargin))  % check no. of input args is correct

if isstruct(sData)       % check are we given a data struct or matrix
  data = sData.data;
  isdatastr = 1;
  if nargin == 2         % 'arg' specified
    if isstr(arg)        % is it a string or not?
      name       = arg;
      inv_params = [];
    else
      name       = arg.name;
      inv_params = arg.inv_params;
    end
    if ~strcmp(sData.normalization.name, name)
      if ~isempty(sData.normalization.inv_params) 
	error('Inverse parameters not compatible with normalization method');
      end
      warning(['Changing normalization method from ' sData.normalization.name ' to ' name]);
    end
  else
    name       = sData.normalization.name;
    inv_params = sData.normalization.inv_params;
  end
else
  if nargin == 1
    error('If the first argument is a matrix, the second argument MUST BE specified');
  end
  data       = sData;
  name       = arg.name;
  inv_params = arg.inv_params;
  isdatastr  = 0;
end
if sum(sum(isinf(data))) > 0, 
  error('Data contains infinite elements');
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% perform normalization

switch name
  case 'som_var_norm'
    if isempty(inv_params)
      data_mean = mean(data);
      if sum(isnan(data_mean)) > 0 
	for j = 1:size(data, 2)
	  if isnan(data_mean(j))     % NaN's in this component
	    [data_mean(j) data_sdev(j) dummy] = som_nanops(data(:, j));
	  else                         % no NaN's in this component
	    data_sdev(j) = std(data(:,j));
	  end
	end
      else                             % no NaN's in any component
	data_sdev = std(data); 
      end
      inv_params = [data_mean; data_sdev];
    else                               % inv_params already defined
      data_mean = inv_params(1,:);
      data_sdev = inv_params(2,:);
    end
    data_sdev(data_sdev == 0) = inf;   % to avoid division by zero; 1/inf = 0
    data = (data - ones(size(data))* ...
	sparse(diag(data_mean))) * sparse(diag(1 ./ data_sdev));
  
  case 'som_lin_norm'
    if isempty(inv_params) 
      data_min   = min(data);
      data_max   = max(data);
      inv_params = [data_min; data_max];
    else                               % inv_params already defined
      data_min = inv_params(1,:);
      data_max = inv_params(2,:);
    end
  
    delta = data_max - data_min;
    delta(delta == 0) = inf;           % to avoid division by zero; 1/inf = 0
    data = (data - ones(size(data)) * ...
	sparse(diag(data_min))) * sparse(diag(1 ./ delta));
  
  case 'som_hist_norm'

    % the basic idea of som_hist_norm:
    % - first, divide the data componentwise into bins so that equal values
    %   always go to the same bin; thus, the maximum number of bins is
    %   N, the number of data vectors
    % - then, normalize components to unit interval so that all components
    %   have equal weight, regardless of the number of bins of each component

    if isempty(inv_params), 
      [data_sorted indices] = sort(data);
      tmpdata = zeros(size(data,1),1);
      inv_params =  ones(size(data)) * inf; % placeholders for unused elements
      for j = 1:size(data, 2),         % process each column separately
	newval = 1;
	fprintf(1,['\rsom_normalize_data: normalizing field #' ...
	      int2str(j) '/' int2str(size(data,2))]);
	for i = 1:(size(data,1) - 1),
	  if data_sorted(i+1,j) > data_sorted(i,j) ...
		| (isnan(data_sorted(i+1,j)) & ~isnan(data_sorted(i,j))),
	    tmpdata((data(:, j) == data_sorted(i, j))) = newval;
	    inv_params(newval, j) = data_sorted(i, j);
	    newval = newval + 1;
	  end
	end
	if isnan(data_sorted(i+1,j)),    % NaN's come last in sorted data
	  tmpdata(isnan(data(:, j))) = newval; 
	else
	  tmpdata((data(:, j) == data_sorted(i + 1, j))) = newval;
	end;
	data(:,j) = tmpdata;
	inv_params(newval, j) = data_sorted(i + 1, j);
	n_bins(j) = newval;
      end
      fprintf(1,'\n');
      inv_params(sum(isinf(inv_params)') == size(inv_params,2),:) = [];
      % remove unused rows, which contain only inf's
      inv_params = [inv_params; n_bins];
      
    else                                  % inv_params already defined
      n_bins = inv_params(size(inv_params,1),:);
      for j = 1:size(data, 2)        
	for i = 1:size(data, 1)
	  if ~isnan(data(i, j)),
	    [d ind] = min(abs(data(i, j) - inv_params(1:n_bins(j), j)));
	    if (data(i, j) - inv_params(ind, j)) > 0 & ind < n_bins(j),
	      data(i, j) = ind + 1;   % data item closer to the left-hand
	    else                            % bin wall, indexed after RH wall
	      data(i, j) = ind;
	    end
	  end
	end
      end
    end
    data = data * sparse(diag(1 ./ n_bins));
  
  case 'som_unit_norm'
    data2 = data.^2;
    data2(isnan(data2)) = 0;
    lengths = sqrt(sum(data2'));
    data = (data' * sparse(diag(1 ./ lengths)))';
    inv_params = lengths;

  otherwise,
    error(['Unknown normalization method: ' name])
end

if isdatastr
  sData.data = data;
  sData.normalization.inv_params = inv_params;
  sData.normalization.name       = name;
else
  sData = data;
end
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%A SHORT EXAMPLE:
%
%A = ceil(10*rand(8,6))
%A(:,3) = 0;  
%A(4,5:6) = nan;  
%msd=som_data_struct(A);
%msd.normalization.name='som_hist_norm';
%msd2=som_normalize_data(msd);
%A1 = msd2.data;
%msd3 = som_denormalize_data(msd2)

%testdata = msd2
%testdata.data = [10     9     0     2     5     9];
%testdata1 = mysom_normalize_data(testdata)         

%mean(A1)  
%std(A1)  
%min(A1)  
%max(A1)  

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%