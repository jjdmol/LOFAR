function h = som_demo(action)
%SOM_DEMO is a demo window for SOM Toolbox
%
%  som_demo(action) 
%
% ARGUMENTS
%
%  action  (char) internal variable
% 
% SOM_DEMO(ACTION) is a demo window for SOM Toolbox for Matlab 5.
% There are 4 demos so far, which can also be executed from command
% line. Type 'som_demoX', where X={1, 2, 3, 4}, to see demos.
%
% See also SOM_DEMO1, SOM_DEMO2, SOM_DEMO3, SOM_DEMO4

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check arguments

if (nargin < 1)
  action = '';
else
  action = lower(action);
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialize 

% Initialize some constant and variable names for DEMO window

FIGURENAME = 'SOM Toolbox -- DEMO';
FIGURETAG  = 'som_demo';
FIGURESIZE = [0.02 0.52 0.5 0.4];

choosedemo = ['Choose a Demo'; ...
              'Demo 1       '; ...
              'Demo 2       '; ...
              'Demo 3       '; ...
              'Demo 4       '];

% Get the handle to the DEMO figure.

set(0, 'ShowHiddenHandles','on');
h_figH = findobj('Type','figure');
h_figH = findobj(h_figH,'flat','Name',FIGURENAME,'Tag',FIGURETAG);
set(0, 'ShowHiddenHandles','off');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This SOM Demo should have only one window. Check if there 
%   exists a demo window. If does, get it top,
%   otherwise either do as 'action' says. If there is no
%   'FIGURENAME' window, create one.

if ~isempty(h_figH)
  if (strcmp(action,''))                   
    figure(h_figH);
% Remove next line?
    fprintf(1,'''%s'' figure (%d) is now active\n',FIGURENAME,h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% POPUPMENU clicked!

  elseif (strcmp(action,'demo_chosen'))     
    ud = get(h_figH,'UserData');
    ud.demovalue  = get(ud.h_pop_btn,'Value');
    switch ud.demovalue
      case 1
        fprintf(1,'\n\nSOM Toolbox for Matlab 5\n');
        set(ud.h_cont_btn, 'Enable', 'off');
        set(ud.h_close_btn, 'String', 'Close');
        return;
      case 2
        fprintf(1,'\n\nSOM Toolbox for Matlab 5\nSOM Demo 1\n');
        set(ud.h_cont_btn, 'Enable', 'on', 'String', 'Begin');
      case 3
        fprintf(1,'\n\nSOM Toolbox for Matlab 5\nSOM Demo 2\n');
        set(ud.h_cont_btn, 'Enable', 'on', 'String', 'Begin');
      case 4
        fprintf(1,'\n\nSOM Toolbox for Matlab 5\nSOM Demo 3\n');
        set(ud.h_cont_btn, 'Enable', 'on', 'String', 'Begin');
      case 5
        fprintf(1,'\n\nSOM Toolbox for Matlab 5\nSOM Demo 4\n');
        set(ud.h_cont_btn, 'Enable', 'on', 'String', 'Begin');
    end 
    set(h_figH,'UserData',ud);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CONTINUE clicked!

  elseif (strcmp(action,'continue'))      
    ud = get(h_figH,'UserData');

    if (strcmp(get(ud.h_cont_btn, 'String'), 'Begin'))     % First time
      ud.h_active = figure('units', 'normalized', ...
                           'position',[0.53 0.52 0.4 0.4]);
      set(ud.h_cont_btn, 'String', 'Continue');
      set(ud.h_close_btn, 'String', 'Stop');
      switch ud.demovalue
        case 1,
          return;
        case 2
          ud.fid = local_openfile('som_demo1.m');  
        case 3
          ud.fid = local_openfile('som_demo2.m');  
        case 4
          ud.fid = local_openfile('som_demo3.m');  
        case 5
          ud.fid = local_openfile('som_demo4.m');  
      end 
      set(gcf, 'UserData', ud);
    end;

    st = local_read_exec(ud.fid);
    if (st > 0)						% these lines
						% are only for GUI
      tmp = get(ud.h_pop_btn, 'String');
      tmp = tmp(get(ud.h_pop_btn, 'Value'), :);
      set(ud.h_demoname, 'String', tmp);		% move 3 lines above
      set(ud.h_textbox, 'Value', []);
      set(ud.h_pop_btn, 'Enable', 'off');		% move
      set(gcf, 'UserData', ud);
    else %end of file
      set(ud.h_cont_btn, 'Enable', 'off');
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CLOSE / STOP clicked!

  elseif (strcmp(action,'close_stop'))      
    ud = get(h_figH,'UserData');
    if (strcmp(get(ud.h_close_btn,'String'),'Close'))
      delete(gcf);
      % delete all other figures, too? - Too dangerous.
      return;
    end;
    if (strcmp(get(ud.h_close_btn,'String'),'Stop'))
      set(ud.h_pop_btn, 'Enable', 'on', 'Value', 1);
      set(ud.h_textbox, 'String', '', 'Value', []);
      set(ud.h_cont_btn, 'Enable', 'off');
      set(ud.h_demoname, 'String', 'SOM Demos');
      set(ud.h_close_btn, 'String', 'Close');
      local_closefile(ud.fid);
      clear ud.fid;				% try to remove fid
      set(gcf,'UserData', ud);
      delete(findobj(ud.h_figH,'type','axes')); % remove possible mistakes
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% COMMAND LINE som_demo with unknown argument

% mfilename is a Matlab function 

  else                                      
    fprintf(1,'%s : Unknown argument ''%s''.\n', ... 
              mfilename,action);

  end  %% end of 'action' section

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% No arguments and no DEMO window exists.
%%    Create one DEMO UI.

else                                         
  origUnits = get(0,'Units');
  set(0,'Units','normalized');
  scrSize = get(0,'ScreenSize');
  ud.demovalue = 1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Uicontrols are done with help of 'guide'. See
%%   HELP GUIDE and 'guide' who it works. These controls
%%   have been edited.

% ATTENTION! This is only the init phase. Lots of other figures
% and so on can be emerged after initialization and they are not
% shown here. You must search them with 'get' and 'findobj' and
% modify with 'set'. 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'Position',FIGURESIZE, ...
	'Tag',FIGURETAG);
ud.h_figH = a;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Bottom Text Box

% Text Box down

% ennen tata ja aina kun popupmenussa on Choose a Demo,
% niin naytetaan jotain SOM-logoa tai kuvaa

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'HorizontalAlignment', 'left', ...
	'Position',[0.01 0.82 0.76 0.17], ...
	'String', ...
{'SOM Toolbox for Matlab 5 created by Laboratory of Information', ...
 'and Computer Science, 1997', ...
 '  See more details by typing ''help somtoolbox'' or', ...
 'at our web site http://www.cis.hut.fi/projects/somtoolbox.'},...
	'Style','text', ...
	'Tag','txt_TitleText');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Enable','on', ...
	'FontName','Courier', ...
	'HorizontalAlignment', 'left', ...
	'ListboxTop', 1, ...
	'Max', 2, ...
	'Position',[0.01 0.01 0.76 0.8], ...
	'String', '', ...
	'Style','listbox', ...
	'Tag','lb_DemoTextBox', ...
	'Value', []);
ud.h_textbox = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Right side: Frame and text

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.78 0.01 0.21 0.98], ...
	'Style','frame', ...
	'Tag','fr_RightSide');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.79 0.91 0.19 0.07], ...
	'String','SOM Demos', ...
	'Style','text', ...
	'Tag','txt_fr_RightSide');
ud.h_demoname = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Pop-up Menu

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','som_demo(''demo_chosen'')', ...
	'Max',4, ...
	'Min',1, ...
	'Position',[0.79 0.78 0.19 0.12], ...
	'String',choosedemo, ...
	'Style','popupmenu', ...
	'Tag','pop_demo', ...
	'Value', 1);
ud.h_pop_btn = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Buttons:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback', [...
            'ud = get(gcf,''UserData'');', ...
            'if (ud.cb_alines == 1),', ...
              'ud.cb_alines = 0;', ...
            'else,', ...
              'ud.cb_alines = 1;', ...
            'end;', ...
            'set(gcf,''UserData'', ud);'], ...
	'Position',[0.79 0.5 0.19 0.1], ...
	'String','Show all text', ...
	'Style', 'checkbox', ...
	'Value', 1, ...
	'Tag','cb_alines');
ud.cb_alines = 1;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','som_demo(''continue'')', ...
	'Enable','off', ...
	'Position',[0.79 0.3 0.19 0.12], ...
	'Enable','off', ...
	'String','Continue', ...
	'Tag','pb_Continue');
ud.h_cont_btn = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','som_demo(''close_stop'')', ...
	'Position',[0.79 0.02 0.19 0.12], ...
	'String','Close', ...
	'Tag','pb_CloseStopBtn');
ud.h_close_btn = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Wait

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'ForeGroundColor',[1 0 0], ...
	'HorizontalAlignment', 'left', ...
	'Position',[0.82 0.18 0.13 0.08], ...
	'String', 'Wait!', ...
	'Style','text', ...
	'Visible', 'off', ...
	'Tag','txt_wait');
ud.h_wait = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UIMENUS:


b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Tools', ...
	'Tag','uim_tools');

c = uimenu('Parent',b, ...
	'Callback','somui_it', ...
	'Label','Init and Train...', ...
	'Tag','uim_tools_it');

c = uimenu('Parent',b, ...
	'Callback','somui_vis', ...
	'Label','Visualization...', ...
	'Tag','uim_tools_vis');

% if error occurs, buttons can be activated
c = uimenu('Parent',b, ...
	'Callback',[...
           'set(findobj(''tag'',''pb_cont_btn''),''Enable'',''on'');', ...
           'set(findobj(''tag'',''pb_close_btn''),''Enable'',''on'');'], ...
	'Separator', 'on', ...
	'Visible', 'off', ...
	'Label','Enable buttons', ...
	'Tag','uim_tools_rm_disables');

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&SOM Toolbox Help', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/'');', ...
	'Label','SOM Toolbox http://www.cis.hut.fi/projects/somtoolbox/', ...
	'Tag','uim_tools_demo');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/somintro/som.html'');', ...
	'Label','&Intro to the SOM from web site', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/docu.html'');', ...
	'Label','Intro to SOM &Toolbox from web site', ...
	'Tag','uim_help');



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: end of ui graphics

set(gcf,'UserData',ud);

end  %% end of som_demo()


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%  LOCAL FUNCTIONS


% init, when demo chosen
% open som_demoX file
% start to read line after line
% for each line

% create a cell for text
%  empty line --> empty cell line
%  comment line --> remove '%' and spaces and copy to cell line
%  function/assign line --> copy to cell line and evaluate
%  reserved
%  ignore clf-functions!

%   pause  --> ask for CONTINUE
%   echo off --> ignore, we can use line's -1 to stop
%   EOF --> Stop->Close, Continue->Choose, Logo
%   clf reset --> not copied to cell ud.h_active=figure
%   '%'  '%1234Text begin here', cell{i{ = line(6:end)
%   anything else, just copy it and evaluate!
%   clc / ignore


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_OPENFILE

function fid = local_openfile(demofile)
% Open demo file. Here we read exactly the same m-file we can
% execute from command line, that is, som_demo1, som_demo2, etc.

[fid, msg]  = fopen(demofile,'r');
if (fid == -1)
  error(msg);
  return;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CLOSEFILE

function local_closefile(fid)
% Close file

st  = fclose(fid);
% fprintf(1,'%s',st);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_READ_EXEC

function st = local_read_exec(fid)
% This subfunction prints text and executes commands from
% 'clc' to 'clc' in the file som_demoX.m

MAX_TEXTLINES = 400;  % 7Nov97: som_demo1: 160, 2: 240, 3: 296, 4: 162
MAX_EXECLINES = 200;

ud = get(gcf, 'UserData');	% is this always current figure?

% Read lines until EOF or pause-clc combination is met
% Collect printable lines to a cell and executable lines
% to another cell

textcell = cell(MAX_TEXTLINES,1);
execcell = cell(MAX_EXECLINES,1);
cont = 1;
st = 1; %ok, -1 is an error

k = 1;
m = 0;


figure(ud.h_active);
while (cont),   
  line = fgetl(fid);		% return -1 when EOF
%  disp(line);			% DEBUGginh
  if (line == -1)
    st = -1;
    cont = 0;
    endoffile = 1;
    break; 			% out from while loop
  else
    if (length(line) == 0)
      k = k+1;
    else
      if ischar(line) 
        % start to compare strings to reserved chars. Mark that strings
        % must be searched so that all possible short words are mentioned.
        %   %      -- comment
        %   clc    -- reset k
        %   clf    -- skip
        %   echo   -- skip
        %   pause  -- function stopping criteria
        %   figure -- create a new figure and save the handle in UserData
        %          -- read and execute line
        if (strcmp(line(1),'%'))
          textcell{k} = line(6:end);
          k = k+1;
        elseif (strcmp(line(1:3),'clc'))
          % There are normally couple of emply lines before 'clc'.
          % They would appear as empty lines also in GUI text box.
          % Remove them by assigning k=1;
          k = 1;
        elseif (strcmp(line(1:3),'clf'))
          %
        elseif (strcmp(line(1:4),'echo'))
          %
        elseif (strcmp(line(1:5),'pause'))
          cont = 0;				% RETURN!!!
          break;
        elseif (strcmp(line(1:7),'figure;'))   % or 1:6 'figure' ???
          m = m + 1;
          execcell{m} = [...
            'h=figure(''units'',''normalized'', ''position'',[0.53 0.50-0.2*rand(1) 0.4 0.4]);', ...
            'ud=get(findobj(''tag'',''som_demo''),''UserData'');', ...
            'ud.h_active = h;', ...
            'set(findobj(''tag'',''som_demo''),''UserData'', ud);'];
        else
          m = m + 1;
          execcell{m} = line;
          line = strcat('>> ',line);
          textcell{k} = line;
          k = k + 1;
        end;
      end;
    end; 
  end;
end;


if ((~ud.cb_alines) | (isempty(get(ud.h_textbox, 'String'))))
  % show only those lines in text box, which are relevant to this
  % exact part of demo
  fromcell = 1;
  outtextcell = cell(k,1);
else
  % show all lines in text box, that is, keep counter in memory
  outtextcell = get(ud.h_textbox, 'String');
  fromcell = size(outtextcell,1) + 1;
end;

j = 0;  % normal counter
for i=fromcell:(fromcell+k-1)
  j = j + 1;
  outtextcell{i} = textcell{j};
end;
set(ud.h_textbox, 'String', outtextcell, 'ListBoxTop', fromcell);
drawnow;

% Disable buttons
set(ud.h_cont_btn,'Enable', 'off');
set(ud.h_close_btn,'Enable', 'off');
set(ud.h_wait, 'Visible', 'on');
drawnow;

%Evaluate functions
for i=1:m
  disp(execcell{i});
  evalin('base', execcell{i});
end;

% Enable control
set(ud.h_cont_btn,'Enable', 'on');
set(ud.h_close_btn,'Enable', 'on');
set(ud.h_wait, 'Visible', 'off');
drawnow;

figure(ud.h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
