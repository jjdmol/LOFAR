function [handles,msg,msize,dim,normalization,comps]=som_figuredata(p,f);
%SOM_FIGUREDATA Checks and returns UserData and subplot handles stored by SOM_SHOW
%
% [handles,msg,msize,dim,normalization,comps]=som_figuredata(p, f)
%
% ARGUMENTS ([]'s are optional)
% 
%  [p]      (matrix or string) subplot numbers in a vector or string 'all'
%                               default: 'all'
%  [f]      (double) figure handle, default is current figure
%
% RETURNS
%
%  handles       (vector)   true handles corresp. to p in figure f
%  msg           (string)   error message or empty string (no error)
%  msize         (vector)   map grid size in the fig.
%  dim           (scalar)   map data dimension in the fig.
%  normalization (struct)   normalization struct used in the map in the fig.
%  comps         (vector)   the component indexes in the fig.
%
% This function gets the handles of Componentplane & Umatrix subplot number p
% from the figure f. 
%
% SOM_SHOW writes the handles into the UserData field of the
% figure where their order won't be mixed up. This function reads the 
% data according to the vector p. If the figure is manipulated
% (original planes are missing) the function warns user or returns error
% string.
% 
% NOTE
% 
%  The main purpose for this is to be a subfuncion for SOM_ADD*, 
%  SOM_CLEAR and SOM_RECOLOR functions
%  But it may be used on command line as follows:
%
% EXAMPLE
% 
%  axes(som_show_handles(5)); hold on; text(1,3,'I am here');
%    % plots text on the fifth plane
%
% SEE: SOM_ADDHITS, SOM_ADDTRAJ, SOM_ADDLABELS, SOM_SHOW 

% Version 1.0beta 061197 Johan

%% Check input args %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(0, 2, nargin))  % check no. of input args 
error(nargchk(0, 6, nargout)) % check no. of output args

%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

handles=[];                                     % initialize output
normalize=[];
comps=[];
dim=[];
msize=[];
msg=[];      

cr=sprintf('\n');                               % carriage return            

if nargin < 2 | isempty(f)
  f=gcf;                                        % default figure
end

if nargin < 1 | isempty(p)                      % default p 
  p= 'all';
end

% Find componentplanes and umatrixes from the figure and get the 
% UserData field where the handles for the components are 
% in the original order. If the fields are corrupted, return an error message.

h_real = [findobj(f, 'Tag', 'Componentplane'); ...
    findobj(f, 'Tag', 'Umatrix')];
eval( 'h_stored=getfield(get(f,''UserData''),''subplotorder'');' , ...
    'msg=[ msg cr ''Figure is corrupted. Missing SOM_SHOW.subplotorder''];');  
eval( 'normalization=getfield(get(f,''UserData''),''normalization'');' , ...
    'msg=[msg cr ''Figure is corrupted. Missing SOM_SHOW.normalization''];');
eval( 'comps=getfield(get(f,''UserData''),''comps'');' , ...
    'msg=[msg cr ''Figure is corrupted. Missing SOM_SHOW.comps''];');    
eval( 'msize=getfield(get(f,''UserData''),''msize'');' , ...
    'msg=[msg cr ''Figure is corrupted. Missing SOM_SHOW.msize''];');    
eval( 'dim=getfield(get(f,''UserData''),''dim'');' , ...
    'msg=[msg cr ''Figure is corrupted. Missing SOM_SHOW.dim''];');    
if ~isempty(msg), return; end

%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

index=ismember(h_stored, h_real);  % the original order for plot axes 

if ~prod(index)                    % missing planes?!
  l1= 'Some of the original planes seems to be missing.';
  l2= '      Subplot numbers now refer to the existing ones.';
  warning([l1 cr l2]);
end

if ~prod(ismember(h_real, h_stored)) % extra planes?!
  warning('There seems to be new planes. Subplot numbers refer to the old ones.');
end

h_stored=h_stored(index);          % existing original plots in original order

if ischar(p)                       % check if p is 'all'
  switch(p)
  case 'all'                                   
    p=1:size(h_stored,1);          % all original subplots
  otherwise
    msg= 'String value for subplot number vector has to be ''all''!';
    return;
  end
end

if ~isvector(p)                    % check the size
  msg= 'Subplot numbers (argument p in help text) have to be in a vector!';
  return
end

if min(p) < 1                      % check for invalid values
  msg= 'Subplot numbers (argument p in help text) must be at least 1!';
  return
end

%% p is too large

if max(p) > size(h_stored,1)
  l1= 'There are not so many existing subplots created by SOM_SHOW in the';
  l2= '      figure as you are trying to refer with subplot numbers.';
  l3= '      This is probably caused by having a too large subplot number.';
  l4= '      However, the reason may be invalid manipulation of';
  l5= '      this figure object or a program failure, too.';
  msg=([l1 cr l2 cr l3 cr cr l4 cr l5]);
  return;
end

%% Action and building output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

handles=h_stored(p);
comps=comps(p);

%% Subfunctions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t=isvector(v)
% ISVECTOR checks if a matrix is a vector or not
%
%  t=isvector(v)
% 
%  ARGUMENTS
%
%  v (matrix) 
%
%  OUTPUT 
%
%  t (logical)
%
%
%  Performs operation ndims(v) == 2 & min(size(v)) ==1
%

t=(ndims(v) == 2 & min(size(v)) == 1);

















