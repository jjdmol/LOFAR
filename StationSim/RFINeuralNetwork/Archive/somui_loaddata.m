function somui_loaddata(action, figH)
%SOMUI_LOADDATA displays a Load Window for somui_it's data.
%
%  somui_loaddata([action],[figh])
%
% ARGUMENTS
%
%   [action]  (string)  action to be taken
%   [figH]    (handle)  figure's handle
%
% SOMUI_LOADDATA internally returns map, data or SOMfig struct to 
% caller's UserData
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
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments


% GUI cannot return any values but of the first round, 
%   for example, figure's handle. Return values must be
%   done by internal structures
%
% ---> There's a function WAITFOR which enables return values
%   for GUI

error(nargchk(0, 2, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

if (nargin < 2)
  figH = gcbf;			% figH is calling figure
end;
if (nargin < 1)
  action = 'create';		% default action is 'create'
end; 
action = lower(action);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization                           

FIGURENAME = 'SOM Toolbox -- Load Data';
FIGURETAG  = 'somui_loaddata';

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
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;

origUnits = get(0,'Units');
set(0,'Units','normalized');

h_caller 	= h_in;
op 		= 0;		% setting 'error' value. Will be 
				% changed in local_ok

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

% load2 points to a part of somui_vis or somui_it loading instructions        

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'DeleteFcn','somui_it(''load2'')', ...
	'NumberTitle','off', ...
	'WindowStyle', 'modal', ...
	'Position',[0.2 0.3 0.35 0.22], ...
	'Tag',FIGURETAG);
h = a;

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
	'Position',[0.02 0.57 0.69 0.4], ...
	'Style','frame', ...
	'Tag','fr_from');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.03 0.69 0.52], ...
	'Style','frame', ...
	'Tag','fr_params');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% From Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'FontWeight','bold', ... 
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.86 0.67 0.1], ...
	'String','From', ...
	'Style','text', ...
	'Tag','st_from_title');

h_rb_from_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Workspace', ...
	'Position', [0.03 0.73 0.25 0.12], ...
	'Tag','rb_from_r1', ...
	'Value', 1);

h_rb_from_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','ASCII file', ...
	'Position', [0.03 0.6 0.25 0.12], ...
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

% FUTURE (somui_xor):
%set([h_rb_from_r1 h_rb_from_r2],'Callback',...
%  'somui_xor(gco,''rb_from_r1'',''et_from_edit1'',...
%  ''pb_from_browse1'',''et_from_edit2'',''pb_from_browse2'')');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_loaddata(''ok'')', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.73 0.26 0.12], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_from_edit1');
h_et_from_edit1 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'Callback','somui_loaddata(''ok'')', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.29 0.6 0.26 0.12], ...
	'String','', ...
	'Style','edit', ...
	'Enable','off', ...
	'Tag','et_from_edit2');
h_et_from_edit2 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_loaddata(''load_var'');', ... 
	'Position',[0.56 0.73 0.13 0.12], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Tag','pb_from_browse1');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_loaddata(''load_file'');', ... 
	'Position',[0.56 0.6 0.13 0.12], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Enable','off', ...
	'Tag','pb_from_browse2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parameters Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.44 0.67 0.1], ...
	'String','Data reading parameters for ASCII files', ...
	'Style','text', ...
	'Tag','st_params_title');

h_rb_params_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Status line', ...
	'Position', [0.03 0.31 0.32 0.12], ...
	'Tag','rb_params_r1', ...
	'Value', 1);

h_rb_params_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Data Dimension:', ...
	'Position', [0.03 0.18 0.32 0.12], ...
	'Tag','rb_params_r2', ...
	'Value', 0);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Enable','off', ...
	'HorizontalAlignment', 'left', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.18 0.1 0.12], ...
	'Style','edit', ...
	'Tag','et_params_edit2');
h_et_params_edit2 = b;

% Radio buttons and en/disabling the edit box of Data dimension:
%   The inner in-clause is for en/disabling, the rest for radio buttons
set(h_rb_params_r1,'UserData',[h_rb_params_r2]);
set(h_rb_params_r2,'UserData',[h_rb_params_r1]);
set([h_rb_params_r1 h_rb_params_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_params_r2'')),',...
         'set(findobj(''tag'',''et_params_edit2''),''Enable'',''on''),',...
      'else,',...
         'set(findobj(''tag'',''et_params_edit2''),''Enable'',''off''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
      'set(gco,''Value'',1),',...
    'end']);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.03 0.05 0.32 0.12], ...
	'String','Don''t care char:', ...
	'Style','checkbox', ...
	'Tag','h_cb_params_care');
h_cb_params_care = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Enable','off', ...
	'HorizontalAlignment', 'left', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.05 0.1 0.12], ...
	'String','x', ...
	'Style','edit', ...
	'Tag','h_et_params_care');
h_et_params_care = b;

% If 'Data dimension' is chosen, then enable the edit box for
%   typing the dimension. If not chosen, disable it.
set(h_cb_params_care,'UserData',h_et_params_care);
set(h_cb_params_care,'Callback',...
    ['if (get(gco,''Value'')==1),', ...
      'set(get(gco,''UserData''),''Enable'',''on''),', ...
      'else, set(get(gco,''UserData''),''Enable'',''off''), end']);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Push Buttons
%
%   LOAD, HELP, CANCEL

% FUTURE: maybe load and cancel should change place
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_loaddata(''ok'');', ...
	'Position',[0.74 0.06 0.23 0.2], ...
	'String','Load', ...
	'Tag','pb_load');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.74 0.35 0.23 0.2], ...
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
%  fprintf(1, 'Cancel clicked!\n');
elseif (var == -2)
%  fprintf(1, 'There is nothing to choose!\n');	
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

global h_et_from_edit2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   *.cod is for Codebook files
%   *.dat is for data files
%   *.mat is for Matlab files, but user should load them from workspace
%
% Matlab's *nasty* feature: you can't scroll in a edit window! :-(


filter = '*.dat';

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

global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;


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


% Data struct					
tmp = get(h_caller,'UserData');
tmp.op = -1;
set(h_caller,'UserData',tmp);

if (isempty(var))
  % read data from file
  % arg1 -- dimension, either from user or then default 'dim_in_data',
  %         in that case, SOMTbx finds out the dim
  % h_rb_params_r1 -- Status line
  if (get(h_rb_params_r1,'Value') == 1)
    % Status line in the SOM_PAK file
    %%%remove    arg1 = 'dim_in_data';
    arg1 = '';    % call som_read_data(filename,[missing])
  else 					
    % User gives the data dimension, that is in arg1
    tmp_edit2 = get(h_et_params_edit2,'String');
    if (~isempty(tmp_edit2))
      if (strcmp(tmp_edit2,'guess'))
        arg1 = 'dim_in_data';           % ask SOMTbx to guess the dim
      else
        arg1 = str2num(tmp_edit2);	% arg1 == <user given dimension>
      end;
    else
      fprintf(1,'Give the data dim or read with status line\n');
      return;
    end;
  end;

  if (get(h_cb_params_care,'Value') == 1)
    % User gives a mark for holes in the data
    if (isempty(get(h_et_params_care,'String')))
      arg2 = 'x';
    else
      arg2 = get(h_et_params_care,'String');
    end;
  else 
    arg2 = '';
  end;

  % disable GUI for the time of disk reading
  h_tmp = findobj(gcf);
  h_tmp = h_tmp(2:end);		% remove the figure itself
  watchon;
  set(h_tmp,'Enable','off');

  drawnow;

  % Call SOM_READ_DATA
  %   SOM_READ_DATA Reads a data file.
  %
  %     sData = som_read_data(filename, dim, [missing])
  %     sData = som_read_data(filename, [missing])
  %
  % where dim--arg1 missing--arg2

  if (isempty(arg2))
    if (isempty(arg1))
      data = som_read_data(filename);
    else
      data = som_read_data(filename, arg1);
    end;
  else
    if (isempty(arg1))
      data = som_read_data(filename, arg2);
    else
      data = som_read_data(filename, arg1, arg2);
    end;
  end;
  % makes an error in som_read_dat if fails

  % enable GUI
  set(h_tmp,'Enable','on');
  watchoff;

  drawnow;

else
  % reads data struct from variable
  found = local_check_var(var);
  if (found == 1)
    data = evalin('base',var);
  else
    fprintf(1,'Variable %s does not exist\n', var);
    disp(sprintf('\a')); 		% BEEP Beep-signal
    return;
  end;
  if (~isfield(data,'data'))
    % Call SOM_DATA_STRUCT to create SOM data struct
    % First argument copies data to data struct and second
    % gives 'var' for data struct's name
    data = som_data_struct(data, var);
  end;
end;
tmp.data = data;
tmp.op = 2;		% return value, op == 2 means data loading operation
set(h_caller,'UserData',tmp);
% fprintf(1,'Successful data reading\n');

delete(findobj('tag',FIGURETAG));

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



