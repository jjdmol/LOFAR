function X = centree_reduit(Z)
% Fonction qui calcule la matrice centree reduite � partir de la matrice de donn�es

% Calcul des moyennes et des �carts types empiriques de chaque variable
% et cr�ation d'une matrice avec les vecteurs moyennes et �carts types
% en lignes
UN = ones(size(Z));
Moy = mean(Z);
Moy = diag(Moy);
Moy = UN * Moy;

taille=size(Z,1);
Ecart_type = diag(sqrt((taille-1)/taille)*std(Z));
Ecart_type = UN * Ecart_type;

% Calcul de la matrice X des donn�es centr�es r�duites
X = (Z - Moy)./Ecart_type ;

