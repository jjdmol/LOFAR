function somui_savemap(action, arg1)
%SOMUI_SAVEMAP saves map structure
%
%  somui_savemap(action, arg1)
%
% ARGUMENTS
%
%  action  (string)  internal variable
%  arg1	   (struct) map-structure
%
% SOMUI_SAVEMAP(ACTION, ARG1) saves map structure either into a SOM_PAK 
% file or variable (workspace)
%
% Variable passing (it) through 'UserData' of IT.
%
% See also SOMUI_SAVE

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;

global mapstring;
global map;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments

error(nargchk(0, 2, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

if (nargin < 1)
  action = 'create';
end;
action = lower(action);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialization                           

FIGURENAME = 'SOM Toolbox -- Save Map';
FIGURETAG  = 'somui_savemap';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% ACTIONs:

if (strcmp(action,'browse_file'))
  local_sfile;

elseif (strcmp(action,'browse_var'))
  local_svar;

elseif (strcmp(action,'save'))
  local_ok;

elseif (strcmp(action,'create'))
  if (nargin < 2)
    map = 'NO MAP GIVEN';
    fprintf(1,'No map given to ''somui_savemap''\n');
    return;
  else
    map        = arg1;
    mapstring  = local_getmap2lb(map);
  end;
  local_create;

else
  if (~exist('action'))
    action = '<None>';
  end;
  fprintf(1,'%s : Unknown argument ''%s''.\n', mfilename, action);
				% mfilename is a Matlab function 

end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)functions LOCAL_CREATE

function local_create

global FIGURENAME;
global FIGURETAG;

global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;

global mapstring;

origUnits = get(0,'Units');
set(0,'Units','normalized');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

a = figure('Color',[0.8 0.8 0.8], ...
	'Units','normalized', ...
	'MenuBar','none', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'Position',[0.04 0.74 0.35 0.22], ...
	'Tag',FIGURETAG, ...
	'WindowStyle','modal');
ld.h = a;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Frames:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.01 0.71 0.98], ...
	'Style','frame', ...
	'Tag','fr_main');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.43 0.69 0.54], ...
	'Style','frame', ...
	'Tag','fr_params');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.03 0.69 0.39], ...
	'Style','frame', ...
	'Tag','fr_browses');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Buttons

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.74 0.27 0.23 0.17], ...
	'String','Cancel', ...
	'Tag','pb_cancel');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_savemap(''save'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.74 0.05 0.23 0.17], ...
	'String','Save', ...
	'Tag','pb_ok');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'HorizontalAlignment','left', ...
	'FontWeight','bold', ...
	'Position',[0.03 0.85 0.67 0.1], ...
	'String','Map structure:', ...
	'Style','text', ...
	'Tag','st_savedata');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'FontName','Courier', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.45 0.67 0.39], ...
	'String',mapstring, ...
	'Style','listbox', ...
	'Tag','st_mapstruct');
ld.h_lb_sm = b;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'FontWeight','bold', ... 
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.3 0.67 0.1], ...
	'String','To:', ...
	'Style','text', ...
	'Tag','st_to_title');

h_rb_to_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Workspace', ...
	'Position', [0.03 0.17 0.25 0.12], ...
	'Tag','rb_to_r1', ...
	'Value', 1);

h_rb_to_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','SOM_PAK file', ...
	'Position', [0.03 0.04 0.25 0.12], ...
	'Tag','rb_to_r2', ...
	'Value', 0);

set(h_rb_to_r1,'UserData',[h_rb_to_r2]);
set(h_rb_to_r2,'UserData',[h_rb_to_r1]);
set([h_rb_to_r1 h_rb_to_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_to_r1'')),',...
         'set(findobj(''tag'',''et_to_edit1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_to_browse1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''et_to_edit2''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_to_browse2''),''Enable'',''off''),',...
      'else,',...
         'set(findobj(''tag'',''et_to_edit1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_to_browse1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''et_to_edit2''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_to_browse2''),''Enable'',''on''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
       'set(gco,''Value'',1),',...
    'end']);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'Callback', 'somui_savemap(''save''),', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.18 0.25 0.12], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_to_edit1');
h_et_to_edit1 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'Callback','somui_savemap(''save'')', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.05 0.25 0.12], ...
	'String','', ...
	'Style','edit', ...
	'Enable','off', ...
	'Tag','et_to_edit2');
h_et_to_edit2 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_savemap(''browse_var'');', ... 
	'Position',[0.56 0.18 0.13 0.12], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Tag','pb_to_browse1');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_savemap(''browse_file'');', ... 
	'Position',[0.56 0.05 0.13 0.12], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Enable','off', ...
	'Tag','pb_to_browse2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%  end of LOCAL_CREATE  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function local_sfile
%    Fetch a file where to save. SOM_PAK map files end with .cod

function local_sfile

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_to_edit2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

filter = '*.cod';

[filename,path] = uiputfile(filter,'Browse a File',150,300);
if (ischar(filename))
  tmp = strcat(path,filename);
  set(h_et_to_edit2,'String',tmp);
elseif (filename==0 | path==0)
  fprintf(1, 'Cancel clicked!\n');
else
  % Never flows here, because uigetfile returns [0 0] if
  % user presses 'Cancel' or makes an error
  fprintf(1, 'File selection did not work\n');  
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%  end of LOCAL_SFILE  %%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function local_svar

function local_svar

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_to_edit1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

var = somui_sel_var('SOM Select');
if (ischar(var))
  set(h_et_to_edit1,'String',var);
elseif (var == -1)
  fprintf(1, 'Cancel clicked!\n');
elseif (var == -2)
  fprintf(1, 'There is nothing to choose!\n');		% REMOVE?
else
  % Never flows here?
  fprintf(1, 'Variable selection did not work\n');	% REMOVE?
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%% end of local_svar %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_OK
%    User clicks the SAVE button
% 

function local_ok

global FIGURENAME;
global FIGURETAG;

global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;

global map;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

if (get(h_rb_to_r1,'Value') == 1)
  % Save to workspace
  var = get(h_et_to_edit1,'String');		% map, data, somfig
  filename = '';
else
  % Save to file
  % FUTURE: Warn, if user is trying to save over an existing file.
  filename = get(h_et_to_edit2,'String');	% filename
  var = '';
end;

if (isempty(var) & isempty(filename))
  fprintf(1,'Type a variable or file name.\n');
  return;
end;

% Check if the argument is a map structure!
if (~isfield(map,'msize'))
  fprintf(1,'Argument not a map struct\n');
  return;
end;    

if (isempty(var))
  % save map to file
  watchon;
  drawnow;
  som_write_cod(map, filename);
  watchoff;
  fprintf(1,'%s: Map saved in %s\n',mfilename, filename);
else
  % save map to the workspace as a variable
  assignin('base',var,map);
  fprintf(1,'%s: Map saved in %s\n',mfilename, var);
end;

delete(gcf);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function local_getmap2lb
%   Reads a map structure and prints it to ready 
%   for listbox. arg == map structure
%   Font in listbox is changed to Courier
%
% See also SOM_INFO

function mapstring = local_getmap2lb(arg)

mdim = length(arg.msize);
dim  = size(arg.codebook, ndims(arg.codebook));
l    = length(arg.train_sequence);

mapstring = cell(12+mdim,1);

mapstring{1}= strcat('Map name .........: ', arg.name);
mapstring{2}= strcat(' Data name .......: ', arg.data_name);
mapstring{3}= strcat('Shape ............: ', arg.shape);
mapstring{4}= strcat(' Lattice .........: ', arg.lattice);
mapstring{5}= strcat(' Neighborhood ....: ', arg.neigh);
mapstring{6}= strcat('Map grid dim .....: ', num2str(mdim));

tmp = num2str(arg.msize(1));
for i = 2:mdim
  tmp = strcat(tmp,'x', num2str(arg.msize(i)));
end
mapstring{7}= strcat(' Map grid size:...: ', tmp);
mapstring{8}= strcat('Codebook dim .....: ', num2str(dim));

for i = 1:dim,
  mapstring{8+i} = strcat(' #',num2str(i),' name .........: ', arg.comp_names{i});
end

if strcmp(arg.init_type, 'unknown')
  mapstring{9+dim}= 'The map has been produced using SOM_PAK program package.';
else
  mapstring{9+dim}= strcat('Init type ........: ', arg.init_type);
  mapstring{10+dim}= strcat('Training type ....: ', arg.train_type);

  if l > 0
    mapstring{11+dim}= strcat('# map trained ....: ', num2str(l));
    mapstring{12+dim}= strcat(' Last training ...: ', arg.train_sequence{l}.time);
  else
    mapstring{11+dim}= 'The map has been initialized,';
    mapstring{12+dim}= ' but not trained.';
  end
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%% end of local_getmap2lb  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%