function [sTo] = som_autolabel(sTo, varargin)

%SOM_AUTOLABEL Automatical labeling, or clearing of labels.
%
% sTo = som_autolabel(sTo, sFrom, [mode], [inds])
%
% ARGUMENTS ([]'s are optional)
%
%  sTo      (struct) data or som struct to which the labels are put 
%  sFrom    (struct) data or som struct from which the labels are taken
%  [mode]   (string) labeling algorithm: 'add' or 'freq' or 'vote',
%            default is 'add'. 
%  [inds]   (vector) the column-indexes of the labels that are to be
%            used in the operation (e.g. [2] would mean to use only
%            only the second label from each vector of sFrom)
%
% RETURNS
%
%  sTo      (struct) the labeled structure 
% 
% NOTE: all empty labels ('') are ignored.
% 
% The modes:
%  'add':   all labels from sFrom are added to sTo
%  'freq':  only one instance of each label is kept and '(#)', where 
%           # is the frequency of the label, is added to the label.
%           Labels are ordered according to frequency. 
%  'vote':  only the label with most instances is kept
%
% NOTE: the operations (ie. in 'freq' and 'vote' modes) are only 
% performed for the new labels. The old labels are left as they are.
%
% EXAMPLES
%
%  sM = som_autolabel(sM,sD);      % label sM based on sD 
%  sD = som_autolabel(sD,sM);   
%     % label sD based on sM, notice that in this case the 
%     % value of mode does not matter (unless it is 'clear')
%     % because a data vector can only have a single BMU
%  sM = som_autolabel(sM,sD,'vote',[1,2,5]);
%     % label each node of sM with the most frequent label in 
%     % that node using labels 1st, 2nd and 5th labels from sD
%
% See also SOM_LABEL, SOM_HITS, SOM_BMUS, SOM_ADDLABELS, SOM_PLANEL.     

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 101297 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 4, nargin));  % check no. of input args is correct

% sTo
if isstruct(sTo)
  if isfield(sTo, 'data')         % data struct
    to = 'data'; 
    [dlen m] = size(sTo.labels); 
  elseif isfield(sTo, 'codebook') % map struct
    to = 'map';
    si = size(sTo.labels); m = 0;
    for i=1:prod(si), m = max(m,length(sTo.labels{i})); end
  else
    error('First argument is an invalid structure.');
  end
  label_inds = 1:m;
else 
  error('First argument is invalid.');
end

% sFrom
if isstruct(varargin{1})
  sFrom = varargin{1};
  if isfield(sFrom, 'data')         % data struct
    from = 'data'; 
    [dlen m] = size(sFrom.labels); 
  elseif isfield(sFrom, 'codebook') % map struct
    from = 'map';
    si = size(sFrom.labels); m = 0;
    for i=1:prod(si), m = max(m,length(sFrom.labels{i})); end
  else
    error('Second argument is an invalid structure.');
  end
  label_inds = 1:m;
else
  error('Second argument is invalid.');
end

% mode, label_inds
mode = 'add'; 
if nargin > 2,
  if any(strcmp(varargin{2},{'add','freq','vote'})), 
    mode = varargin{2};
  elseif isempty(varargin{2}),
    %ok
  else
    error('Third argument is invalid.');
  end
  if nargin==4,
    label_inds = varargin{3};
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize

% convert the sFrom.labels to a cell array of cells
if strcmp(from,'data'), 
  tmplab = sFrom.labels;
  sFrom.labels = cell(size(sFrom.data,1),1);
  for i=1:size(sFrom.data,1), sFrom.labels{i} = tmplab(i,:); end
  clear tmplab;
end
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

% calculate BMUs
if strcmp(to,'data'), bmus = som_bmus(sFrom,sTo,1);
else bmus = som_bmus(sTo,sFrom,1); end

% for each vector in sTo, make a list of all new labels
if strcmp(to,'data'), Labels = cell(size(sTo.data,1),1); 
else Labels = cell(prod(sTo.msize),1); end
for d=1:length(bmus), 
  m = bmus(d); if strcmp(to,'data'), t = d; f = m; else t = m; f = d; end
  if ~isnan(m), 
    % choose valid indexes
    inds = label_inds(find(label_inds <= length(sFrom.labels{f}))); 
    % add the corresponding labels
    for j=1:length(inds), 
      if ~isempty(sFrom.labels{f}{inds(j)}), 
        Labels{t}{length(Labels{t})+1} = sFrom.labels{f}{inds(j)}; 
      end
    end 
  end
end

if strcmp(mode,'add'), 
  sTo = som_label(sTo,[1:length(Labels)]',Labels,'add'); 
  return; 
end

for i=1:length(Labels),

  % calculate frequency of each label in each node
  new_labels = {};
  new_freq = [];
  for j=1:length(Labels{i}),
    if isempty(Labels{i}{j}), % ignore
    elseif ~any(strcmp(Labels{i}{j},new_labels)), % a new one!
       k = length(new_labels) + 1;
       new_labels{k} = Labels{i}{j};
       new_freq(k) = sum(strcmp(new_labels{k},Labels{i}));
    else, % an old one, ignore       
    end
  end

  % based on frequency, select label(s) to be added 
  if length(new_labels) > 0,
    % sort labels according to frequency
    [dummy order] = sort(1./(1+new_freq));
    new_labels = new_labels(order);
    new_freq = new_freq(order);

    switch mode,
    case 'freq', 
      % replace each label with 'label(#)' where # is the frequency
      for j=1:length(new_labels), 
        labf = sprintf('%s(%d)',new_labels{j},new_freq(j));
        new_labels{j} = labf;
      end
      Labels{i} = new_labels;
    case 'vote',
      % place only the one with most votes 
      Labels{i} = {new_labels{1}};
    end 
  end 

end

sTo = som_label(sTo,[1:length(Labels)]',Labels,'add');





