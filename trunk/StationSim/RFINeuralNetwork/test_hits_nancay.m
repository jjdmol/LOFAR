% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_hits_nino
%
%  Afficle les poids avec la cardinalite (Hits)
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
global MyVar;
global MyData2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Labeliser avec les Hits ou Cardinalite (nombre de
%%% données par cell)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
cardinalite =[];
for i=1:prod(sizeMap)
Cardina = length(find(MyData2==i-1)); 
cardinalite = [ cardinalite;Cardina];
end

% Effacer tout label
sMap = som_label(sMap, 'clear');

% Calculer les Hits¹
% Hits ... pour savoir combien de données sont attachés a
% chaque cellule du map.
Hits = reshape(cardinalite,sizeMap(1),sizeMap(2));

% affiche les poids par plans. La taille des cellules est
% proportionnelle a sa cardinalite (Hits)
figure
h=som_show_hist(sMap,[1], '', 'vert', Hits);
% Afficher seulement le plan 1 avec les Hits :
%h=som_show(sMap,[1],'normalized','vert',Hits);
% Afficher en Taille Fixe tous les plans :
%figure
%h=som_show(sMap);

% ajoute la cardinalité comme Label et affiches les Labels.
H1 = Hits(:);
I1 = 1:length(H1);
MyHits=[H1 I1'];
sMap = som_label(sMap,I1', num2cell(MyHits));
hlbl=som_addlabels(sMap,'all','all',[8],'g');