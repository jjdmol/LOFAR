function [sTo] = som_label(sTo, inds, Labels, mode)

%SOM_LABEL Give/clear labels to/from map or data struct.
%
% sTo = som_label(sTo, Inds, Labels, [mode])
%  or
% sTo = som_label(sTo, 'clear')
%
% ARGUMENTS ([]'s are optional)
%
%  sTo     (struct) data or som struct to which the labels are put 
%  inds    (vector or matrix or string) indexes of the vectors to 
%           which the labels are put, size n x 1. For a map, also 
%           a matrix form is supported, size n x mdim, so that the 
%           unit may be specified using its subscripts in the 
%           codebook matrix. String 'all' is the same as [1:n], 
%           where n is the number of vectors in the struct.
%  Labels  (string matrix or cell matrix of strings or 
%           cell array of cells - see below) the labels themselves 
%  [mode]  (string) 'add' or 'replace' or 'clear', default is 'add'
%
% RETURNS
%
%  sTo      (struct) the modified structure 
% 
% NOTE: all empty labels ('' or []) are ignored in Labels variable. 
% NOTE: at the end of the function, the label list of each (given)
%       vector is 'pruned': all empty labels are either removed
%       or packed to the end of the list (removing might not be 
%       possible, since in the case of data struct all vectors must
%       have an equal number of labels).
% 
% The labels can be given in three forms: 
%   1. string matrix (or simply a string if only a single index is given) 
%      each label is referred to as: Labels(i,:)
%      e.g. ['label1'; 'label2'; 'label3']
%      NOTE: all labels must be of equal length
%   2. cell matrix of strings
%      each label is referred to as: Labels{i,j}
%      e.g. {'l1', 'label2'; 'l3', ''; 'a_long_label', 'last label'}
%      NOTE: all rows must have equal number of items
%   3. cell array of cell arrays of strings
%      each label is referred to as: Labels{i}{j}
%      e.g. {{'l1','label2'}; {'l3'}; {'a_long_label','last label'}}
% All labels in a row (either Labels{i,:} or Labels{i}{:}) are 
% handled as a group, and are given to the same vector. 
% If the mode is 'clear', the Labels is ignored.
%
% The modes are:
%   'add':     just adds the given labels
%   'replace': first removes all labels from indicated vectors (performs
%              a 'clear') and then adds new ones 
%   'clear':   removes all labels from indicated vectors
%
% EXAMPLES
%
%  This is the basic way to add a label to map structure:
%   sMap = som_label(sMap,[3 4],'label')
%
%  The following examples have identical results (assuming 10x5 map): 
%   sMap = som_label(sMap,[4; 13],   ['label1'; 'label2'])
%   sMap = som_label(sMap,[4 1; 3 2],{'label1'; 'label2'})
%   sMap = som_label(sMap,[4; 13],   {{'label1'};{'label2'}})
%
%  Labeling the BMU of a vector x
%   sMap = som_label(sMap,som_bmus(sMap,x),'BMU')
%
%  To 'prune' the label lists of a struct
%   n = prod(sMap.msize); sMap = som_label(sMap,[1:n]',cell(n,1));
%
%  Clearing labels from a struct
%   sMap = som_label(sMap,'clear')           % clear all
%   sMap = som_label(sMap,'all',[],'clear')  % clear all
%   sMap = som_label(sMap,[1:10]',[],'clear')  
% 
% See also SOM_AUTOLABEL, SOM_HITS, SOM_BMUS, SOM_ADDLABELS, SOM_PLANEL.     

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 101297 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 4, nargin));  % check no. of input args is correct

% sTo
if isstruct(sTo)
  if isfield(sTo, 'data'),         to = 'data'; % data struct
  elseif isfield(sTo, 'codebook'), to = 'map';  % map struct
  else error('First argument is an invalid struct.');
  end
else 
  error('First argument should be a map or data struct.');
end

% clear all labels? 
if (nargin==2 & strcmp(inds,'clear')) | ...
   (nargin==4 & strcmp(inds,'all') & strcmp(mode,'clear')), 
  if strcmp(to,'map'), sTo.labels = cell(sTo.msize);
  else sTo.labels = cell(size(sTo.data,1),1); end
  return;
end

% inds - indexes of vectors
% should be a scalar, vector, matrix or string
if ~isnumeric(inds), 
  if strcmp(inds,'all'), 
    if strcmp(to,'map'), inds = [1:prod(sTo.msize)]'; 
    else inds = [1:size(sTo.data,1)]'; end
  else
    error('Invalid second argument.')
  end
end
n = size(inds,1); 
d = size(inds,2); 
if d>1, 
  if strcmp(to,'data'), 
    error('Cannot refer to data vectors with multiple subscripts.');
  elseif d ~= length(sTo.msize), 
    error('Wrong number of subscripts.')
  else
    % convert to single index form
    inds = som_sub2ind(sTo.msize,inds);
  end
end

% mode, label_inds
if nargin==4, 
  if ~strcmp(mode,'add') & ~strcmp(mode,'replace') & ~strcmp(mode,'clear') & ~strcmp(mode,'prune'), 
    error('Mode should be add, replace, clear or prune.')
  end
else
  mode = 'add'; 
end

% Labels
nl = size(Labels);
if iscell(Labels) & iscell(Labels{1}) & nl(2)>1, 
  if nl(1)==1,  Labels = Labels'; nl = size(Labels);
  else nl(2)>1, error('Cannot use a cell matrix of cells.');
  end
end
if strcmp(mode,'clear') | strcmp(mode,'prune'), % Labels are ignored
else  
  if nl(1) ~= n, error('The number of labels and indexes does not match.'); end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize

% convert Labels to a cell array of cells
if strcmp(mode,'add') | strcmp(mode,'replace'), 
  if iscell(Labels), 
    tmplab = Labels; Labels = cell(nl(1),1);  
    for i=1:nl(1), 
      if ~iscell(tmplab{i}) 
        if ~isempty(tmplab{i}), Labels{i} = tmplab(i,:);
        else Labels{i} = {}; end
      else
        Labels(i) = tmplab(i);
      end
    end
    clear tmplab;
  elseif ~iscell(Labels), % string matrix
    tmplab = Labels;
    Labels = cell(nl(1),1);
    for i=1:nl(1), Labels{i} = {tmplab(i,:)}; end
    clear tmplab;
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

% since labels are saved a little bit differently in map and 
% data structs, they have to be handled separately

if strcmp(to,'map'), 

  switch mode, 
  case 'clear', 
    for i=1:n, sTo.labels{inds(i)} = []; end 
  case 'add', 
    for i=1:n, 
      l0 = length(sTo.labels{inds(i)});
      l1 = length(Labels{i});
      sTo.labels{inds(i)}(l0+(1:l1)) = Labels{i};
    end 
  case 'replace', 
    for i=1:n, 
      sTo.labels{inds(i)} = Labels{i};
    end 
  end

else % the target is a data structure

  switch mode, 
  case 'clear', 
    sTo.labels(inds,:) = cell(n,size(sTo.labels,2));
  case 'add', 
    l0 = size(sTo.labels,2);
    for i=1:n, 
      l1 = length(Labels{i});
      for j=1:l1, sTo.labels{inds(i),l0+j} = Labels{i}{j}; end 
    end
  case 'replace', 
    if all(inds == [1:size(sTo.data,1)]), d = 1; 
    else d = size(sTo.labels,2); end
    sTo.labels(inds,:) = cell(n,d);
    for i=1:n, 
      l1 = length(Labels{i});
      for j=1:l1, sTo.labels{inds(i),j} = Labels{i}{j}; end 
    end
  end

end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% cleanup 

% prune empty labels out

if strcmp(to,'map'), 

  for i=1:n,
    l0 = length(sTo.labels{inds(i)});
    if l0, 
      select = zeros(1,l0); 
      for j=1:l0, select(j) = ~isempty(sTo.labels{inds(i)}{j}); end
      if any(select), sTo.labels{inds(i)} = sTo.labels{inds(i)}(find(select));
      else sTo.labels{inds(i)} = []; end
    end
  end

else  % sTo is data struct

  l0 = size(sTo.labels,2);
  select = zeros(1,l0); 
  for i=1:n,
    for j=1:l0, select(j) = ~isempty(sTo.labels{inds(i),j}); end
    s = sum(select);
    if s>0, 
      tmplab = sTo.labels(inds(i),find(select));
      sTo.labels(inds(i),:) = cell(1,l0);
      sTo.labels(inds(i),1:s) = tmplab;
    end
  end
  % prune empty columns (except leave first column)
  select(1) = 1;
  for j=2:l0, select(j) = ~isempty([sTo.labels{:,j}]); end
  sTo.labels = sTo.labels(:,find(select));

end



