function varargout = station_gui(varargin)
% STATION_GUI Application M-file for station_gui.fig
%    FIG = STATION_GUI launch station_gui GUI.
%    STATION_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 08-Apr-2003 14:26:07


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


% The sub-band splitter and the channel splitter don't differ in functionality
% Since they both use the same code, both can use the same GUI.
function varargout = SubBandButton_Callback(h, eventdata, handles, varargin)
    subband_gui;
%   g = findobj('Tag','SubBandEdit');
%   text=['#Subbands: ',num2str(NumberSubBands)];
%   set(g, 'String', text);


function varargout = ChannelButton_Callback(h, eventdata, handles, varargin)
    channel_gui; 


% --------------------------------------------------------------------
function varargout = BeamFormerButton_Callback(h, eventdata, handles, varargin)
    beamformerGUI ;
    

% --------------------------------------------------------------------
function varargout = SignalButton_Callback(h, eventdata, handles, varargin)
  % open new window containing options for the input signal
  % including which antennae to use, signal source and noise level
  % signal_fig = openfig('signal_gui.fig','reuse');
  %AntennaSignals = signal_gui;
  gui_sources


% --------------------------------------------------------------------
function varargout = OutputButton_Callback(h, eventdata, handles, varargin)
    output_gui;



% --------------------------------------------------------------------
function varargout = AddWeightButton_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = File_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Print_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Close_Callback(h, eventdata, handles, varargin)
  % close all gui windows
  close all

% --------------------------------------------------------------------
function varargout = RunButton_Callback(h, eventdata, handles, varargin)
    % generate pattern
    dirpath=[get(findobj('tag','Path_Edit'),'string') '\'];
    if exist([dirpath 'data\configuration.mat'])==0
         message='You must load or configure some data';
        title='Warning';
        msgbox(message,title,'Warn')
    end
    if exist([dirpath 'data\subband_options.mat'])==0
        message='You must configure the subband splitter';
        title='Warning';
        msgbox(message,title,'Warn')
    end
    if exist([dirpath 'data\bf_options.mat'])==0
        message='You must configure the beamforming block';
        title='Warning';
        msgbox(message,title,'Warn') 
    end
    if exist([dirpath 'data\output_options.mat'])==0
        message='You must configure the ouput visualisation';
        title='Warning';
        msgbox(message,title,'Warn')
    end
    if exist([dirpath 'data\channel_options.mat'])==0
        message='You must configure the channel splitter';
        title='Warning';
        msgbox(message,title,'Warn')
    end
         
    load([dirpath 'data\configuration.mat']);
    load([dirpath 'data\bf_options.mat']);
    load([dirpath 'data\output_options.mat']);
    load([dirpath 'data\subband_options.mat']);
    file=[dirpath 'Matlab_Dat_Files\' Filename_genconfig '.dat'];
    Filename_array=[dirpath 'Matlab_Dat_Files\' Filename_array];
    Filename_beam=[dirpath 'Matlab_Dat_Files\' Filename_beam];
    [AntennaSignals] = reader_data(file);
    [px,py]=reader_array(Filename_array,NAntennas);
    [Beam_Phi_theta]=reader_BeamTrajectory(Filename_beam);
    [Phi_theta]=reader_sourcesTrajectory(record_traj,dirpath);
    resolution=str2num(get(findobj('Tag','Resolution_Edit'),'String'));
     assignin('base','AntennaSignals',AntennaSignals);
     assignin('base','Phi_theta',Phi_theta);
     assignin('base','Beam_Phi_theta',Beam_Phi_theta);
     assignin('base','py',py);
     assignin('base','px',px);
    
    
    % not functiun of the subband taht we look at ??? 
    if (beam_side|beam_3d|beam_contour|beam_top)
    fprintf('Generating pattern\n');
    tic;
    %Generating pattern
    genepattern(px,py,Beam_Phi_theta,dirpath,resolution);
    fprintf('Plotting pattern\n');
    
    %plot pattern
    plotpattern(px,py,dirpath);
    end
    
    if NumberSubBands~=1
    if (Method_polyphase==0)
    fprintf('FFT Subband splitter...\n');
    fft_subband(NumberSubBands,[1:NumberSubBands],AntennaSignals);
    else
    fprintf('Polyphase Subband splitter...\n');
    polyphase_Antenna(AntennaSignals,NumberSubBands,SubbandFilterLength,NumberSubBands,option_polyphase,SelectedSubBands);
    end
    end
       
%          if (RFIblanking & rfi_mit_power)
%              fprintf('\tPlotting RFI blanking results\n');
%              load([dirpath '/antenna_signals']);
%              RFImitResults(AntennaSignals,SelectedSubBands,DFTSystemResponse,NumberSubBands,SubbandFilterLength, ...
%                  SelectedSubBandSignals, NumberOfAntennas, size(SelectedSubBandSignals),FlaggingCube,sb_quant_signal,...
%                  sb_quant_inputfft,sb_quant_outputfft); 
%          end 

     % Calculating the eigen system on a large (nulled) grid may fill the memory to overflow
     
     
     
     %TO DO add the same for every subbands : PLay with the name of the diffrent files to make the distinction
     
     %Loop for EVD refresh
     load([dirpath 'data/bf_options.mat']);
     load([dirpath 'data/antenna_signals.mat']);
     EVD_Bool=useEVD;
     SVD_Bool=useSVD;
     PASTd_Bool=usePASTd;
     %number of refresh for EVD : floor((number of snapshot)/ UpdateTimePastd*MovingWindow)

     
     StepUser=str2num(get(findobj('tag','edit5'),'String'));
     RefreshEVD_SVD=floor(size(SelectedSubBandSignals,2)/(UpDateTimeTrack*WindowStepTrack));
     if RefreshEVD_SVD<=StepUser
     RefreshEVD_SVD=floor(size(SelectedSubBandSignals,2)/(UpDateTimeTrack*WindowStepTrack))-1;%-1 to prevent exceed dimension in applying Pastd
     else
     RefreshEVD_SVD=StepUser;
     end
     
     offset=1;
     RefreshPASTd=WindowStepTrack;
     %RefreshPASTd_end=floor((size(SelectedSubBandSignals,2)-(UpDateTimeTrack*WindowStepTrack)*RefreshEVD_SVD)/WindowStepTrack);
     WeightVectorMatrix=[]; %matrix to store the WeightVector
     WeightTechnic=[];;
     indicator=0 %used to reload either the previous Pastd Weights or the initialised one 
     track_frames=0;
     
     
     for index_plot=1:RefreshEVD_SVD-1 %RefreshEVD_SVD
            
            %STA function indicator
            EVD_Bool=useEVD;
            SVD_Bool=useSVD;
            PASTd_Bool=0;
            
            track_frames=track_frames+1; %Frame counter 
            
            %Position to read the data out
            position_startEVD_SVD=offset
            position_stopEVD_SVD=offset+Snapshot_Buffer-1
            
            %Data Selection
            xq=SelectedSubBandSignals(:,position_startEVD_SVD:position_stopEVD_SVD,SelectedSubBands);
            
            %Apply STA Algorithm
            eigensystem(xq,dirpath,px,py,Beam_Phi_theta,Phi_theta,EVD_Bool,SVD_Bool,PASTd_Bool,position_stopEVD_SVD,indicator);  
            
            %Plot output time series 
            fprintf('Plotting spectrum\n');
            plotspectrum(xq,px,py,Beam_Phi_theta(1,position_stopEVD_SVD),Beam_Phi_theta(2,position_stopEVD_SVD),position_stopEVD_SVD);
            
            %Save the WeightVector in a matrix
            load([dirpath 'data/rfi_eigen.mat']);
            WeightVectorMatrix(track_frames,:)=WeightVector.';
            WeightTechnic(track_frames,:)=[EVD_Bool, SVD_Bool, PASTd_Bool];
            % plot pattern without RFI, one beam (no subband splitter).  
            if (ad_beam_top|ad_beam_contour|ad_beam_side|ad_beam_3d)==1
            fprintf('Generating pattern with rfi nulls\n');
            genebeamnulls(dirpath,px,py,resolution);
            fprintf('Plotting new pattern\n');    
            plotpatternrfi(dirpath,px,py,Phi_theta,position_startEVD_SVD,position_stopEVD_SVD);
            
            %Frame acquisition
            figure(5)
            mov1(track_frames) = getframe(gcf);
            figure(6)
            mov2(track_frames) = getframe(gcf);
            figure(7)
            mov3(track_frames) = getframe(gcf);
            figure(8)
            mov4(track_frames) = getframe(gcf);
            figure(9)
            mov5(track_frames) = getframe(gcf);   
            end
        
         if usePASTd==1
         for Pastd_Apply=1:WindowStepTrack
             
            EVD_Bool=0;
            SVD_Bool=0;
            PASTd_Bool=1;
            
            track_frames=track_frames+1; %Frame counter 
            
            if Pastd_Apply==1
                indicator=1;
            else indicator=0;
            end
            
            %Position to read the data out
            position_startPastd=position_stopEVD_SVD-BufferTrack+Pastd_Apply*WindowStepTrack
            position_stopPastd=position_stopEVD_SVD+Pastd_Apply*WindowStepTrack
            
            %Data Selection
            xq=SelectedSubBandSignals(:,position_startPastd:position_stopPastd,SelectedSubBands);
            
            %Apply STA Algorithm
            eigensystem(xq,dirpath,px,py,Beam_Phi_theta,Phi_theta,EVD_Bool,SVD_Bool,PASTd_Bool,position_stopPastd,indicator);  
            
            %Plot output time series 
            fprintf('Plotting spectrum\n');
            plotspectrum(xq,px,py,Beam_Phi_theta(1,position_stopPastd),Beam_Phi_theta(2,position_stopPastd),position_stopPastd);
            
            %Save the WeightVector in a matrix
            load([dirpath 'data/rfi_eigen.mat']);      
            WeightVectorMatrix(track_frames,:)=WeightVector.';
            WeightTechnic(track_frames,:)=[EVD_Bool, SVD_Bool, PASTd_Bool];
            
              if (ad_beam_top|ad_beam_contour|ad_beam_side|ad_beam_3d)==1
               % plot pattern without RFI, one beam (no subband splitter).
               fprintf('Generating pattern with rfi nulls\n');
               genebeamnulls(dirpath,px,py,resolution);
               fprintf('Plotting new pattern\n');    
               plotpatternrfi(dirpath,px,py,Phi_theta,position_startPastd,position_stopPastd);
            
         
               %Frame acquisition
               figure(5)
               mov1(track_frames) = getframe(gcf);
               figure(6)
               mov2(track_frames) = getframe(gcf);
               figure(7)
               mov3(track_frames) = getframe(gcf);
               figure(8)
               mov4(track_frames) = getframe(gcf);
               figure(9)
               mov5(track_frames) = getframe(gcf);   
              end
            end%End for
         end%End if Pastd
        offset=Snapshot_Buffer+offset;
    end %End for
    
  
    %AVI Conversion
    if size(mov1)~=0
        movie2avi(mov1,'C:\Documents and Settings\dromer\Desktop\Pattern_Phi_theta')
    end
    if size(mov2)~=0
        movie2avi(mov2,'C:\Documents and Settings\dromer\Desktop\Pattern_cartesien')
    end
    if size(mov3)~=0
        movie2avi(mov3,'C:\Documents and Settings\dromer\Desktop\Side_View')
    end
    if size(mov4)~=0
        movie2avi(mov4,'C:\Documents and Settings\dromer\Desktop\Pattern_3D')
    end
    if size(mov5)~=0
        movie2avi(mov5,'C:\Documents and Settings\dromer\Desktop\Time_series_subband')
    end;
   
    save([dirpath 'data/WeightVector.mat'],'WeightVectorMatrix','WeightTechnic')
    %Save in blietz format
    dirpath_subband=[dirpath 'Config_Files\'];
    filename0='WeightVector.blitz';
    filename1='WeightTechnic.blitz';
    record_blietz(WeightVectorMatrix,dirpath_subband,filename0)
    record_blietz(WeightTechnic,dirpath_subband,filename1)
    
    %     if (get(findobj('Tag', 'ChannelEnableCheck'),'Value'))
%         fprintf('Channel splitter\n');
%         ChannelSplitter(get(findobj('Tag','BFenableCheck'),'Value'));
%         load([dirpath '/channel_options.mat'])
%         if (CH_RFIblanking & rfi_mit_power)
%             fprintf('\tPlotting RFI blanking results\n');
%             load([dirpath '/antenna_signals']);
%             RFImitResults(BFSignals,SelectedChannels,DFTSystemResponse,NumberChannels,ChannelFilterLength, ...
%                 SelectedChannelSignals, NumberOfAntennas, size(SelectedChannelSignals),FlaggingCube,ch_quant_signal,ch_quant_inputfft,...
%                 ch_quant_outputfft); 
%         end
%     end
    toc;



    
% --------------------------------------------------------------------
function varargout = SubbandEnableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ChannelEnableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BFenableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = DFTtestButton_Callback(h, eventdata, handles, varargin)
    DFTFilterBankTest;



% --------------------------------------------------------------------
function varargout = pushbutton13_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit2_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit5_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = checkbox4_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = checkbox5_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = checkbox6_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = checkbox7_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = checkbox8_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Clear_button_Callback(h, eventdata, handles, varargin)


set(findobj('tag','SignalButton'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','BeamFormerButton'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','SubBandButton'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','ChannelButton'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','OutputButton'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','text26'),'String','off');
set(findobj('tag','text25'),'String','off');
set(findobj('tag','text25'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','text26'),'BackgroundColor',[0.92 0.91 0.84]);
set(findobj('tag','Nfft_data_resu'),'String',0);
set(findobj('tag','nsubbands_text'),'String',0);
set(findobj('tag','Npoints_text'),'String',0);
path=get(findobj('Tag','Path_Edit'),'String');
delete([path 'data\configuration.mat']);
delete([path 'data\subband_options.mat']);
delete([path 'data\channel_options.mat']);
delete([path 'data\bf_options.mat']);
delete([path 'data\output_options.mat']);
delete([path 'data\Path_info.mat']);


% --------------------------------------------------------------------
function varargout = edit6_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Resolution_Edit_Callback(h, eventdata, handles, varargin)

