function h=som_recolorbar(p, ticks, scale, labels)
%SOM_RECOLORBAR Refresh and  rescale colorbars in the current SOM_SHOW fig.
%
% h=som_recolorbar([p], [ticks], [scaling], [labels])
%
% ARGUMENTS ([]'s are optional) 
%
%  [p]      (vector|string) subplot number vector or string 'all', 
%                            default: 'all'
%  [ticks]  (string|cell)   string 'auto' or 'border', default: 'auto'
%                            or p x 1 cell array of p row vectors
%  [scale]  (string)        string 'denormalized' | 'normalized',
%                            default: 'normalized' 
%  [labels] (cell)          p x 1 cell array of cells containing strings
%
% RETURNS 
%
%  h        (vector)        handles to the colorbar objects.
%
% This function refreshes the colorbars in the figure created by SOM_SHOW.
% Refreshing  is necessary if you have changed the colormap.
% Each colorbar may have letters 'u' and/or 'd' as label. Letter 'd' means
% that the scale is denormalized and 'u' is for user specified labels.
%
% Different argument cominations
%
% 1. Argument 'ticks' has string values:
%
%  - 'auto' for input argument ticks sets the automatic tick
%     marking on (factory default). 
%  - 'border' sets the tick marks to the color borders. This is 
%     convenient if there are only few colors in use. 
%
%  Argument scale controls the scaling of the tick mark label values. 
%  'normalized' means that the tick mark labels are directly the values 
%  of the ticks, that is, they refer to the map codebook values. 
%  Value 'denormalized' scales the tick mark label values back to the original
%  data scaling. This is made using som_denormalize_data.
%
% 2. Argument 'ticks' is a cell array of vectors:
%
%  The values are set to be the tick marks to the colorbar specified by p.
%
%  - if arg. scale is 'normalized' the ticks are set directly to the colorbar.
%  - if arg. scale is 'denormalized' the tick values are first normalized 
%    in the same way as the data.
%  
% Argument 'labels' specify user defined labels to the tick marks
%
% NOTE: ticks are rounded to contain three significant digits.
%
% EXAMPLE
%
%  colormap(jet(5)); som_recolorbar('all','border','denormalized')
%      % Uses five colors and sets the ticks on the color borders.
%      % Tick label values are denormalized back to the original data scaling
%
%  colormap(copper(64));som_recolorbar
%      % changes to colormap copper and resets default ticking and labeling
%
%  som_recolorbar([1 3],{[0.1 0.2 0.3];[0.2 0.4]},'denormalized')
%      % Ticks colorbar 1 by first normalizing values 0.1, 0.2, 0.3 and
%      % then setting the ticks to the colorbar. Labels are of course 
%      % 0.1, 0.2 and 0.3. Ticks colorbar 3 in the same way using values
%      % 0.2 and 0.4.
%
%  som_recolorbar([2 4],{{0.1 0.2};{-1.2 3}},'normalized',{{'1' '2'};{'a' 'b'}})
%      % Ticks colorbar 2 and 4 directly to the specified values. Sets labels
%      % '1' '2' and 'a' 'b' to the ticks.
%
%  som_recolorbar([2 4],{{0.1 0.2};{-1.2 3}},'normalized',{{'1' '2'};{'a' 'b'}})
%      % as previous one, but normalizes tick values first
%
% See also SOM_SHOW, SOM_NORMALIZE_DATA, SOM_DENORMALIZE_DATA

% Version 1.0beta Johan 061197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%% Init & check %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(0, 4, nargin))    % check no. of input args
error(nargchk(0, 1, nargout))   % check no. of output args

% Check the subplot vector p and  get the handles, exit if error
% Default subplot vector is 'all'

if nargin < 1 | isempty(p)                       % default p
  p= 'all';
end

% check SOM_SHOW and get the figure data. Exit, if error

[handles, msg, msize, dim, normalization, comps]=som_figuredata(p, gcf);
error(msg);                                       

if nargin < 2 | isempty(ticks)                   % default tick mode is 'auto'
  ticks = 'auto';
elseif isa(ticks,'cell')                         % check for cell
  tickValues=ticks; 
  ticks= 'explicit';
end
if ~ischar(ticks)                                % invalid argumnet
  error('The second argument should be a string or a cell array of vectors.');
end

switch ticks                                     % check ticks
case { 'auto','border'}
  ;
case 'explicit'
  if length(tickValues) ~= length(handles)
    error('Cell containing the ticks has wrong size.')
  end
otherwise
  error('''auto'' or ''border'' expected for the second argument.');
end

if nargin < 3 | isempty(scale)                   % default mode is normalized
  scale= 'normalized';
end
if ~ischar(scale)                                % check scale type
  error('The third argument should be a string.'); 
end
switch scale                                     % check the string
case { 'normalized', 'denormalized'}
  ;  
otherwise   
  error('''normalized'' or ''denormalized'' expected for the third argument.')
end

if nargin < 4 | isempty(labels)                  % default is autolabeling
  labels = 'auto';
elseif ~isa(labels,'cell')                       % check type
  error('The fourth argument should be a cell array of cells containing strings.')
else
  labelValues=labels;                            % set labels
  labels = 'explicit';
  if length(labelValues) == length(handles)      % check size
    ;
  else
    error('Cell containing the labels has wrong size')
  end
end

%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

n=size(colormap,1)+1;                      % number of colors+1

for i=1:length(handles),                   % MAIN LOOP BEGINS
  
  axes(handles(i));                        % set axes, refres colorbar and  
  h_(i)=colorbar;                          % get colorbar handles
  if comps(i)==-2
    set(findobj(h_(i)),'Visible','off');   % turn off the colorbar of empty
  end                                      % grid

  if comps(i)>0,                           % manipulate component planes only
    colorbardir=get(h_(i),'YaxisLocation');
    switch colorbardir                     % get colorbar direction &
    case 'left'                            % set some strings
      Tick='Xtick'; Lim='Xlim'; LabelMode='XTickLabelMode'; Label='XtickLabel';
    case 'right'
      Tick='Ytick'; Lim='Ylim'; LabelMode='YTickLabelMode'; Label='YtickLabel';
    otherwise
      error('Internal error: unknown value for YaxisLocation'); % fatal
    end                                                         
    
    switch ticks                         
    case 'auto'
      set(h_(i),LabelMode,'auto');        % factory default ticking
      tickValues{i}=get(h_(i),Tick);      % get tick values
    case 'border' 
      limit=caxis;                        
      t=linspace(limit(1),limit(2),n);    % set n ticks between min and max 
      t([1 length(t)])=get(h_(i),Lim); % <- caxis is not necerraily the same 
      tickValues{i}=t;                    % as the colorbar min & max values
    case 'explicit'
      if strcmp(scale,'denormalized')     % normalize tick values
	tickValues{i}=normalize('normalize',tickValues{i},comps(i),dim, ...
	    normalization);
      end
    otherwise  
      error('Internal error: unknown tick type')   % this shouldn't happen
    end
    
    switch labels
    case 'auto'
      switch scale                         
      case 'normalized'
	labelValues{i}=round2(tickValues{i});     % use the raw ones 
      case 'denormalized'                 % denormalize tick values
	labelValues{i}=normalize('denormalize',tickValues{i},comps(i),dim, ...
	    normalization); 
	labelValues{i}=round2(labelValues{i});     % round the scale
      otherwise
	error('Internal error: unknown scale type'); % this shouldn't happen
      end
    case 'explicit'
      ;                                            % they are there already
    otherwise
      error('Internal error: unknown label type'); % this shouldn't happen
    end
    set(h_(i),Tick,tickValues{i});                 % set ticks and labels
    set(h_(i),Label,labelValues{i});            

    % Label the colorbar with letter 'd' if denormalized and 'u' if
    % the labels are user specified
    
    mem_axes=gca;axes(h_(i));
    ch='  ';
    if strcmp(scale,'denormalized')
      ch(1)='d';
    end
    if strcmp(labels,'explicit')
      ch(2)='u';
    end
    xlabel(ch); 
    axes(mem_axes);
  end                                              % MAIN LOOP ENDS 
end                                              

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0
  h=h_;
end

%% Subfunction: DENORMALIZE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function vector=normalize(mode, vector, c, d, normalization)
% DENORMALIZE dernormalizes a vector of tick marks
%
% INPUTS
%  
%   mode          (char)      'normalize' or 'denormalize'
%   vector        (vector)    tick values
%   c             (scalar)    component index
%   d             (scalar)    data dimension
%   normalization (struct)    normalization struct from map struct
%
% The point is that som_denormalize_data denormalizes data vectors
% and the tick marks are from one component only
% This function creates a data matrix where all the values, except
% one component (the tick values) are dummies.

error(nargchk(5, 5, nargin))    % check no. of input args
error(nargchk(1, 1, nargout))   % check no. of output args

if ~isvector(vector)                 % check the vector
  error('Invalid tick values');  
elseif size(vector,2)>size(vector,1) % has to be a vertical vector
  vector=vector';
end

data=ones(length(vector),d);         % create data vectors
data(:,c)=vector;
switch mode
case 'normalize'
    data=som_normalize_data(data,normalization); % denormalize
case 'denormalize'
  data=som_denormalize_data(data,normalization); % denormalize
otherwise
  error('Internal error: unknown normalization mode');
end
vector=data(:,c);                    % take the proper component

%% Subfunction: ROUND2 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ROUND2 rounds the labels to tol significant digits

function r=round2(d)

tol=3;

zero=(d==0);
d(zero)=1;
k=floor(log10(abs(d)))-(tol-1);
r=round(d./10.^k).*10.^k;
r(zero)=0;

%% Subfunction: ISVECTOR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t=isvector(v)
% ISVECTOR checks if a matrix is a vector or not

t=(ndims(v) == 2 & min(size(v)) == 1) & ~ischar(v);
















