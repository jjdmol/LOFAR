function varargout = gui_FFT(varargin)
% GUI_FFT Application M-file for gui_FFT.fig
%    FIG = GUI_FFT launch gui_FFT GUI.
%    GUI_FFT('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 15-Apr-2003 17:44:46
global Nfft1_analyser;
global sig;
global gui_FFT;

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');

	% Use system color scheme for figure:
	set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
	guidata(fig, handles);
    load('data\signal_analyser.mat');
    set(handles.length_text,'string',length(sig));
    h1=get(handles.FFT_points1);
    Nfft_temp=str2num(h1.String);
    set(handles.Stop_FFT,'string',Nfft_temp);
    set(handles.checkbox_FFT,'Enable','off');
    set(handles.text5,'Enable','off');
    set(handles.FFT_points2,'Enable','off');
    
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
function varargout = edit2_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit3_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit4_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Apply_Button_Callback(h, eventdata, handles, varargin)
global start;
global stop;
global Nfft1_analyser;
global Nfft2_analyser;
global sig;

 
 stage=get(findobj('Tag','checkbox_FFT'),'value');
 Nfft1_analyser=str2num(get(findobj('Tag','FFT_points1'),'string'));
 Nfft2_analyser=str2num(get(findobj('Tag','FFT_points2'),'string'));
 start=str2num(get(findobj('Tag','Start_FFT'),'string'));
 stop=str2num(get(findobj('Tag','Stop_FFT'),'string'));
 FFT_subband_shifted2(Nfft1_analyser,Nfft2_analyser,start,stop,sig,stage);


% --------------------------------------------------------------------
function varargout = FFT_points2_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function varargout = checkbox_FFT_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function varargout = FFT_points1_Callback(h, eventdata, handles, varargin)
global Nfft1_analyser;
g = findobj('Tag','FFT_points1');
Nfft1_analyser=str2num(get(g,'String'));

g = findobj('Tag','Stop_FFT');
set(g, 'String',num2str(Nfft1_analyser));



% --------------------------------------------------------------------
function varargout = Start_FFT_Callback(h, eventdata, handles, varargin)

g = findobj('Tag','Start_FFT');
Nfft1_analyser=str2num(get(g,'String'));

g = findobj('Tag','Stop_FFT');
Nfft2_analyser=str2num(get(g,'String'));


if Nfft1_analyser==Nfft2_analyser;
set(findobj('Tag','checkbox_FFT'),'Enable','on');
set(findobj('Tag','text5'),'Enable','on');
set(findobj('Tag','FFT_points2'),'Enable','on');
else
set(findobj('Tag','checkbox_FFT'),'Enable','off');
set(findobj('Tag','text5'),'Enable','off');
set(findobj('Tag','FFT_points2'),'Enable','off');
end



% --------------------------------------------------------------------
function varargout = Stop_FFT_Callback(h, eventdata, handles, varargin)

g = findobj('Tag','Start_FFT');
Nfft1_analyser=str2num(get(g,'String'));

g = findobj('Tag','Stop_FFT');
Nfft2_analyser=str2num(get(g,'String'));


if Nfft1_analyser==Nfft2_analyser;
set(findobj('Tag','checkbox_FFT'),'Enable','on');
set(findobj('Tag','text5'),'Enable','on');
set(findobj('Tag','FFT_points2'),'Enable','on');
else
set(findobj('Tag','checkbox_FFT'),'Enable','off');
set(findobj('Tag','text5'),'Enable','off');
set(findobj('Tag','FFT_points2'),'Enable','off');
end

