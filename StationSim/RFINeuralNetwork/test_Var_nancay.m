% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_hits_nino
%
%  Afficle les poids avec la cardinalite (Hits)
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
global MyVar;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec les Hits ou Cardinalite (nombre de
%%% données par cell)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
MyVar2=reshape(MyVar,sizeMap(1),sizeMap(2));
% Effacer tout label
sMap = som_label(sMap, 'clear');

% Calculer les ŒHits¹
% Hits ... pour savoir combien de données sont attachés a
% chaque cellule du map.
Hits = som_hits(sMap, sData);

% affiche les poids par plans. La taille des cellules est
% proportionnelle a sa cardinalite (Hits)
figure
%h=som_show(sMap,[], '', 'vert', Hits);
% Afficher seulement le plan 1 avec les Hits :
h=som_show_Var(sMap,2,'normalized','vert', MyVar2);
% Afficher en Taille Fixe tous les plans :
%h=som_show(sMap);

% ajoute la cardinalité comme Label et affiches les Labels.
H1 = MyVar;
My=[];
I1 = 1:length(MyVar);
My=[MyVar I1'];
sMap = som_label(sMap,I1', num2cell(My));
hlbl=som_addlabels(sMap,'all','all',[7],'b');