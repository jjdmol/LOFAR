function somui_loadmap(action, figH, type)
%SOMUI_LOADMAP displays a Load Window for somui_it's map.
%
%  somui_load([action], [figh])
%
% ARGUMENTS
%
%   [action]  (string)  internal variable
%   [figH]    (handle)  calling figure's handle
%   [type]    (integer) default for loading map/data/SOMfig struct
%
% SOMUI_LOADMAP  internally returns map to somui_it.
%
% See also SOM_READ_COD, SOM_READ_DATA, SOMUI_SEL_VAR, SOMUI_SEL_FILE,
%          SOMUI_SAVE

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments

error(nargchk(0, 3, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

if (nargin < 2)
  figH = gcbf;			% figH is calling figure, == somui_it
end;
if (nargin < 1)
  action = 'create';		% default action is 'create'
end; 
action = lower(action);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization                           

FIGURENAME = 'SOM Toolbox -- Load Map';
FIGURETAG  = 'somui_loadmap';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTIONs to be taken
%   'local_loadfile'
%   'local_loadvar'
%   'local_ok'
%   'local_create'

if (strcmp(action,'load_file'))
  local_lf;

elseif (strcmp(action,'load_var'))
  local_lv;

elseif (strcmp(action,'ok'))		% LOAD!
  local_ok;

elseif (strcmp(action,'create'))
  local_create(figH);

else
  if (~exist('action'))
    action = '<None>';
  end;
  fprintf(1,'%s : Unknown argument ''%s''.\n', mfilename, action);
				% mfilename is a Matlab function 
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%  end of main  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CREATE

function local_create(h_in);

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;

origUnits = get(0,'Units');
set(0,'Units','normalized');

h_caller 	= h_in;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

% load2 points to a part of somui_vis or somui_it loading instructions        
%   somui_it should exist!!!

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'DeleteFcn','somui_it(''load2'')', ...
	'NumberTitle','off', ...
	'WindowStyle', 'modal', ...
	'Position',[0.2 0.3 0.35 0.12], ...
	'Tag',FIGURETAG);
h = a;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Frames:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.02 0.71 0.96], ...
	'Style','frame', ...
	'Tag','fr_main');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.05 0.69 0.9], ...
	'Style','frame', ...
	'Tag','fr_from');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% From Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.7 0.66 0.22], ...
	'String','From', ...
	'Style','text', ...
	'Tag','st_from_title');

h_rb_from_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Workspace', ...
	'Position', [0.03 0.38 0.25 0.25], ...
	'Tag','rb_from_r1', ...
	'Value', 1);

h_rb_from_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','ASCII file', ...
	'Position', [0.03 0.1 0.25 0.25], ...
	'Tag','rb_from_r2', ...
	'Value', 0);

set(h_rb_from_r1,'UserData',[h_rb_from_r2]);
set(h_rb_from_r2,'UserData',[h_rb_from_r1]);
set([h_rb_from_r1 h_rb_from_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_from_r1'')),',...
         'set(findobj(''tag'',''et_from_edit1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_from_browse1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''et_from_edit2''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_from_browse2''),''Enable'',''off''),',...
      'else,',...
         'set(findobj(''tag'',''et_from_edit1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_from_browse1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''et_from_edit2''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_from_browse2''),''Enable'',''on''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
      'set(gco,''Value'',1),',...
    'end']);


b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_loadmap(''ok'')', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.38 0.26 0.25], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_from_edit1');
h_et_from_edit1 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_loadmap(''ok'')', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.1 0.26 0.25], ...
	'String','', ...
	'Style','edit', ...
	'Enable','off',...
	'Tag','et_from_edit2');
h_et_from_edit2 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_loadmap(''load_var'');', ... 
	'Position',[0.56 0.38 0.13 0.25], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Tag','pb_from_browse1');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_loadmap(''load_file'');', ... 
	'Position',[0.56 0.1 0.13 0.25], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Enable','off',...
	'Tag','pb_from_browse2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Push Buttons
%
%   LOAD, CANCEL

% FUTURE: maybe load and cancel should change place
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_loadmap(''ok'');', ...
	'Position',[0.74 0.1 0.23 0.35], ...
	'String','Load', ...
	'Tag','pb_load');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.74 0.5 0.23 0.35], ...
	'String','Cancel', ...
	'Tag','pb_cancel');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%  end of local_create  %%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_LV
%    User has clicked BROWSE-button in order to choose a
%    variable from Matlab's workspace

function local_lv

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_from_edit1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

var = somui_sel_var('SOM Select');
if (ischar(var))
  set(h_et_from_edit1,'String',var);
elseif (var == -1)
  fprintf(1, 'Cancel clicked!\n');
elseif (var == -2)
  fprintf(1, 'There is nothing to choose!\n');		% REMOVE?
else
  % Never flows here?
  fprintf(1, 'Variable selection did not work\n');	% REMOVE?
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%% end of local_lv %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_LF
%    User has clicked BROWSE-button in order to choose a
%    file from disk

function local_lf

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_from_edit2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%   *.cod is for Codebook files
%   *.dat is for data files
%   *.mat is for Matlab files, but user should load them from workspace
%
% Matlab's *nasty* feature: you can't scroll in a edit window! :-(

filter = '*.cod';

[filename, path] = somui_sel_file(filter,'SOM Select',300,200);
if (ischar(filename))
  tmp = strcat(path,filename);
  set(h_et_from_edit2,'String',tmp);
elseif (filename==0 | path==0)
  fprintf(1, 'Cancel clicked!\n');
else
  % Never flows here, because uigetfile returns [0 0] if
  % user presses 'Cancel' or makes an error
  fprintf(1, 'File selection did not work\n');  
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%% end of local_lf %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_OK
%    User clicks the LOAD button
% 

function local_ok

global FIGURENAME;
global FIGURETAG;

global h_caller;
global op;			% default value 0 given in local_create

global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

if (get(h_rb_from_r1,'Value') == 1)
  % Load from workspace
  var = get(h_et_from_edit1,'String');		% map, data, somfig
  filename = '';
else
  % Load from file
  filename = get(h_et_from_edit2,'String');	% filename
  var = '';
end;

if (isempty(var) & isempty(filename))
  fprintf(1,'Type a variable or file name.\n');
  return;
end;

% Map struct
  if (isempty(var))
    % read map from file
    map = som_read_cod(filename);
    if (~exist('map'))
      fprintf(1,'File reading %s did not succeed\n', filename);
      return;
    end;
  else
    % read map from variable
    % first check if variable exists
    found = local_check_var(var);
    if (found == 1)
      map = evalin('base',var);
    else
      fprintf(1,'Variable %s does not exist\n', var);
      return;
    end;
    if (~isfield(map,'codebook'))
      fprintf(1,'Variable %s is not a map structure\n', var);
      return;
    end;
  end;
  tmp = get(h_caller,'UserData');
  tmp.map = map;
  tmp.op = 1;		% return value, op == 1 means map loading operation
%FUTURE:
%  tmp.last.mapfile = ... 
%  Copy the name for the next Load-window
  set(h_caller,'UserData',tmp);
%  fprintf(1,'Successful map reading\n');


delete(gcf);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CHECK_VAR

function f_out = local_check_var(varin)

tmpvars = evalin('base','who');
f_out = 0;
for i = 1:length(tmpvars)
  if (strcmp(tmpvars{i},varin))
    f_out = 1;
    i = length(tmpvars);
  end;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



