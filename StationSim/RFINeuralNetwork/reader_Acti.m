%Recherche des neurones Activés pour les afficher sur l'histogramme
global Contrl_Actives;    
global numero_donnee;
global range;
ecrire={};
ecrire1=[];
A=fichier{numero_donnee};
    A=str2num(A);
    for i=1:length(range(:,3))
    ecrire1=['N:' num2str(range(i,2)) ' - As :' num2str(range(i,3)) '']; 
    ecrire{i}=ecrire1;
    end
    
set(Contrl_Actives,'String',ecrire);