global MyData3;
global U;
global fichier;

if (length(findstr('PRSOM',file_name)>0) )
      figure('Name','Carte PRSOM')
  elseif length(findstr('SOM',file_name))>0;
      figure('Name','Carte SOM')
else figure('Name','No Match')
end


global NbClasse;
nbrli=sizeMap(1);
nbrcol=sizeMap(2);
t=0;
for i=1:nbrli
    for j=1:nbrcol
        t=t+1;
        couleur=MyData3(t);
        Card=find(MyData2==t-1);
p_btn = uicontrol(gcf,...
                  'style', 'pushbutton',...
                 'Position',[10*i*6.1-40 nbrcol*3.2*10-10*j*3 60 20 ],...
                 'String', ['' num2str(t) ',N:' num2str(length(Card)) ',Cl:' num2str(MyData3(t)) ''],...
                 'CallBack',[ 'reader1 (' num2str(t-1) ')'],'BackgroundColor',[U(couleur,:)]);
             
         end
     end
     
%sliders = uicontrol(gcf,...
 %                'style','slider');