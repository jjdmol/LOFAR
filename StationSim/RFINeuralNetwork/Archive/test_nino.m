% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_nino
%
%  - Lecture des donnees El Nino
%  - Specification d'une carte topologique
%  - Entrainement de la carte 
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Taille de la fenetre temporelle en nombre de pixels
% chaque pixel etant la donnee moyenne d'un mois de mesures
% la taille de la fenetre est donc le nombre de mois pris
% simultanement
t_fenetre = 3;

% Indice de la variable ElNino a prendre en compte (1,2,3,4)
var_nino  = 2;

[nino_don, nino_varnames] = load_nino(var_nino, t_fenetre);

n_data = size(nino_don,1);

nino_labels = [];

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definition de la Structure des Donnees
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sData = som_data_struct(nino_don,'nino', nino_labels, nino_varnames);

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
%alpha      = 0.1;
alpha      = [];
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Entrainement fin de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

epochs     = 50;
radius     = [2 0.2];
%alpha      = 0.05;
alpha      = [];
alpha_type = 'linear';
tracking   = 3;
sMap       = som_train(sMap,sData,epochs, radius, alpha, alpha_type, tracking);

figure
som_show(sMap,'all')
