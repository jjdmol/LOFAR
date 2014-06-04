function varargout = gui_qt(varargin)
% GUI_QT Application M-file for gui_qt.fig
%    FIG = GUI_QT launch gui_qt GUI.
%    GUI_QT('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 20-Sep-2002 12:18:33

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');

	% Use system color scheme for figure:
	set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

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




%********************************************************************%
%this function enables to load the test file on the prototyping board%
%********************************************************************%

% --------------------------------------------------------------------
function varargout = pushbutton1_Callback(h, eventdata, handles, varargin)

%Test file number
test_number = get(handles.test,'Value');

load_test_rs232(test_number);



%********************************************************************%
%this function enables to get the output of the test from the board  %
%********************************************************************%

% --------------------------------------------------------------------
function varargout = pushbutton2_Callback(h, eventdata, handles, varargin)

%Test file number
test_number = get(handles.test,'Value');
update=0;

%Read a .txt file out of the RAM of the FPGA via RS232
read_test_rs232(update,test_number);


%********************************************************************%
%this function updates the plot of the output                        %
%********************************************************************%

% --------------------------------------------------------------------
function varargout = pushbutton4_Callback(h, eventdata, handles, varargin)

%Test file number
test_number = get(handles.test,'Value');
update=1;

%Read a .txt file out of the RAM of the FPGA via RS232
read_test_rs232(update,test_number);



% --------------------------------------------------------------------
function varargout = popupmenu1_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = out_bin_Callback(h, eventdata, handles, varargin)













