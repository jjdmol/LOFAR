global U;

%classe=[];
%for i=1:length(MatClass)
  % [ligne,colonne]=find(MatClass(i,:)== max(MatClass(i,:)));
   %if colonne==1
   %   sMap.labels{i}=['non']; 
    %  classe=[classe 1];
    % else sMap.labels{i}=['OUI'];
   %   classe=[classe 2];
   % end
   %end

%sMap = som_autolabel(sMap,sData,'vote');
%figure
%som_show(sMap,1);
%sMap=som_label(sMap,[1:49],lab[1:49])
%som_addlabels(sMap,1,'all',[],'black')

x1=sMap.codebook(:,:,1);
x2=sMap.codebook(:,:,2);
x3=sMap.codebook(:,:,3);

trans1=sMap.codebook(:,:,1)';
trans2=sMap.codebook(:,:,2)';
trans3=sMap.codebook(:,:,3)';

[ligcarte,colcarte]=size(sMap.codebook(:,:,3));
taille_carte=colcarte*ligcarte;
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

Xcb1 = New_codebook(:,:,arg_acpplan1);
Xcb2 = New_codebook(:,:,arg_acpplan2);
%Xcb3 = New_codebook(:,:,3);
%Xcb4 = New_codebook(:,:,4);
%Xcb5 = New_codebook(:,:,5);

Xcbt1=Xcb1';
Xcbt2=Xcb2';
%Xcbt3=Xcb3';
%Xcbt4=Xcb4';
%Xcbt5=Xcb5';

%_________________________________________________________________________________________
%Donnees projetes dans les axes de l'ACP
new_don = donnee * PC;

Xdon1 = new_don(:,arg_acpplan1);
Xdon2 = new_don(:,arg_acpplan2);
%Xdon3 = new_don(:,3);
%Xdon4 = new_don(:,4);
%Xdon5 = new_don(:,5);
       
if (arg_acpplan1<cmy & arg_acpplan2<=cmy & arg_acpplan2>arg_acpplan1)

figure;    

hold on
for i=1:(taille_carte)
    suivi=0;
       couleur=MyData3(i);
       text(Xcb1(i)+0.005,Xcb2(i)+0.005,[' ' num2str(i) '-' num2str(couleur) ''],'Color',U(couleur,:))
       plot(Xcb1(i),Xcb2(i),'o','MarkerFaceColor',U(couleur,:))
       point_text=find(MyData2==i-1);
       for j=1:length(point_text)   
       suivi=suivi+1;
       %text(Xdon1(point_text(j))+0.5,Xdon2(point_text(j))+0.5,num2str(i))   
       plot(Xdon1(point_text(j)),Xdon2(point_text(j)),'*','MarkerEdgeColor',U(couleur,:));
       end
end
plot(Xcb1,Xcb2,Xcbt1,Xcbt2,'b')
set(gcf,'windowbuttondownfcn','ecris_freq');  


else message='Entrer des valeurs valables pour les plans de l''ACP';
    title='Attention';
    msgbox(message,title,'warn')

end


      