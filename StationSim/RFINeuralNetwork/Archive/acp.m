function [ACP,indexacp]=acp(X)
% Calcul de l'ACP de la matrice X
% La matrice ACP contient sur chaque ligne : 
% 1ere colonne : valeurs propres dans l'ordre décroissant
% (2:nb)eme colonne : coordonnees du vecteur propre correspondant
% nb : nombre de vecteurs propres
 

[n p]=size(X);
disp([int2str(n) ' individus']);
disp([int2str(p) ' variables']);


% Calcul de la matrice de corrélation de X
COR=corrcoef(X);

% Calcul des valeurs et vecteurs propres de la matrice de corrélation
[VEP VAP] = eig(COR)
VAP = diag(VAP);
test=VEP*VEP';

% Calcul du nombre de valeurs propres
nb = length(VAP);
disp([int2str(nb) ' composantes trouvées'])

% Association de chaque vecteur propre à chaque valeur propre 
% (sur une meme ligne)
ACP=[VAP VEP'];

% Classement des valeurs propres par ordre décroissant
[ACP,indexacp]=sortrows(ACP,1); % Classement des lignes selon la premiere colonne
ACP=flipud(ACP);
indexacp=flipud(indexacp);
