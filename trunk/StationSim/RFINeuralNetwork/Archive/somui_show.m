function [h_ax, h_clbr] = somui_show(sfin, h_sf)
%SOMUI_SHOW Shows the comp planes and U-matrix of SOMUI_VIS GUI.
%
% [h_ax, h_clbr] = somui_show(sfin, h_sf)
%
% ARGUMENTS
%
%   sfin  (struct)	SOMfig structure
%   h_sf  (handle)	to SOMfig figure	
%
% RETURNS
%
%   h_ax     (handle)	axes
%   h_clbr   (handle)	colorbars
%
% SOMUI_SHOW is a modification to SOM_SHOW. 
%
% See also SOM_SHOW, SOMUI_FIG, SOMUI_VIS

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments

error(nargchk(1, 2, nargin))  % check no. of input args is correct
error(nargchk(0, 2, nargout)) % check no. of output args is correct

% Chech argument 'sfin'
if (isstruct(sfin))
  if (isfield(sfin,'issfstruct'))
    if (isfield(sfin,'map'))
      sMap = sfin.map;
    else
      error('No map in SOMfig structure.');
    end;
  else
    error('Argument not a SOMfig struct.\n');
  end;
comps = sfin.use_comps;
end;


d=size(sMap.codebook,3);      			% numb. of components
s=[size(sMap.codebook,1) size(sMap.codebook,2)];% size of the map

% FUTURE: Create sfin.M
M=ones(s);             

if ndims(comps) > 2 | min(size(comps)) ~= 1
  error('The argument comps has to be a vector or string ''all''');
end

if isstr(comps)     % check the string 'all'
  switch(comps)
  case 'all',
    comps=[0:d];
    if min(comps)<0 | max(comps)> d
      error('The argumnet comps has negative indexes or indexes greater than dim');
    end
  end
end
if ndims(M) ~= 2
  error('The argument M has to be a n1xn2 matrix');
end

if s ~= size(M)
  error('The size of M is inconsistent to the size of the component som_planes')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Initialization                      

n=length(comps);                  % the number of subfigures
c=setdiff(unique(comps),0);       % get the unique component indexes
c=c(~isnan(c));                   

                                  % Estimate the suitable dimensions for
y=ceil(sqrt(n));                  % the subplots
x=ceil(n/y);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

%% figure(h_sf);   taako mattaa 161097 -- UI:t katoaa jossain vaiheesa!

if d>1                            % calculate U-matrix with all comps
  umat0=som_umat(sMap);          
elseif sum(comps == 0)            % U-matrix (all) requested?              
  umat0=ones(s)*NaN;              % som_umat can't compute, set to NaN.
  warning('Didn''t compute u-matrix: must be at least two  components!');
end

if length(c)>1 & sum(isnan(comps)), % calculate U-matrix with selected comps
  umat1=som_umat(sMap);
elseif sum(isnan(comps))          % U-matrix (selected) requested?
  umat1=ones(s)*NaN;              % som_umat can't compute, set to NaN
  warning('Didn''t compute u-matrix: must be at least two components!');
end


% decide the subplot axis size
% fsize stands for how big the area for subplots is

fsize = [0.02 0.02 0.96 0.96];		% without any GUI or titles
					% symmetric, little space left
if (sfin.sf_params.show_gui)
  fsize = fsize + [0 0.16 0 -0.16];		% see frames in somui_fig
end;
if (sfin.sf_params.show_ti)
  fsize = fsize - [0 0 0 0.06];
  if (isempty(sfin.title))
      % FUTURE: title = input('\nGive the title for the SOMfig : ','s');
	%%% ATT.: somui_chtitle
      title = sfin.map.name;    
    if (isempty(title))
      sfin.title = sfin.map.name;
    else
      sfin.title = title;
    end;
  end;
  % Give a title for the figure:
  h_title = somui_text([0.02 0.94 0.96 0.04], 2, 0.8, 'TitleString', ...
	sfin.title);
  sfin.h_title = h_title;
end;


% Important saving:
set(gcf,'UserData',sfin);

% decides how big one subplot is
deltax = fsize(3)/y;		% Horizontal lenght of each subplot
deltay = fsize(4)/x;		% Vertical height of each subplot

% define offsets for subplots
% if offset_coef = [1 1], then a map uses all the space in subplots
% Matlab's default subplot function uses about 0.75 (?)

offset_coef = [.8 .8];		% offset coefficents x and y
offset = ([1 1] - offset_coef)*0.5 .* [deltax deltay];
				% from its own subplot area
axis_size = offset_coef .* [deltax deltay];

%%& deciding spsize
a = deltax * (0:(y-1));
b = (((x-1):-1:0) * deltay)';
a = repmat(a,x,1)';
b = repmat(b,1,y)';
a = reshape(a,x*y,1);
b = reshape(b,x*y,1);
a = a + fsize(1) + offset(1);
b = b + fsize(2) + offset(2);
c = repmat(axis_size(1),x*y,1);
d = repmat(axis_size(2),x*y,1);

spsize = [a b c d];


% Back to som_show routines:

for i=1:n,                         % Main loop
  h_ax_(i,1)=subplot('position',spsize(i,:)); % open a new subplot
  if comps(i) == 0 
    som_planeU(sMap.lattice, umat0);
    h_label=xlabel('U-matrix (all components)');
  elseif isnan(comps(i))
    som_planeU(sMap.lattice, umat1);
    h_label=xlabel('U-matrix (shown components)');
  else 
    som_plane(sMap.lattice, sMap.codebook(:,:,comps(i)), M);
    h_label=xlabel(strrep(sMap.comp_names{comps(i)},'_','\_'));
  end
  
  % Adjust axis ratios to optimal (only 2D!) and put the
  % title (xlabel) as close to axis as possible
  
  if (sfin.sf_params.show_cn)
    set(h_label,'Visible','on','verticalalignment','top');
  else
    set(h_label,'Visible','off','verticalalignment','top');
  end;
  set(gca,'plotboxaspectratio',[s(2) s(1) s(1)]);
  
  if s(1)>s(2)                    % Select the pos. of colorbar
    dir='vert';                   % according to the maps shape   
  else 
    dir='horiz';
  end

  if (sfin.sf_params.show_cb == 1)
    h_clbr_(i)=colorbar(dir);       % Draw the colorbars to each subplot
  end;

  if (sfin.sf_params.show_ax)
    axis on;
  else
    axis off;
  end;
end

% Do not set the name of the figure - it has already named in somui_vis

% SOM_SHOW. struct is almost like sfin but smaller
% Now we add subplotorder into struct
sfin.subplotorder = h_ax_;
sfin.dim = size(sMap.codebook,3); 

set(gcf, 'UserData', sfin);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% create output if it really is needed

if nargout > 0
  h_ax=h_ax_'; h_clbr=h_clbr_;
end

% 290897 Johan




