function som_clear(type, p)

%SOM_CLEAR Clear hit marks, labels or trajectories from current figure. 
%
% som_clear([type], [p])
%
% ARGUMENTS ([]'s are optional) 
%        
%  [type] (string) which markers to delete, 'Hit' or 'Lab' or 'Traj' or 'Sel'
%           default: 'all' (see below)
%  [p]    (vector|string) subplot number vector or string 'all'. 
%           If this is not given, the target is the whole current figure 
%           object.
%
% If no value (or empty matrix)  is given for p, the function operates on
% every axis in the current figure. It simply searches for the objects with
% certain 'tags'. It doesn't matter if the figure objects are created by
% SOM Toolbox -functions.
%
% However, if vector p or string 'all' _is_ given, the figure has to be
% created by SOM_SHOW.
%  
% Input argument 'type' may be
%
%        'Hit'   Hit marks (som_addhits, som_planeH)
%        'Lab'   Labels (som_addalbels, som_planeL)  
%        'Traj'  Trajectories (som_addtraj, som_planeT)
%        'Sel'   Deletes all objects created by som_manualclassify 
%                (in all figures)
%
% EXAMPLES 
%          
%   som_clear;
%      % deletes all labels, hit marks and trajectories in the figure
%      % (and objects created by som_manualclassify everywhere)
%   som_clear('hit');
%      % deletes all the hit marks in the current figure
%   som_clear('lab',[1 2]);
%      % deletes labels in SOM_SHOW figure subplots 1 and 2. 
%
% See also SOM_PLANEH, SOM_PLANEL, SOM_PLANET,SOM_ADDHITS, SOM_ADDLABELS,
%          SOM_ADDTRAJ, SOM_MANUALCLASSIFY, SOM_SHOW

% Version 1.0beta Johan 061197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%%% Check number of arguments

error(nargchk(0,2, nargin))     % check no. of input args is correct
error(nargchk(0,0, nargout))    % check no. of output args is correct

%%% Initialize & check & action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargin == 0 | isempty(type) | strcmp(type,'all') % delete everything 
                                                    % in the gcf
  delete(findobj(gcf,'Tag','Hit'));
  delete(findobj(gcf, 'Tag','Lab'));
  delete(findobj(gcf, 'Tag','Traj'));
  delete(findobj('Tag','Sel'));       % delete Sel everywhere!
  return
end

if nargin < 2 | isempty(p)            % check handles
  handle=gcf;                       
else                                  % check subplot handles if p is given
  [handle,msg]=som_figuredata(p,gcf);
  if ~isempty(msg)
    error('2nd argument invalid or figure not made by SOM_SHOW: try SOM_CLEAR without arguments.');
    end
end

switch lower(type)                    % check type & make proper tag names
case 'hit'  
  tag = 'Hit'; 
case 'lab'
  tag = 'Lab';
case 'traj'
  tag = 'Traj';                     
case 'sel'
  delete(findobj('Tag','Sel'));
  return;
otherwise                             
  error('Invalid object tag. Must be {lab | hit | traj | sel}');
end

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for i=1:length(handle),
  h=findobj(handle(i),'Tag',tag);     % find object handles 
  delete(h);                          % delete objects
end				

%%% No output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

