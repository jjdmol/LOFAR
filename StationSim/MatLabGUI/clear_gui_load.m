    clear all;
    close(gcf);
    global mat;
    global compteur_sig;
    global list_sig;
    global list_traj;
    global list_trajectory;
    global Filename_genconfig;
    global compteur_traj;
    global sig;
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
    global traj_struct;
    compteur=0;
    compteur_sig=0;
    compteur_traj=0;
    AntennaSignals=[];
    [Path_dat,Path_config,Path_info]=Path_Obtention;

   