function varargout = interface1(varargin)
% INTERFACE1 Application M-file for interface1.fig
%    FIG = INTERFACE1 launch interface1 GUI.
%    INTERFACE1('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 25-Jul-2002 14:45:08

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');
    
	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
	guidata(fig, handles);

	if nargout > 0
		varargout{1} = fig;
	end

elseif ischar(varargin{1}) % INVOKE NAMED SUBFUNCTION OR CALLBACK

	try
		if (nargout)
			[varargout{1:nargout}] = feval(varargin{:}); % FEVAL switchyard
		else
			feval(varargin{:}); % FEVAL switchyard
		end
	catch
		disp(lasterr);
	end

end


%| ABOUT CALLBACKS:
%| GUIDE automatically appends subfunction prototypes to this file, and 
%| sets objects' callback properties to call them through the FEVAL 
%| switchyard above. This comment describes that mechanism.
%|
%| Each callback subfunction declaration has the following form:
%| <SUBFUNCTION_NAME>(H, EVENTDATA, HANDLES, VARARGIN)
%|
%| The subfunction name is composed using the object's Tag and the 
%| callback type separated by '_', e.g. 'slider2_Callback',
%| 'figure1_CloseRequestFcn', 'axis1_ButtondownFcn'.
%|
%| H is the callback object's handle (obtained using GCBO).
%|
%| EVENTDATA is empty, but reserved for future use.
%|
%| HANDLES is a structure containing handles of components in GUI using
%| tags as fieldnames, e.g. handles.figure1, handles.slider2. This
%| structure is created at GUI startup using GUIHANDLES and stored in
%| the figure's application data using GUIDATA. A copy of the structure
%| is passed to each callback.  You can store additional information in
%| this structure at GUI startup, and you can change the structure
%| during callbacks.  Call guidata(h, handles) after changing your
%| copy to replace the stored original so that subsequent callbacks see
%| the updates. Type "help guihandles" and "help guidata" for more
%| information.
%|
%| VARARGIN contains any extra arguments you have passed to the
%| callback. Specify the extra arguments by editing the callback
%| property in the inspector. By default, GUIDE sets the property to:
%| <MFILENAME>('<SUBFUNCTION_NAME>', gcbo, [], guidata(gcbo))
%| Add any extra arguments after the last argument, before the final
%| closing parenthesis.




% --------------------------------------------------------------------
function varargout = listbox1_Callback(h, eventdata, handles, varargin)
global R;
set(handles.listbox1,'String',R);


% --------------------------------------------------------------------
function varargout = edit2_Callback(h, eventdata, handles, varargin)
global echantillonnage; 
echantillon=get(handles.edit2,'string');
echantillonnage=str2num(echantillon);
%disp('voila ' echantillonnage '')

% --------------------------------------------------------------------
function varargout = edit3_Callback(h, eventdata, handles, varargin)
global arg_acpplan1; 
arg_acpplan1=get(handles.edit3,'string');
arg_acpplan1=str2num(arg_acpplan1);
%disp('voila ' echantillonnage '')

% --------------------------------------------------------------------
function varargout = edit4_Callback(h, eventdata, handles, varargin)
global arg_acpplan2 ; 
arg_acpplan2=get(handles.edit4,'string');
arg_acpplan2=str2num(arg_acpplan2);
%disp('voila ' echantillonnage '')

% --------------------------------------------------------------------
function varargout = pushbutton3_Callback(h, eventdata, handles, varargin)

% --------------------------------------------------------------------
function varargout = pushbutton5_Callback(h, eventdata, handles, varargin)

% --------------------------------------------------------------------
function varargout = pushbutton7_Callback(h, eventdata, handles, varargin)

% --------------------------------------------------------------------
function varargout = edit5_Callback(h, eventdata, handles, varargin)
global n_choice;
n_choice=get(handles.edit5,'string');
n_choice=str2num(n_choice);


% --------------------------------------------------------------------
function varargout = pushbutton9_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit6_Callback(h, eventdata, handles, varargin)
global feature;
feature=str2num(get(handles.edit6,'string'));




% --------------------------------------------------------------------
function varargout = pushbutton10_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit7_Callback(h, eventdata, handles, varargin)
global Classe_stat;
Classe_stat=str2num(get(handles.edit7,'string'));


