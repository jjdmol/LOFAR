function [me, st, md] = som_nanops(sData)

%SOM_NANOPS NaN operations used by somtoolbox.
%
% [mean, std, median] = som_nanops(data)
%
% ARGUMENTS 
%
%  sData  (struct or matrix) data struct or 
%          data matrix, size dlen x dim
%
% RETURNS
%
%  me     (double) columnwise mean
%  st     (double) columnwise standard deviation
%  md     (double) columnwise median

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 150797, 190997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

(nargchk(1, 1, nargin));  % check no. of input args is correct

if (isstruct(sData))
  D = sData.data;
else
  D = sData;
end

if isempty(D) 
  me = NaN;
  st = NaN;
  md = NaN;
  return;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% computation

dim = size(D, 2);

% compute average and  median

for i = 1:dim,
  ind = find(~isnan(D(:, i))); % indices of non-NaN elements
  n   = length(ind);           % no of non-NaN elements

  me(i) = sum(D(ind, i));      % compute average
  if n == 0
    me(i) = NaN;
  else
    me(i) = me(i) / n;
  end

  md(i) = median(D(ind, i));   % compute median
end


% compute standard deviation

for i = 1:dim,
  ind = find(~isnan(D(:,i)));          % indices of non-NaN elements
  n   = length(ind);                   % no of non-NaN elements

  st(i) = sum((me(i) - D(ind, i)).^2); % compute standard deviation
  if n == 0                         
    st(i) = NaN;
  elseif n == 1
    st(i) = 0;
  else
    st(i) = sqrt(st(i) / (n - 1));
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

