function [Path_dat,Path_config,Path_info]=Path_Obtention;
h=get(findobj('Tag','StationSimGUI'));
Path=get(findobj(h.Children,'tag','Path_Edit'),'String');
Path_dat = [Path '\Matlab_Dat_Files\'];
Path_config = [Path '\Config_Files\'];
Path_info = [Path '\Info_Files\'];