function beamformerGUI(varargin)
% BEAMFORMERGUI Application M-file for beamformerGUI.fig
%    FIG = BEAMFORMERGUI launch beamformerGUI GUI.
%    BEAMFORMERGUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 18-Jul-2002 09:24:46

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
    
 % Wait for callbacks to run and window to be dismissed:
  uiwait(fig);

  % UIWAIT might have returned because the window was deleted using
  % the close box - in that case, return 'cancel' as the answer, and
  % don't bother deleting the window!
  if ~ishandle(fig)
	  answer = 'cancel';
  else
  	  % so, we need to delete the window.
      
      if (get(findobj(fig,'Tag','InnerProductRadio'),'Value'))
          BFstrat = 1;    
          if (get(findobj('Tag','MDLradio'),'Value'))
		% use MDL 
              rfi_value = str2num(get(findobj('Tag','MDLedit'),'String'));
              rfi_strat = 1; 
          elseif (get(findobj('Tag','ThresholdRadio'),'Value'))
		% Thresholding
              rfi_value = str2num(get(findobj('Tag','MDLedit'),'String'));
              rfi_strat = 2;
          else % DataRadio is set
		% Use value from generated data (exact)
              rfi_strat = 3;
              rfi_value = -1;    
          end
      elseif (get(findobj(fig,'Tag','FFTradio'),'Value'))
	  rfi_strat=-1; % default -- (FFT doesn't use a RFI detection strategy like
	  rfi_value=-1; % default -- inner product BF does)
          BFstrat = 2;   
      end
      dirpath='data';
      save([dirpath '\bf_options.mat'], 'BFstrat','rfi_strat','rfi_value');
      
      handles = guidata(fig);
      
      
	  delete(fig);
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
function varargout = radiobutton1_Callback(h, eventdata, handles, varargin)
    if (get(h, 'Value'))
        % unset FFT radiobutton
        g = findobj('Tag','FFTradio');
        set(g, 'Value',0);
	
	set(findobj('Tag','MDLradio'),'Enable','on');
	set(findobj('Tag','ThresholdRadio'),'Enable','on');
	set(findobj('Tag','DataRadio'),'Enable','on');

	if (get(findobj('Tag','MDLradio'),'Value'))
	    set(findobj('Tag','MDLedit'),'Enable','on');
	elseif (get(findobj('Tag','ThresholdRadio'),'Value'))
  	    set(findobj('Tag','ThresholdEdit'),'Enable','on');
	end
    else
        set(findobj('Tag','InnerProductRadio'),'Value',1);
    end      



% --------------------------------------------------------------------
function varargout = OkButton_Callback(h, eventdata, handles, varargin)
    uiresume;



% --------------------------------------------------------------------
function varargout = FFTradio_Callback(h, eventdata, handles, varargin)
    if (get(h, 'Value'))
        % unset inner product radiobutton
        g = findobj('Tag','InnerProductRadio');
        set(g, 'Value', 0);

	set(findobj('Tag','MDLradio'),'Enable','off');
	set(findobj('Tag','ThresholdRadio'),'Enable','off');
	set(findobj('Tag','DataRadio'),'Enable','off');
	
	set(findobj('Tag','MDLedit'),'Enable','off');
	set(findobj('Tag','ThresholdEdit'),'Enable','off');
    else
        set(findobj('Tag','FFTradio'),'Value',1);
    end



% --------------------------------------------------------------------
function varargout = MDLradio_Callback(h, eventdata, handles, varargin)

    if get(h,'Value')
        set(findobj('Tag','ThresholdRadio'),'Value',0);
        set(findobj('Tag','DataRadio'),'Value',0);
        set(findobj('Tag','MDLedit'),'Enable','on');
        set(findobj('Tag','ThresholdEdit'),'Enable','off');    
    else
        set(findobj('Tag','MDLradio'),'Value',0);    
    end
        
      




% --------------------------------------------------------------------
function varargout = ThresholdRadio_Callback(h, eventdata, handles, varargin)
    if get(h,'Value')
        set(findobj('Tag','MDLradio'),'Value',0);
        set(findobj('Tag','DataRadio'),'Value',0);
        set(findobj('Tag','ThresholdEdit'),'Enable','on');    
        set(findobj('Tag','MDLedit'),'Enable','off');    
    else
        set(findobj('Tag','ThresholdRadio'),'Value',1);
    end
    

% --------------------------------------------------------------------
function varargout = DataRadio_Callback(h, eventdata, handles, varargin)
    if get(h,'Value')
        set(findobj('Tag','ThresholdRadio'),'Value',0);
        set(findobj('Tag','MDLradio'),'Value',0);
        set(findobj('Tag','ThresholdEdit'),'Enable','off');    
        set(findobj('Tag','MDLedit'),'Enable','off');    
    else
        set(findobj('Tag','DataRadio'),'Value',1);
    end
    
    
% --------------------------------------------------------------------
function varargout = MDLedit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ThresholdEdit_Callback(h, eventdata, handles, varargin)

