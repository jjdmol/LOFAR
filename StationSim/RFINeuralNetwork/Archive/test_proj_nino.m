% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fichier script test_proj_nino
%
%  - Projection de la carte par ACP-Som Toolbox
%  - Projection de la carte par Sammon-Som Toolbox
%  - Projection de la carte et des donnees par ACP-Matlab
%    Stat toolbox (fonction PRINCOMP)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Projection de la carte
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

pMap = som_projection(sMap,2,'pca');
figure
som_showgrid(sMap,pMap),  axis on
%plot(sData.data(1,:),sData.data(2,:),'*r');
hold off,
title('Projection par ACP');


pMap = som_sammon(sMap,2);
figure
som_showgrid(sMap,pMap), hold off, axis on
title('Projection par Sammon');








% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Projection de la carte et des donnees a l'aide de
%   l'ACP Matlab: PRINCOMP
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
N = size(sMap.codebook,3);
X = reshape(sMap.codebook,prod(sMap.msize),N);
[PC, SCORE, LATENT, TSQUARE] = princomp(X);

 %Calcule les nouveaux codebooks projetes dans les axes de l'ACP
newX = X * PC;

New_codebook = reshape(newX, [sMap.msize, N]);

Xcb1 = New_codebook(:,:,1);
Xcb2 = New_codebook(:,:,2);
Xcb3 = New_codebook(:,:,3);

 %Donnees projetes dans les axes de l'ACP
new_nino_don = nino_don * PC;

Xdon1 = new_nino_don(:,1);
Xdon2 = new_nino_don(:,2);
Xdon3 = new_nino_don(:,3);

figure
plot(Xdon1,Xdon2,'*r',Xcb1,Xcb2,'b',Xcb1',Xcb2','b',Xcb1,Xcb2,'ob')
set(gca,'YDir','reverse')
axis image
