function beamformerGUI(varargin)
% BEAMFORMERGUI Application M-file for beamformerGUI.fig
%    FIG = BEAMFORMERGUI launch beamformerGUI GUI.
%    BEAMFORMERGUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 07-Apr-2003 12:23:15

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');


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
          
          useFFTbeamformer =get(findobj(fig, 'Tag','FFT_radio'),'Value');
          useInnerProduct =get(findobj(fig, 'Tag','Inner_Product_Radio'),'Value');
          if useInnerProduct
          Threshold_radio    = get(findobj(fig, 'Tag','Threshold_RFI_Radio'),'Value');
          MDLValue = get(findobj(fig, 'Tag','MDL_RFI_radio'),'Value');
          Threshold_value=str2num(get(findobj(fig, 'Tag','Threshold_RFI_Edit'),'String'));
          else
          Threshold_value = 0;
          MDLValue=0;
          Threshold_radio =0;
          end 
          
          BufferACM    = str2num(get(findobj(fig, 'Tag','forgetting_Edit'),'String'));
          BetaPASTd = str2num(get(findobj(fig, 'Tag','Beta_Edit'),'String'));
          useSVD    = get(findobj(fig, 'Tag','Sources_SVD_Radio'),'Value');
          useDetection    = get(findobj(fig, 'Tag','source_det_text'),'Value');
          useEVD    = get(findobj(fig, 'Tag','Sources_EVD_Radio'),'Value');
          plotSVD   = get(findobj(fig, 'Tag','SVD_Plotting_Radio'),'Value');
          plotEVD   = get(findobj(fig, 'Tag','EVD_Plotting_Radio'),'Value');
          plotPASTd = get(findobj(fig, 'Tag','PASTd_Plotting_Radio'),'Value');
          usePASTd = get(findobj(fig, 'Tag','PASTd_Track_Checkbox'),'Value');
          InitTrackSVD=get(findobj(fig, 'Tag','Track_SVD_Checkbox'),'Value');
          InitTrackEVD=get(findobj(fig, 'Tag','Track_EVD_Checkbox'),'Value');
          UpDateTimeTrack=str2num(get(findobj(fig, 'Tag','Update_Edit'),'String'));
          Snapshot_Buffer=str2num(get(findobj(fig, 'Tag','Snapshot_Edit'),'String'));
	      BufferTrack=str2num(get(findobj(fig, 'Tag','Track_Snapshot_Edit'),'String'));
          WindowStepTrack=str2num(get(findobj(fig, 'Tag','Step_Edit'),'String'));
          match=get(findobj('Tag','Match_Checkbox'),'value');
          Forgetting=str2num(get(findobj('Tag','forgetting_Edit'),'String'));
          
      
          dirpath='Data';
          save([dirpath '/bf_options.mat'],'Forgetting','Snapshot_Buffer','useDetection','usePASTd','BetaPASTd','useSVD','useEVD', ...
          'plotSVD','plotEVD','plotPASTd','useFFTbeamformer','useInnerProduct','BufferACM','InitTrackSVD','InitTrackEVD',...
          'UpDateTimeTrack','WindowStepTrack','Threshold_value','Threshold_radio','MDLValue','BufferTrack','match');
          h=get(findobj('Tag','StationSimGUI'));
          set(findobj(h.Children,'tag','BeamFormerButton'),'BackgroundColor',[0.11 0.36 0.59]);
          
          %Update main window
          load('data\configuration.mat','Npositions')
          set(findobj(h.Children,'tag','Npoints_text'),'string','on');
          
          if useDetection
          set(findobj(h.Children,'tag','text25'),'string','on');
          set(findobj(h.Children,'tag','text25'),'BackgroundColor',[0.11 0.36 0.59]);
          else
          set(findobj(h.Children,'tag','text25'),'string','off');
          end
          
          if usePASTd 
          set(findobj(h.Children,'tag','Npoints_text'),'Enable','on');
          load('data\configuration.mat','Npositions');
          maxUpdate=floor(Npositions/(UpDateTimeTrack*WindowStepTrack));
          set(findobj(h.Children,'tag','Npoints_text'),'string',maxUpdate);
          set(findobj(h.Children,'tag','edit5'),'string',maxUpdate);
          set(findobj(h.Children,'tag','edit5'),'Enable','on');
          set(findobj(h.Children,'tag','text16'),'Enable','on');
          set(findobj(h.Children,'tag','text18'),'Enable','on');
          set(findobj(h.Children,'tag','text26'),'string','on');
          set(findobj(h.Children,'tag','text26'),'BackgroundColor',[0.11 0.36 0.59]);
          else
          set(findobj(h.Children,'tag','text26'),'BackgroundColor',[0.92 0.91 0.84]);   
          set(findobj(h.Children,'tag','edit5'),'Enable','off');
          set(findobj(h.Children,'tag','text16'),'Enable','off');
          set(findobj(h.Children,'tag','text18'),'Enable','off');
          set(findobj(h.Children,'tag','Npoints_text'),'Enable','off');
          set(findobj(h.Children,'tag','text26'),'string','off');
          end
           

      
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
function varargout = Ok_Button_Callback(h, eventdata, handles, varargin)
 Snapshot_Buffer=str2num(get(findobj('Tag','Snapshot_Edit'),'String'));
 BufferTrack=str2num(get(findobj('Tag','Track_Snapshot_Edit'),'String'));
    if BufferTrack > Snapshot_Buffer
          message='You must use a number of snapshot for Pastd equal or less than the number of snapshot buffer used for EVD, SVD';
          title='Warning';
          msgbox(message,title,'Warn')
    else
      uiresume;
    end



% --------------------------------------------------------------------
function varargout = MDL_RFI_radio_Callback(h, eventdata, handles, varargin)
  if get(findobj('Tag','Threshold_RFI_Radio'),'Value')
        set(findobj('Tag','Threshold_RFI_Radio'),'Value',0);
        set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');   
        set(findobj('Tag','Threshold_text'),'Enable','off');      
        set(findobj('Tag','MDL_RFI_radio'),'Value',1);   
    else
        set(findobj('Tag','Threshold_RFI_Radio'),'Value',1);
        set(findobj('Tag','Threshold_RFI_Edit'),'Enable','on');  
        set(findobj('Tag','Threshold_text'),'Enable','on');  
        set(findobj('Tag','MDL_RFI_radio'),'Value',0);  
        
    end
        
      




% --------------------------------------------------------------------
function varargout = Threshold_RFI_Radio_Callback(h, eventdata, handles, varargin)
  if get(findobj('Tag','Threshold_RFI_Radio'),'Value') 
        set(findobj('Tag','MDL_RFI_radio'),'Value',0);
           set(findobj('Tag','Threshold_text'),'Enable','on');  
        set(findobj('Tag','Threshold_RFI_Radio'),'Value',1);
        set(findobj('Tag','Threshold_RFI_Edit'),'Enable','on');
    else
          set(findobj('Tag','Threshold_RFI_Radio'),'Value',0);
             set(findobj('Tag','Threshold_text'),'Enable','off');  
        set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');    
        set(findobj('Tag','MDL_RFI_radio'),'Value',1);  

     

    end
    





% --------------------------------------------------------------------
function varargout = EVD_Plotting_Radio_Callback(h, eventdata, handles, varargin)


   if get(h,'Value')
        set(findobj('Tag','EVD_Plotting_Radio'),'Value',1);
        set(findobj('Tag','SVD_Plotting_Radio'),'Value',0);  
        else set(findobj('Tag','SVD_Plotting_Radio'),'Value',1);
    end



% --------------------------------------------------------------------
function varargout = SVD_Plotting_Radio_Callback(h, eventdata, handles, varargin)
  if get(h,'Value')
        set(findobj('Tag','EVD_Plotting_Radio'),'Value',0);
        set(findobj('Tag','SVD_Plotting_Radio'),'Value',1);
    else      set(findobj('Tag','EVD_Plotting_Radio'),'Value',1);
    end



% --------------------------------------------------------------------
function varargout = PASTd_Plotting_Radio_Callback(h, eventdata, handles, varargin)



% --------------------------------------------------------------------
function varargout = PASTd_Track_Checkbox_Callback(h, eventdata, handles, varargin)

if  get(findobj('Tag','PASTd_Track_Checkbox'),'Value')==0;
          set(findobj('Tag','Beta_Edit'),'Enable','off');
          set(findobj('Tag','Track_SVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Track_EVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Update_Edit'),'Enable','off');
	      set(findobj('Tag','Mov_Window_Radio'),'Enable','off');
          set(findobj('Tag','Fix_Window_Radio'),'Enable','off');
	      set(findobj('Tag','Track_Snapshot_Edit'),'Enable','off');
          set(findobj('Tag','Step_Edit'),'Enable','off');
          set(findobj('Tag','Update_text'),'Enable','off');
          set(findobj('Tag','text17'),'Enable','off');
            set(findobj('Tag','Beta_Text'),'Enable','off');
             set(findobj('Tag','text16'),'Enable','off');
           set(findobj('Tag','text11'),'Enable','off');
             set(findobj('Tag','text19'),'Enable','off');
             set(findobj('Tag','text14'),'Enable','off');
             set(findobj('Tag','text20'),'Enable','off');
             set(findobj('Tag','PASTd_Plotting_Radio'),'Enable','off');
          
      else
          set(findobj('Tag','Beta_Edit'),'Enable','on');
          set(findobj('Tag','Track_SVD_Checkbox'),'Enable','on');
          set(findobj('Tag','Track_EVD_Checkbox'),'Enable','on');
          set(findobj('Tag','Update_Edit'),'Enable','on');
	      set(findobj('Tag','Mov_Window_Radio'),'Enable','on');
          set(findobj('Tag','Fix_Window_Radio'),'Enable','on');
	      set(findobj('Tag','Track_Snapshot_Edit'),'Enable','on');
          set(findobj('Tag','Step_Edit'),'Enable','on');
          set(findobj('Tag','Update_text'),'Enable','on');
          set(findobj('Tag','text17'),'Enable','off');
            set(findobj('Tag','Beta_Text'),'Enable','on');
             set(findobj('Tag','text16'),'Enable','on');
           set(findobj('Tag','text11'),'Enable','on');
             set(findobj('Tag','text19'),'Enable','on');
             set(findobj('Tag','text14'),'Enable','on');
             set(findobj('Tag','text20'),'Enable','on');
             set(findobj('Tag','text16'),'Enable','on');
              set(findobj('Tag','PASTd_Plotting_Radio'),'Enable','on');
      end
    


% --------------------------------------------------------------------
function varargout = Sources_EVD_Radio_Callback(h, eventdata, handles, varargin)

if get(findobj('Tag','Sources_EVD_Radio'),'Value')==1;
   set(findobj('Tag','Track_EVD_Checkbox'),'Value',1)
    set(findobj('Tag','Track_SVD_Checkbox'),'Value',0)
   set(findobj('Tag','Sources_SVD_Radio'),'Value',0)
   set(findobj('Tag','forgetting_Edit'),'Enable','on');
   set(findobj('Tag','Forgetting_text'),'Enable','on');
else
   set(findobj('Tag','Sources_SVD_Radio'),'Value',1)
    set(findobj('Tag','Track_EVD_Checkbox'),'Value',0)
    set(findobj('Tag','Track_SVD_Checkbox'),'Value',1)
   set(findobj('Tag','forgetting_Edit'),'Enable','off');
   set(findobj('Tag','Forgetting_text'),'Enable','off');
end


% --------------------------------------------------------------------
function varargout = forgetting_Edit_Callback(h, eventdata, handles, varargin)








% --------------------------------------------------------------------
function varargout = Track_SVD_Checkbox_Callback(h, eventdata, handles, varargin)
if get(findobj('Tag','Track_SVD_Checkbox'),'Value')==1;
   set(findobj('Tag','Track_EVD_Checkbox'),'Value',0);
   set(findobj('Tag','Sources_SVD_Radio'),'Value',1);
   set(findobj('Tag','Sources_EVD_Radio'),'Value',0);
   set(findobj('Tag','forgetting_Edit'),'Enable','off');
   set(findobj('Tag','Forgetting_text'),'Enable','off');
   set(findobj('Tag','Track_EVD_Checkbox'),'Value',0);
else
   set(findobj('Tag','Sources_EVD_Radio'),'Value',1);
   set(findobj('Tag','Sources_SVD_Radio'),'Value',0);
   set(findobj('Tag','forgetting_Edit'),'Enable','on');
   set(findobj('Tag','Forgetting_text'),'Enable','on');
   set(findobj('Tag','Track_EVD_Checkbox'),'Value',1);
   
end



% --------------------------------------------------------------------
function varargout = source_det_text_Callback(h, eventdata, handles, varargin)

if  get(findobj('Tag','source_det_text'),'Value')==0;
          set(findobj('Tag','Sources_EVD_Radio'),'Enable','off');
          set(findobj('Tag','Sources_SVD_Radio'),'Enable','off');
          set(findobj('Tag','Snapshot_Edit'),'Enable','off');
          set(findobj('Tag','forgetting_Edit'),'Enable','off');
          set(findobj('Tag','MDL_RFI_radio'),'Enable','off');
          set(findobj('Tag','Threshold_RFI_Radio'),'Enable','off');
          set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');
          set(findobj('Tag','EVD_Plotting_Radio'),'Enable','off');
          set(findobj('Tag','SVD_Plotting_Radio'),'Enable','off');
          set(findobj('Tag','PASTd_Plotting_Radio'),'Enable','off');
          set(findobj('Tag','Beta_Edit'),'Enable','off');
          set(findobj('Tag','Track_SVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Track_EVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Update_Edit'),'Enable','off');
	      set(findobj('Tag','Mov_Window_Radio'),'Enable','off');
          set(findobj('Tag','Fix_Window_Radio'),'Enable','off');
	      set(findobj('Tag','Track_Snapshot_Edit'),'Enable','off');
          set(findobj('Tag','Step_Edit'),'Enable','off');
          set(findobj('Tag','PASTd_Track_Checkbox'),'Enable','off');
          set(findobj('Tag','text11'),'Enable','off');
          set(findobj('Tag','Update_text'),'Enable','off');
          set(findobj('Tag','text17'),'Enable','off');
            set(findobj('Tag','Beta_Text'),'Enable','off');
             set(findobj('Tag','text16'),'Enable','off');
           set(findobj('Tag','Forgetting_text'),'Enable','off');
             set(findobj('Tag','text19'),'Enable','off');
             set(findobj('Tag','text14'),'Enable','off');
             set(findobj('Tag','text20'),'Enable','off');
             set(findobj('Tag','Snapshot_text'),'Enable','off');
             set(findobj('Tag','Plotting_text'),'Enable','off');
             set(findobj('Tag','Threshold_text'),'Enable','off');
             set(findobj('Tag','RFI_source_text'),'Enable','off');
     
             
      else 
          set(findobj('Tag','Sources_EVD_Radio'),'Enable','on');
          set(findobj('Tag','Sources_SVD_Radio'),'Enable','on');
          set(findobj('Tag','Snapshot_Edit'),'Enable','on');
          set(findobj('Tag','MDL_RFI_radio'),'Enable','on');
          set(findobj('Tag','Threshold_RFI_Radio'),'Enable','on');
          set(findobj('Tag','Threshold_RFI_Edit'),'Enable','on');
          set(findobj('Tag','EVD_Plotting_Radio'),'Enable','on');
          set(findobj('Tag','SVD_Plotting_Radio'),'Enable','on');  
          set(findobj('Tag','Snapshot_text'),'Enable','on');
          set(findobj('Tag','Plotting_text'),'Enable','on');
          set(findobj('Tag','PASTd_Track_Checkbox'),'Enable','on');
          set(findobj('Tag','Threshold_text'),'Enable','on');
          set(findobj('Tag','RFI_source_text'),'Enable','on');
          
          if  get(findobj('Tag','PASTd_Track_Checkbox'),'value')==0;
            set(findobj('Tag','PASTd_Plotting_Radio'),'Enable','off');
          set(findobj('Tag','Beta_Edit'),'Enable','off');
          set(findobj('Tag','Track_SVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Track_EVD_Checkbox'),'Enable','off');
          set(findobj('Tag','Update_Edit'),'Enable','on');
	      set(findobj('Tag','Mov_Window_Radio'),'Enable','off'); 
	      set(findobj('Tag','Track_Snapshot_Edit'),'Enable','off');
          set(findobj('Tag','Step_Edit'),'Enable','off'); 
           set(findobj('Tag','text11'),'Enable','off');
          set(findobj('Tag','Update_text'),'Enable','off');
          set(findobj('Tag','text17'),'Enable','off');
            set(findobj('Tag','Beta_Text'),'Enable','off');
             set(findobj('Tag','text16'),'Enable','off');
           set(findobj('Tag','Update_Edit'),'Enable','off');
             set(findobj('Tag','text19'),'Enable','off');
             set(findobj('Tag','text14'),'Enable','off');
             set(findobj('Tag','text20'),'Enable','off');
             set(findobj('Tag','text16'),'Enable','off');
              
          else 
          set(findobj('Tag','PASTd_Plotting_Radio'),'Enable','on');
          set(findobj('Tag','Beta_Edit'),'Enable','on');
          set(findobj('Tag','Track_SVD_Checkbox'),'Enable','on');
          set(findobj('Tag','Track_EVD_Checkbox'),'Enable','on');
          set(findobj('Tag','Update_Edit'),'Enable','on');
	      set(findobj('Tag','Mov_Window_Radio'),'Enable','on'); 
	      set(findobj('Tag','Track_Snapshot_Edit'),'Enable','on');
          set(findobj('Tag','Step_Edit'),'Enable','on'); 
          set(findobj('Tag','PASTd_Track_Checkbox'),'Enable','on');
           set(findobj('Tag','text11'),'Enable','on');
          set(findobj('Tag','text17'),'Enable','on');
            set(findobj('Tag','Beta_Text'),'Enable','on');
             set(findobj('Tag','text16'),'Enable','on');
           set(findobj('Tag','Update_Edit'),'Enable','on');
             set(findobj('Tag','text19'),'Enable','on');
             set(findobj('Tag','text14'),'Enable','on');
             set(findobj('Tag','text20'),'Enable','on');
             set(findobj('Tag','text16'),'Enable','on');
          end 
       
         
         if get(findobj('Tag','FFT_radio'),'value')==1;
             set(findobj('Tag','Threshold_text'),'Enable','off');
             set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');
              set(findobj('Tag','MDL_RFI_radio'),'Enable','off');
                 set(findobj('Tag','Threshold_RFI_Radio'),'Enable','off');
         else
             set(findobj('Tag','Threshold_text'),'Enable','on');
             set(findobj('Tag','Threshold_RFI_Edit'),'Enable','on');
             set(findobj('Tag','Threshold_RFI_Radio'),'Enable','on');
             set(findobj('Tag','MDL_RFI_radio'),'Enable','on');
         end
         
           if get(findobj('Tag','Threshold_RFI_Radio'),'value')==0;
             set(findobj('Tag','Threshold_text'),'Enable','off');
             set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');
         else
             set(findobj('Tag','Threshold_text'),'Enable','on');
             set(findobj('Tag','Threshold_RFI_Edit'),'Enable','on');
         end 
         
          if  get(findobj('Tag','Sources_EVD_Radio'),'value')==0;
              set(findobj('Tag','forgetting_Edit'),'Enable','off');
              set(findobj('Tag','Forgetting_text'),'Enable','off');
          else 
              set(findobj('Tag','Forgetting_text'),'Enable','on');
              set(findobj('Tag','forgetting_Edit'),'Enable','on');
          end
      end



% --------------------------------------------------------------------
function varargout = edit9_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Track_EVD_Checkbox_Callback(h, eventdata, handles, varargin)

if get(findobj('Tag','Track_EVD_Checkbox'),'Value')==1;
   set(findobj('Tag','Sources_EVD_Radio'),'Value',1);  
   set(findobj('Tag','Sources_SVD_Radio'),'Value',0);
    set(findobj('Tag','forgetting_Edit'),'Enable','on');
    set(findobj('Tag','Forgetting_text'),'Enable','on');
   set(findobj('Tag','Track_SVD_Checkbox'),'Value',0);
   
% else 
%      set(findobj('Tag','Sources_EVD_Radio'),'Value',0);  
%    set(findobj('Tag','Sources_SVD_Radio'),'Value',1);
%     set(findobj('Tag','forgetting_Edit'),'Enable','off');
%     set(findobj('Tag','Forgetting_text'),'Enable','off');
%    set(findobj('Tag','Track_SVD_Checkbox'),'Value',1)
end




% --------------------------------------------------------------------
function varargout = Sources_SVD_Radio_Callback(h, eventdata, handles, varargin)

if get(findobj('Tag','Sources_SVD_Radio'),'Value')==1;
   set(findobj('Tag','Track_EVD_Checkbox'),'Value',0)
    set(findobj('Tag','Track_SVD_Checkbox'),'Value',1)
   set(findobj('Tag','Sources_EVD_Radio'),'Value',0)
   set(findobj('Tag','forgetting_Edit'),'Enable','off');
   set(findobj('Tag','Forgetting_text'),'Enable','off');
else
   set(findobj('Tag','Sources_SVD_Radio'),'Value',0)
   set(findobj('Tag','Sources_EVD_Radio'),'Value',1)
    set(findobj('Tag','Track_EVD_Checkbox'),'Value',1)
    set(findobj('Tag','Track_SVD_Checkbox'),'Value',0)
   set(findobj('Tag','forgetting_Edit'),'Enable','on');
   set(findobj('Tag','Forgetting_text'),'Enable','on');
end



% --------------------------------------------------------------------
function varargout = Inner_Product_Radio_Callback(h, eventdata, handles, varargin)
 if (get(h, 'Value'))
        % unset FFT radiobutton
        g = findobj('Tag','FFT_radio');
        set(g, 'Value',0);
    


	if (get(findobj('Tag','MDL_RFI_radio'),'Value'))
	    set(findobj('Tag','MDL_RFI_Edit'),'Enable','on');

    else
        set(findobj('Tag','Inner_Product_Radio'),'Value',1);
    end    	
    
    if get(findobj('Tag','source_det_text'),'value')==1;
	set(findobj('Tag','MDL_RFI_radio'),'Enable','on');
	set(findobj('Tag','Threshold_RFI_Radio'),'Enable','on');
    set(findobj('Tag','RFI_source_text'),'Enable','on');
	set(findobj('Tag','Threshold_text'),'Enable','on');
    end
end




% --------------------------------------------------------------------
function varargout = FFT_radio_Callback(h, eventdata, handles, varargin)
 if (get(h, 'Value'))
        % unset inner product radiobutton
        g = findobj('Tag','Inner_Product_Radio');
        set(g, 'Value', 0);

	set(findobj('Tag','MDL_RFI_radio'),'Enable','off');
	set(findobj('Tag','Threshold_RFI_Radio'),'Enable','off');
	set(findobj('Tag','RFI_source_text'),'Enable','off');
	set(findobj('Tag','Threshold_text'),'Enable','off');
    set(findobj('Tag','Threshold_RFI_Edit'),'Enable','off');
    else
        set(findobj('Tag','FFT_radio'),'Value',1);
        
    end








% --------------------------------------------------------------------
function varargout = checkbox8_Callback(h, eventdata, handles, varargin)

