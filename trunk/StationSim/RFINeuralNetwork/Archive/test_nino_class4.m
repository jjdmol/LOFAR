% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_nino_class
%
% Meme script que test_nino_class mais on ajoute
% la declaration des labels selon la presence ou absence
% en Temps T du phenomene El Nino.
%
%  - Lecture des donnees El Nino
%  - declaratoion des 2 classes:
%    - Presence du phenomene El Nino
%    - Absence du phenomene
%  - Specification d'une carte topologique
%  - Entrainement de la carte 
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Taille de la fenetre temporelle en nombre de pixels:
% Chaque pixel etant la donnee moyenne d'un mois de mesures
% la taille de la fenetre est donc le nombre de mois a prendre
% simultanement. Les pixels representent des periodes de temps
% passes. Si taille est egale a 3 alors ils representent la
% variable en temps T-3, T-2 et T-1.


t_fenetre = 4;

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Indice de la variable de temperature El Nino a prendre en
% compte. Il y a 4 variables de temperature (1,2,3,4)

var_nino = 2;

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Indice de la variable utilise pour declarer le Label
var_nino_lab = 2;
% Valeur d'Alpha dans la formule de normalisation utilise
% pour la procedure d'etiquetage.
alpha_norm = 1;
% Seuil pour etiquetage.
seuil_norm = 1;
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Chargement des donnees et labels
[nino_don, nino_labels, nino_num_labels, nino_varnames] = ...
      load_nino_class(var_nino, var_nino_lab, t_fenetre, alpha_norm, seuil_norm);

n_data = size(nino_don,1);

nino_cls = nino_num_labels + 1;

%foncton_analyse_acp(sData.data)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition de la Structure des Donnees
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sData = som_data_struct(nino_don,'nino', nino_labels, nino_varnames);
foncton_analyse_acp(sData.data);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Normalisation des Donnees (pas obligatoire)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%sData = som_normalize_data(sData,'som_var_norm');

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialisation de la carte topologique ou 'map'
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

msize     = [7 7];
init_type = 'random';   %% 'random' ou 'linear'
lattice   = 'hexa';     %% 'hexa' ou 'rect'
shape     = 'rect';     %% 'rect', 'cyl', ou 'toroid'
sMap  = som_init(sData, msize, init_type, lattice, shape);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Options de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Neigh     = 'gaussian'; %% 'gaussian', 'cutgauss', 'bubble' ou 'ep'
TrainType = 'batch';    %% 'seq' ou 'batch'
Mask      = ones(size(nino_don,2), 1);  %% active tous les donnees
sMap  = som_trainops(sMap, Neigh, TrainType, Mask);

figure

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

epochs     = 30;
radius     = [5 2];
alpha      = 0.1;
%alpha      = [];
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement fin de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

epochs     = 50;
radius     = [2 0.1];
%alpha      = 0.05;
alpha      = [];
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);


%subplot(1,2,1),
%som_showgrid(sMap,sMap.codebook,'surf')
%f_samm = gcf; 
%axis on,hold on
%plot3(sData.data(:,1),sData.data(:,2),sData.data(:,3),'*r')
%title('vision selon le trois composantes Carte et Données Nino1');
%rotate3d on
%hold off

figure
%subplot(1,2,2),
S = som_sammon(sMap,3,10,'seconds');
%f_samm = gcf; 
som_showgrid(sMap,S,'surf')
axis on,% hold on
%plot3(sData.data(:,1),sData.data(:,2),sData.data(:,3),'*r')
title('Projection par Sammon selon le trois composantes');
%rotate3d on
%hold off

