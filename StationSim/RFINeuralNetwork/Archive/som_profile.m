function h=som_profile(sMap, fcn, scale, m)

%SOM_PROFILE Draws the model vectors as bar, pies or plots.
%
% h=som_profile(sMap, [fcn], [scale], [m])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap    (map struct)
%  [fcn]   (string)     script (see below)
%  [scale] (string)     'normalized' or 'denormalized' (see SOM_SHOW)
%  [m]     (matrix)     (size sMap.msize) if m(i,j)<=0 the model vector is 
%                       not drawn
% RETURNS   
%
%  h     (vector) handles to the subplots
%
% This function draws each model vector as a subplot using some
% visualization method as bar or pie plot.
% 
% The string fcn is evaluated by EVAL fucntion. However, there are five
% ''preprogrammed'' styles: 'PLOT', 'PLOT_AXIS_OFF', 'BAR', 'BAR_AXIS_OFF' 
% and 'PIE'.
%  
% 'BAR' uses function BAR to plot the model vectors. Axes are set so that
% y-axis minimum is the same as the lowest value in the codebook and the 
% maximum is the biggest value in the codebook. Each subplot uses the same
% scale. 'BAR_AXIS_OFF' works identically, but turns the axes off. 
% 'BAR_COLOR' gives different color to each bar.
% 
% 'PLOT' Same as 'BAR' but uses function plot. 'PLOT_AXIS_OFF'
% works identically, but turns the axes off.
%
% 'PIE' uses function PIE to plot the model vectors. The preset colormap is
% used. Texts showing the percentage are deleted. 
%
% NOTE: You can give any other string for fcn to be evaluated by EVAL. 
% The model vector is in the variable v. Reserved variables in the workspace 
% of this function are v, fcn, h, h_, i, j, m nad scale. 
% For example 'BAR' sets fcn to 
% 'bar(v); axis([0.5 size(sMap.codebook,3)+.5 min(sMap.codebook(:)) max(sMap.codebook(:))]); set(gca,''xtick'',[]);'
%
% See also PLOT, BAR, PIE, EVAL.
%
% EXAMPLES
%
% som_profile(sMap,'PLOT','denormalized',som_hits(sMap,sData)>10);
%  % Plots model vectors for map units where are over 10 data hits of sData.
%  % The original data scaling is used.
% som_profile(sMap,'bar(v);axis([0 10 1 5]);axis off;hold on');
%  % plots model vectors as bar plots using specified axis limits,
%  % turns axis off, and hold on for further plots
%  % The denormalization is not made.
% som_profile(sMap,'plot(v(1:5),''r'');plot(v(6:10),''g'');');
%  % plots 5 first components with red and five next with green to the
%  % same plot

% Version 1.0beta Johan 181297 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             


%%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(1, 4, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct


%%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargin < 4 | isempty(m)
  m=ones(sMap.msize);
end

if size(m) ~= sMap.msize
  error('Size mismatch');
end

if nargin < 3 | isempty(scale)
  scale='normalized';
end

switch scale
case{ 'normalized' ,'denormalized'}
  ;
otherwise
  error('String ''normalized'' or ''denormalized'' expected.');
end
  
if nargin < 2 | isempty(fcn)
  fcn='BAR';
end

switch fcn
case 'BAR'
  fcn='bar(v); axis([0.5 size(sMap.codebook,3)+.5 min(sMap.codebook(:)) max(sMap.codebook(:))]); set(gca,''xtick'',[]);';
case 'PIE'
  fcn='delete(findobj(pie(v),''type'' ,''text''));';
case 'PLOT'
  fcn='plot(v); axis([0.5 size(sMap.codebook,3)+.5 min(sMap.codebook(:)) max(sMap.codebook(:))]);';
case 'BAR_COLOR'
  fcn='v=[v''; v''];bar(v); axis([0.6 1.4 min(sMap.codebook(:)) max(sMap.codebook(:))]);set(gca, ''xtick'',[]);';
case 'BAR_AXIS_OFF'
  fcn='bar(v); axis([0.5 size(sMap.codebook,3)+.5 min(sMap.codebook(:)) max(sMap.codebook(:))]);axis off';
case 'PLOT_AXIS_OFF'
  fcn='plot(v); axis([0.5 size(sMap.codebook,3)+.5 min(sMap.codebook(:)) max(sMap.codebook(:))]); axis off';
  end

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if strcmp(scale,'denormalized')
  sMap=map_denormalize(sMap);
end

h_=[];
for i=1:sMap.msize(1),
  for j=1:sMap.msize(2),
    if m(i,j),
      h_(i,j)=subplot(sMap.msize(1),sMap.msize(2),(i-1)*sMap.msize(2)+j);
      v=shiftdim(sMap.codebook(i,j,:));
      eval(fcn)
    end
  end
end

if strcmp(sMap.lattice,'hexa') 
  for i=2:2:sMap.msize(1),
    for j=1:sMap.msize(2),
      if ~isempty(h_) & h_(i,j)~=0,    
	pos=get(h_(i,j),'position');
	set(h_(i,j),'position',[pos(1)+pos(3)/2 pos(2:4)]);
      end
    end
  end
end

%%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, h=h_(:); end                 % Set h only, 
                                           % if there really is output

%% SUBFUNCTION %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function map2=map_denormalize(map)

k=0;
for i=1:map.msize(1)
  for j=1:map.msize(2)
    k=k+1;
    data(k,:)=shiftdim(map.codebook(i,j,:))';
  end
end

data=som_denormalize_data(data,map.normalization);
map2=map;
k=0;
for i=1:map.msize(1)
  for j=1:map.msize(2)
    k=k+1;
    map2.codebook(i,j,:)=data(k,:);
  end
end











