function varargout = GUI_sources(varargin)
% GUI_SOURCES Application M-file for GUI_sources.fig
%    FIG = GUI_SOURCES launch GUI_sources GUI.
%    GUI_SOURCES('callback_name', ...) invoke the named callback.
    global NAntennas;
    global chain_l;
    global mat;
    global compteur_sig;
    global list_sig;
    global list_traj;
    global list_trajectory;
    global Filename_genconfig;
    global compteur_traj;
    global sig;
    global Npositions;
    global AntennaSignals
    global traject;
    global NumberOfAntennas;
    global Filename_array;
    global Filename_beam
    global nsubbands;
    global Path_dat;
    global Path_config;
    global Path_info;
    global Total_Time;
    global compteur;
    global Nfft;
    global traj_struct;
    global record_traj;

    
if nargin == 0  % LAUNCH GUI
    
    indicator = 0;
    compteur=0;
    compteur_sig=0;
    compteur_traj=0;
    AntennaSignals=[];
    
    traj_struct=struct('Phi','Theta','Speed','Time');
	fig = openfig(mfilename,'reuse');    compteur=0;
	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
    set(handles.figure1,'Name','DataGenerator');
	guidata(fig, handles);
    [Path_dat,Path_config,Path_info]=Path_Obtention;
    
    h=get(findobj('Tag','StationSimGUI'));
    dirpath=[get(findobj(h.Children,'tag','Path_Edit'),'String') '\']
    if exist(dirpath)==0
       message='Path does not exist';
       close(handles.figure1); 
       title='Warning';
       msgbox(message,title,'Warn')
   else
    
    if exist([dirpath 'data'])==0
       message='Directories does not exist, creating directories... : data, Config_Files, Matlab_Dat_Files, Info_Files';
       title='Warning';
       msgbox(message,title,'Warn')
       mkdir data;
       mkdir Config_Files
       mkdir Matlab_Dat_Files
       mkdir Info_Files
       save('data/Path_info.mat','Path_dat','Path_config','Path_info')
    else
    save('data/Path_info.mat','Path_dat','Path_config','Path_info')
    end
    end
   
	if nargout > 0
		varargout{1} =fig;
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
function varargout = Apply_traj_button_Callback(h, eventdata, handles, varargin)

global compteur;
global traj_struct;
global mat;
global list_traj;

if length(get(findobj('Tag','NFFt_edit'),'string'))==0
    message='NFFt = 0';
    title='Warning';
    msgbox(message,title,'Warn')
end
if str2num(get(findobj('Tag','sampl_freq_edit'),'string'))==0
     message='Sampling Frequency = 0';
     title='Warning';
     msgbox(message,title,'Warn')  
end

if str2num(get(findobj('Tag','Total_time_edit'),'string'))==0
    message='Total Time = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else
    
% if compteur>1
% traj_struc(compteur-1)=traj_struc(compteur);
% end
compteur=compteur+1;
phi=str2num((get(findobj('Tag','Phi_edit'),'string')))
theta=str2num((get(findobj('Tag','theta_edit'),'string'))) 
Speed=str2num((get(findobj('Tag','Speed_edit'),'string')))
Time=str2num((get(findobj('Tag','Time_edit'),'string')))
traj_struct(compteur).Phi=phi;traj_struct(compteur).Theta=theta;traj_struct(compteur).Speed=Speed;traj_struct(compteur).Time=Time
for i=1:compteur
list_traj{i}=['   ' num2str(traj_struct(i).Phi) ' -  ' num2str(traj_struct(i).Theta) '  -  ' num2str(traj_struct(i).Speed) ' - ' num2str(traj_struct(i).Time) '' ];
mat(i,:)=[traj_struct(i).Phi,traj_struct(i).Theta,traj_struct(i).Speed,traj_struct(i).Time];
end
g = findobj('Tag','Listbox_Load_Trajectory');
%assignin('base','mat',mat)
set(g, 'String',list_traj);
end


% --------------------------------------------------------------------
function varargout = Save_traj_button_Callback(h, eventdata, handles, varargin)
global mat;
global list_traj;
global compteur;
global Filename_Beam;
global nsubbands;
global Path_dat;
global Path_config;
global Path_info;



Nfft=str2num((get(findobj('Tag','NFFt_edit'),'string')));
frequence=str2num((get(findobj('Tag','sampl_freq_edit'),'string')));
Total_Time=str2num((get(findobj('Tag','Total_time_edit'),'string')));

if Nfft==0
    message='NFFt = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else 
    if frequence==0
        message='frequence = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else 
    if Total_Time==0
        frequence==0
        message='Total Time = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else  
    
[Phi_theta]=interpolation(mat,Nfft,frequence,Total_Time);

if  get(findobj('Tag','Beam_checkbox'), 'Value')==0
Nfft=str2num((get(findobj('Tag','NFFt_edit'),'string')));
[Filename_traj,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');
if strfind(Filename_traj,'.dat')>0
Filename_traj=Filename_traj(1:strfind(Filename_traj,'.dat')-1);
end
fid=fopen([Path_dat Filename_traj '.dat'],'w+');
fprintf(fid,['nfft : ' num2str(Nfft) '\n']);
fprintf(fid,'npoints : %g\n',size(Phi_theta,1));
fprintf(fid,'position :\n');
for i=1:size(Phi_theta,1)
   fprintf(fid,'%g  %g  %g\n', Phi_theta(i,:).');
end
fclose(fid);
fid=fopen([Path_config Filename_traj '.config'],'w+');
fprintf(fid,' %g  x %g\n',size(Phi_theta,1),size(Phi_theta,2)-1);
fprintf(fid,'[');
for i=1:size(Phi_theta,1)
   fprintf(fid,'%g  %g  \n', Phi_theta(i,1:2));
end
fprintf(fid,']');
fclose(fid);

fid=fopen([Path_info Filename_traj '.info'],'w+');
fprintf(fid,'Source trajectory : \n');
fprintf(fid,['nfft : ' num2str(Nfft) '\n']);
fprintf(fid,'Phi      -      Theta      -     Time    -     Speed : \n')
for i=1:compteur
fprintf(fid,[list_traj{i} '\n']);
end
fclose(fid)

else 
nsubbands=(get(findobj('Tag','NFFt_edit'), 'String'));

[Filename_Beam,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');    
if strfind(Filename_Beam,'.dat')>0
Filename_Beam=Filename_Beam(1:strfind(Filename_Beam,'.dat')-1);
end
fid=fopen([Path_dat Filename_Beam '.dat'],'w+');
fprintf(fid,['nsubbands :' num2str(nsubbands) '\n']);
fprintf(fid,'npoints : %g\n',size(Phi_theta,1));
fprintf(fid,'position :\n');
for i=1:size(Phi_theta,1)
   fprintf(fid,'%g  %g  %g\n', Phi_theta(i,:).');
end
fclose(fid);

fid=fopen([Path_config Filename_Beam '.config'],'w+');
fprintf(fid,['nsubbands : ' num2str(nsubbands) '\n']);
fprintf(fid,' %g  x %g\n',size(Phi_theta,1),size(Phi_theta,2)-1);
fprintf(fid,'[');
for i=1:size(Phi_theta,1)
   fprintf(fid,'%g  %g  \n', Phi_theta(i,1:2));
end
fprintf(fid,']');
fclose(fid);


fid=fopen([Path_info Filename_Beam '.info'],'w+');
fprintf(fid,['nsubbands : ' num2str(nsubbands) '\n']);
fprintf(fid,'Beam trajectory : \n');
fprintf(fid,'Phi - Theta - Time - Speed : \n');
for i=1:compteur
fprintf(fid,[list_traj{i} '\n']);
end
fclose(fid)
end
end
end
end



% --------------------------------------------------------------------
function varargout = Sig_gen_button_Callback(h, eventdata, handles, varargin)
%addpath C:\users\Dromer\Home\Data_Generator\
% addpath C:\Jerome\Astron\Data_Generator\
Gui_datagen



% --------------------------------------------------------------------
function varargout = load_Mod_sig_gen_Callback(h, eventdata, handles, varargin)
global sig;
global list_sig;
global compteur_sig;
global Total_Time;
global Path_dat;

[filename_sig,Pathname]=uigetfile([Path_dat '*.dat'],'Load The File ');
stop=0;
fid=fopen([Pathname filename_sig],'r');
while(stop==0)   
line=fgetl(fid);
resp0=strfind(line,'Fsampling');
if length(resp0)>0
Fsampl=str2num(line(resp0(1)+11:25));
Total_Time=str2num(line(strfind(line,'Total Time :')+12:length(line)));
end
resp1=strfind(line,'Data');
if length(resp1)
    stop=1;
end
end
compteur_sig=compteur_sig+1;
sig=fscanf(fid,'%e');
assignin('base','sig',sig)
fclose(fid);
list_sig{compteur_sig}=filename_sig;
% for i=1:compteur_sig
%     list_edit{i}=list_sig{i};
% end
g = findobj('Tag','listbox_sig');
set(g, 'String',list_sig);
g = findobj('Tag','Total_time_edit');
set(g, 'String',Total_Time);
g = findobj('Tag','sampl_freq_edit');
set(g, 'String',Fsampl);
%assignin('base','Fsampl',Fsampl)



 

% --------------------------------------------------------------------
function varargout = Apply_add_Button_Callback(h, eventdata, handles, varargin)

global sig;
global traject;
global AntennaSignals;
global chain_l;
global Nfft;
global NAntennas;


if  length(get(findobj('Tag','NFFt_edit'),'string'))==0
    message='You must give a number of FFT points';
    title='Warning';
    msgbox(message,title,'Warn')
else
if  str2num(get(findobj('Tag','NFFt_edit'),'string'))==0
    message='You must give a number of FFT points';
    title='Warning';
    msgbox(message,title,'Warn')

else

if length(chain_l)==0
     message='You must save a array configuration';
    title='Warning';
    msgbox(message,title,'Warn')
else 
            g = findobj('Tag','ArrayTypeList');
            array_type = get(g, 'Value');

            % get parameters for selected array type
            [chain,params] = arrayparams(array_type);
            [px,py] = arraybuilder(array_type, chain);
             
            %
            % Snap to grid if requested to
            %
            null_mask=1;
            GridSize = str2num(get(findobj( 'Tag', 'GridEdit'),'String'));
            SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
            NullGrid = get(findobj('Tag', 'NullGridCheck'),'Value');

            xg = (max(px) - min(px)) / (GridSize-1);
            yg = (max(py) - min(py)) / (GridSize-1);
           
            if (SnapToGrid)
                xg=1; % default
                yg=1;
                [px,py,xg,yg,null_mask] = Fitgrid(px,py,GridSize,NullGrid);
            end

            % Only now can we check the number of antennas (could be changed by NullGrid)
            NumberOfAntennas = length(py);
            g = findobj('Tag','sampl_freq_edit');
            bandwidth=str2num(get(g, 'String'))
            NAntennas=length(px);
            
            diff=size(traject,2)*Nfft-size(sig,1);
            
            if (diff<0)
                AntennaSignals=zeros(NAntennas,size(traject,2)*Nfft);
                sig=sig(1:size(traject,2)*Nfft);
            else 
            if (diff>0)
                AntennaSignals=zeros(NAntennas,length(sig)-mod(length(sig),Nfft));
                sig=sig(1:length(sig)-mod(length(sig),Nfft));
            end
            end 
            
            fprintf('Generating data...\n');
            sh = nband_siggen4(NumberOfAntennas, bandwidth, Nfft, px, py,sig,traject);
            AntennaSignals = sh + AntennaSignals;
            %assignin('base','AntennaSignals',AntennaSignals)
end
end
end


% --------------------------------------------------------------------
function varargout = ArrayTypeList_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');
    % get parameters for selected array type
    params = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);
    NumberOfAntennas = length(y);
    f = findobj('Tag','AntennaEdit');
    set(f,'String',num2str(NumberOfAntennas));
    



%--------------------------------------------------------------------
function varargout = SnapToCheck_Callback(h, eventdata, handles, varargin)


if (get(h, 'Value'))
    set(findobj('Tag','NullGridCheck'),'Enable','on');
else
    set(findobj('Tag','NullGridCheck'),'value',0); 
    set(findobj('Tag','NullGridCheck'),'Enable','off');    
end


%--------------------------------------------------------------------
function varargout = PlotArrayButton_Callback(h, eventdata, handles, varargin)
global chain;
global Array_config;
global NumberofAntennas;
global Filename_array;

g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    params = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);

    %
    % Snap to grid if requested to
    %
     SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
    NullGrid= get(findobj('Tag','NullGridCheck'),'Value');
    null=ones(1,size(x,2));
    if (SnapToGrid)
        old_x=x;
        old_y=y;
        GridSize = str2num(get(findobj('Tag', 'GridEdit'),'String'));
        xg=1; % default
        yg=1;
        [x,y,xg,yg,null] = Fitgrid(x,y,GridSize,NullGrid);
    end
   
    % visualize the array without the nulled antennas
    
    x = x .* null;
    y = y .* null;
    
    Array_config=[x.' y.'];
    figure(2);
    plot(x,y,'s','markersize',2,'markerfacecolor',[0 0 1]),grid
    if (SnapToGrid)
        hold on;
        plot(old_x,old_y,'s','markersize',2,'markerfacecolor','y','markeredgecolor','r')
        hold off
    else
       
    end
    xlabel('x-location (/lambda)'), ylabel('y-location (/lambda)')

    if NullGrid
        title('Array Configuration - grid nulled');
    elseif SnapToGrid
        title('Array Configuration - snap to grid %dx%d',GridSize,GridSize);
    else
        title('Array Configuration');
    end;
    axis equal
    axis square 




% --------------------------------------------------------------------
function varargout = Load_trajectory_text_Callback(h, eventdata, handles, varargin)


global compteur_traj;
global list_trajectory;
global traject;
global mat;
global Filename_beam;
global Path_dat;
global nsubbands;
global record_traj;
global Npositions;
global Nfft;
        

[filename_traj,Pathname]=uigetfile([Path_dat '*.dat'],'Load The File ');
stop=0;
if get(findobj('Tag','Beam_load_checkbox'), 'Value');
Filename_beam=filename_traj;
end
fid=fopen([Pathname filename_traj],'r');
t=0;
compteur_traj=compteur_traj+1;
while(stop==0)   
line=fgetl(fid);
t=t+1;
resp0=strfind(line,'npoints :');
if length(resp0)>0
    lig=str2num(line(resp0+9:length(line)));
end
resp=strfind(line,'nsubbands :');
if length(resp)>0
   nsubbands=str2num(line(resp+11:length(line)))
end
resp=strfind(line,'nfft :');
if length(resp)>0
   Nfft=str2num(line(resp+6:length(line)))
end
if length(strfind(line,'position :'))>0;
    stop=1;
end
end

list_trajectory{compteur_traj}=filename_traj;

if get(findobj('Tag','Beam_load_checkbox'), 'Value')==0;
record_traj=list_trajectory;
set(findobj('Tag','Nfft_text'),'string','Datagenerator NFFT ');  
set(findobj('Tag','NFFt_edit'),'string',Nfft);
else
set(findobj('Tag','Nfft_text'),'string','Nsubbands for beamforming');  
 set(findobj('Tag','NFFt_edit'),'string',nsubbands);   
end

Npositions=lig;
traject=fscanf(fid,'%g',[3 lig]);
assignin('base','traject',traject);
fclose(fid);
set(findobj('Tag','listbox_trajectory_vis'),'string',list_trajectory);




% --------------------------------------------------------------------
function varargout = Save_sources_Callback(h, eventdata, handles, varargin)
global AntennaSignals;
global list_sig;
global list_trajectory;
global chain_l;
global NumberOfAntennas;
global Total_Time;
global Filename_array;
global nsubbands;
global Filename_beam;
global Filename_genconfig;
global Path_dat;
global Path_config;
global Path_info;
global Nfft;

if length(Filename_beam)==0;
    message='You must load or generate a beam trajectory';
    title='Warning';
    msgbox(message,title,'Warn')
else
[Filename_genconfig ,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');
if strfind(Filename_genconfig,'.dat')>0
Filename_genconfig=Filename_genconfig(1:strfind(Filename_genconfig,'.dat')-1);
end

Filename_genconfig_dat=[Filename_genconfig '.dat'];
fid=fopen([Path_dat Filename_genconfig_dat],'w+');
fprintf(fid,'%g x %g\n',size(AntennaSignals,2),size(AntennaSignals,1));
fprintf(fid,'[\n');
fprintf(fid,'%f',real(AntennaSignals));
fprintf(fid,']\n');
fprintf(fid,'[\n');
fprintf(fid,'%f',imag(AntennaSignals));
fprintf(fid,'\n]\n');
fclose(fid)


if get(findobj('Tag', 'Blitz_Checkbox'),'value')
Filename_genconfig_dat=[Filename_genconfig '.blitz'];
fid=fopen([Path_config Filename_genconfig_dat],'w+');
fprintf(fid,'%g x %g\n',size(AntennaSignals,2),size(AntennaSignals,1));
fprintf(fid,'[');
for i=1:size(AntennaSignals,2)
    for j=1:size(AntennaSignals,1)
    fprintf(fid,'(%g,%g)\t', real(AntennaSignals(j,i)),imag(AntennaSignals(j,i)));
    end
 fprintf(fid,'\n');
end
fprintf(fid,']');
fclose(fid);
end

SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
NullGrid= get(findobj('Tag','NullGridCheck'),'Value');

Filename_genconfig_config=[Filename_genconfig '.config'];
fid=fopen([Path_config Filename_genconfig_config],'w+');
fprintf(fid,'Nfft : %d \n',Nfft);
fprintf(fid,'fs : %d \n',str2num(get(findobj('Tag','sampl_freq_edit'), 'String')));
fprintf(fid,'time : %d \n',str2num(get(findobj('Tag','Total_time_edit'), 'String')));
fprintf(fid,'nant : %g\n',NumberOfAntennas);
fprintf(fid,'snapToGrid : %s\n',num2str(SnapToGrid));
fprintf(fid,'nullGrid : %s\n',num2str(NullGrid));
fprintf(fid,'arrayfile : %s\n',[Filename_array '.config']);
Filename_rec=Filename_beam(1:strfind(Filename_beam,'.dat')-1);
fprintf(fid,'beamtrajectory : %s\n',[Filename_rec '.config']);
fprintf(fid,['nsources : ' num2str(length(list_sig)) '\n\n' ]);
fprintf(fid,'Modulated Signal    -     Trajectory :\n');
for i=1:length(list_sig)
Filename_record1=list_sig{i};
Filename_record2=list_trajectory{i};
Filename_record1=Filename_record1(1:strfind(Filename_record1,'.dat')-1);
Filename_record2=Filename_record2(1:strfind(Filename_record2,'.dat')-1);
fprintf(fid,['source : ' Filename_record1 '.config' '\t' Filename_record2 '.config' '\n']);
end
fclose(fid);

Filename_genconfig_info=[Filename_genconfig  '.info'];
fid=fopen([Path_info Filename_genconfig_info],'w+');
fprintf(fid,'Nfft : %d \n',Nfft);
fprintf(fid,'Bandwidth : %d \n',str2num(get(findobj('Tag','sampl_freq_edit'), 'String')));
fprintf(fid,'Total Time : %d \n\n',str2num(get(findobj('Tag','Total_time_edit'), 'String')));
fprintf(fid,'Sources definition :\n ');
fprintf(fid,'Modulated Signal  -  Trajectory :\n');
for i=1:length(list_sig)
Filename_record1=list_sig{i};
Filename_record2=list_trajectory{i};
Filename_record1=Filename_record1(1:strfind(Filename_record1,'.dat')-1);
Filename_record2=Filename_record2(1:strfind(Filename_record2,'.dat')-1);
fprintf(fid,[Filename_record1 '.info' '  |  ' Filename_record2 '.info' '\n']);
end
fprintf(fid,'\nArray configuration : \n');
fprintf(fid,'Number of Antennas : %g\n',NumberOfAntennas);
fprintf(fid,'Array Type : %s\n',chain_l{1});
Filename_rec=Filename_beam(1:strfind(Filename_beam,'.dat')-1);
fprintf(fid,'Beamtrajectory : %s\n',[Filename_rec '.info']);
fprintf(fid,'ArrayFile : %s\n',[Filename_array '.info']);
fprintf(fid,'SnapToGrid : %s\n',num2str(SnapToGrid));
fprintf(fid,'NullGrid : %s\n',num2str(NullGrid));
fclose(fid);
set(findobj('Tag','Sources_gen_listbox'),'string',[Filename_genconfig '.dat']);
end


% --------------------------------------------------------------------
function varargout = Total_time_edit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Save_array_button_Callback(h, eventdata, handles, varargin)
global Array_config;
global chain_l;
global AntennaSignals;
global NumberOfAntennas;
global Filename_array;
global Path_dat;
global Path_info;
global Path_config;

g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    [params,chain] = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);

    %
    % Snap to grid if requested to
    %
     SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
    NullGrid= get(findobj('Tag','NullGridCheck'),'Value');
    null=ones(1,size(x,2));
    if (SnapToGrid)
        old_x=x;
        old_y=y;
        GridSize = str2num(get(findobj('Tag', 'GridEdit'),'String'));
        xg=1; % default
        yg=1;
        [x,y,xg,yg,null] = Fitgrid(x,y,GridSize,NullGrid);
    end
   
    % visualize the array without the nulled antennas
    
    x = x .* null;
    y = y .* null;

Array_config=[x.' y.'];
NumberOfAntennas=length(x);
[Filename_array,Pathname]=uiputfile([Path_dat '*.dat'],'Save The File ');
if strfind(Filename_array,'.dat')>0
Filename_array=Filename_array(1:strfind(Filename_array,'.dat')-1);
end
Filename_array_dat=[Filename_array '.dat'];
fid=fopen([Path_dat Filename_array_dat],'w+');
fprintf(fid,'%g\n',NumberOfAntennas)
for i=1:size(Array_config,2)
    fprintf(fid,'%g',NumberOfAntennas)
    fprintf(fid, ' [\n');
    fprintf(fid,'%g\t', Array_config(:,i));
    fprintf(fid,'\n]');
    fprintf(fid,'\n');
end
fclose(fid);

Filename_array_config=[Filename_array '.config'];
fid=fopen([Path_config Filename_array_config],'w+');
fprintf(fid,'%g\n',NumberOfAntennas)
for i=1:size(Array_config,2)
    fprintf(fid,'%g',NumberOfAntennas)
    fprintf(fid, ' [\n');
    fprintf(fid,'%g\t', Array_config(:,i));
    fprintf(fid,'\n]');
    fprintf(fid,'\n');
end
fclose(fid);

Filename_array_info=[Filename_array '.info'];
fid=fopen([Path_info Filename_array_info],'w+');
fprintf(fid,'Number of Antennas : %g\n',NumberOfAntennas);
fprintf(fid,'Array Type : %s\n',chain);
fprintf(fid,'File : %s\n',Filename_array_info);
fprintf(fid,'SnapToGrid : %s\n',num2str(SnapToGrid));
fprintf(fid,'NullGrid : %s\n',num2str(NullGrid));
fclose(fid);
chain_l{1}=chain;


% --------------------------------------------------------------------
function varargout = Beam_checkbox_Callback(h, eventdata, handles, varargin)
 
 if  get(findobj('Tag','Beam_checkbox'), 'Value')
   set(findobj('Tag','Nfft_text'),'string','Nsubbands for beamforming');  
   set(findobj('Tag','Trajectory_generation_text'),'string','Beam trajectory');
else if  get(findobj('Tag','Beam_checkbox'), 'Value')==0
   set(findobj('Tag','Nfft_text'),'string','Datagenerator NFFT ');  
   set(findobj('Tag','Trajectory_generation_text'),'string','Source trajectory');
 end
end
 
 


% --------------------------------------------------------------------
function varargout = Beam_load_checkbox_Callback(h, eventdata, handles, varargin)

 if  get(findobj('Tag','Beam_load_checkbox'), 'Value')
   set(findobj('Tag','Trajectory_load_text'),'string','Beam trajectory');  
else if  get(findobj('Tag','Beam_checkbox'), 'Value')==0  
   set(findobj('Tag','Trajectory_load_text'),'string','Source trajectory');
 end
end


% --------------------------------------------------------------------
function varargout = NFFt_edit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = NullGridCheck_Callback(h, eventdata, handles, varargin)

if get(findobj('Tag','SnapToCheck'),'value')==0;
   set(findobj('Tag','NullGridCheck'),'value',0);
   set(findobj('Tag','NullGridCheck'),'Enable','off');
% else
%     set(findobj('Tag','NullGridCheck'),'Enable','off');    
end


% --------------------------------------------------------------------
function varargout = Apply_gen_button_Callback(h, eventdata, handles, varargin)
global Filename_beam;
global Filename_array;
global Filename_genconfig;
global nsubbands;
global Nfft;
global Npositions;
global NAntennas;
global record_traj;

if length(strfind(Filename_genconfig,'.info'))>0
Filename_genconfig=[Filename_genconfig(1:strfind(Filename_genconfig,'.info')-1)];
end 
if length(strfind(Filename_array,'.dat'))==0
Filename_array=[Filename_array '.dat'];
end 

save('data\configuration.mat','record_traj','NAntennas','Filename_beam','Npositions','Filename_array','Nfft','Filename_genconfig','nsubbands');
h=get(findobj('Tag','StationSimGUI'));
set(findobj(h.Children,'tag','Nfft_data_resu'),'String',num2str(Nfft));
set(findobj(h.Children,'tag','nsubbands_text'),'String',num2str(nsubbands));
set(findobj(h.Children,'tag','Npoints_text'),'String',num2str(Npositions));
set(findobj(h.Children,'tag','SignalButton'),'BackgroundColor',[0.11 0.36 0.59]);
clear sig;
clear AntennaSignals;
clear Fsampl;
clear Nfft;
close(gcf)


% --------------------------------------------------------------------
function varargout = Clear_traj_button_Callback(h, eventdata, handles, varargin)

global mat;
global compteur;
global list_traj;
mat=[];
compteur=0;
list_traj={};
set(findobj('Tag','Listbox_Load_Trajectory'),'string',list_traj);


% --------------------------------------------------------------------
function varargout = Plot_trajectory_button_Callback(h, eventdata, handles, varargin)
global mat
if length(mat)==0
     message='You must generate a trajectory';
    title='Warning';
    msgbox(message,title,'Warn')
else
frequence=str2num((get(findobj('Tag','sampl_freq_edit'),'string')));
Total_Time=str2num((get(findobj('Tag','Total_time_edit'),'string')));
Nfft=str2num((get(findobj('Tag','NFFt_edit'),'string')));
if Nfft==0
    message='NFFt = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else 
    if frequence==0
        message='frequence = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else 
    if Total_Time==0
        frequence==0
        message='Total Time = 0';
    title='Warning';
    msgbox(message,title,'Warn')
else  
    

g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    [params,chain] = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);

    %
    % Snap to grid if requested to
    %
     SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
    NullGrid= get(findobj('Tag','NullGridCheck'),'Value');
    null=ones(1,size(x,2));
    if (SnapToGrid)
        old_x=x;
        old_y=y;
        GridSize = str2num(get(findobj('Tag', 'GridEdit'),'String'));
        xg=1; % default
        yg=1;
        [x,y,xg,yg,null] = Fitgrid(x,y,GridSize,NullGrid);
    end
   
    % visualize the array without the nulled antennas
    
    px = x .* null;
    py = y .* null;


    
[Phi_theta]=interpolation(mat,Nfft,frequence,Total_Time);

    digwindow=1; 
    relfreq=1;

 % Generate Digital beamforming window 
 digwin = taper(px, py, digwindow, 128);
 N=100;
 pat_steered=[];
 LookingDirection=steerv(px,py,0,0);
 fin=N*N;
 i=0;
 phi_theta=[];
 time = 0
  for u = 0:N
     u1 = -1 + 2*u/N;
     for v = 0:N
         v1 = -1 + 2*v/N;
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi = atan2(u1,v1);
             phi_theta=[phi_theta; phi theta];
             S{u+1,v+1}=[phi,theta];
             if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end

             pat_steered(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(LookingDirection.')';
           
         end;
     end;
 end

BeamPattern = abs(pat_steered)/max(max(abs(pat_steered)));
[Phi_theta]=interpolation(mat,Nfft,frequence,Total_Time);

figure(1)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(Phi_theta(:,1)).*sin(Phi_theta(:,2)),sin(Phi_theta(:,2)).*sin(Phi_theta(:,1)),'*m')
hold off
end
end
end

end


% --------------------------------------------------------------------
function varargout = pushbutton14_Callback(h, eventdata, handles, varargin)

global list_trajectory
global compteur;
global traj_struct;
global compteur_sig;
global list_sig;
global compteur_traj;
global sig;
global AntennaSignals;
global Filename_genconfig;
global Path_dat;
global Path_config;
global Path_info;

Filename_genconfig=[];
file=[];
sig={}
list_sig={}
list_trajectory={}
compteur=0;
compteur_sig=0;
compteur_traj=0;
AntennaSignals=[];
load('data/Path_info.mat');
set(findobj('Tag','Sources_gen_listbox'),'string',file);
set(findobj('Tag','listbox_sig'),'string',list_sig);
set(findobj('Tag','listbox_trajectory_vis'),'string',list_trajectory);
set(findobj('Tag','Beam_load_checkbox'),'value',0);


% --------------------------------------------------------------------
function varargout = Load_sources_button_Callback(h, eventdata, handles, varargin)
global Filename_beam;
global Filename_array;
global Filename_genconfig;
global nsubbands;
global Nfft;
global Path_info;
global Path_dat;
global Npositions;
global NAntennas;
global record_traj;

[Filename_genconfig,Pathname]=uigetfile([Path_info '*.info'],'Save The File ');
fid=fopen([Pathname Filename_genconfig],'r')
stop=0;
tra=0
while(stop==0)   
line=fgetl(fid);
resp0=strfind(line,'Nfft :');
if length(resp0)>0
Nfft=str2num(line(resp0+6:length(line)));
end
resp11=strfind(line,'|');
if length(resp11)>0
    tra=tra+1;
    essai=line(resp11+3:length(line));
    record_traj{tra}=[essai(1:strfind(essai,'.info')-1) '.dat'];
end
resp1=strfind(line,'Beamtrajectory :');
if length(resp1)>0
   Filename_beam=line(resp1+17:length(line));
end
    
    
resp2=strfind(line,'ArrayFile :');
if length(resp2)>0
   Filename_array=line(resp2+12:length(line));
   stop=1;
end
end
fclose(fid);
Filename_array=[Filename_array(1:strfind(Filename_array,'.info')-1) '.dat'];
Filename_beam=[Filename_beam(1:strfind(Filename_beam,'.info')-1) '.dat'];
fid = fopen([Path_dat Filename_beam],'r')
line=fgetl(fid);
resp3=strfind(line,'nsubbands :');
nsubbands=str2num(line(resp3+11:length(line)));
fclose(fid)
set(findobj('tag','Sources_gen_listbox'),'string',Filename_genconfig)

Filename_dat=[Filename_genconfig(1:strfind(Filename_genconfig,'.info')-1) '.dat'];
fid = fopen([Path_dat Filename_dat ],'r');
line=fgetl(fid);
resp4=strfind(line,'x');
Npositions=str2num(line(1:resp4-1))/Nfft;
NAntennas=str2num(line(resp4+1:length(line)));
fclose(fid)




% --------------------------------------------------------------------
function varargout = Blitz_Checkbox_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Signal_Analyser_Button_Callback(h, eventdata, handles, varargin)

global sig;

if length(sig)==0
      message='You must load a signal';
    title='Warning';
    msgbox(message,title,'Warn') 
else 
save('data\signal_analyser.mat','sig');
gui_FFT;
end

