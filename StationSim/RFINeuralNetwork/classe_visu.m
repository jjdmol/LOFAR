global MyData3;
global U;
global fichier;
global donnee_ben;
if (length(findstr('PRSOM',file_name)>0) )
      figure('Name','Carte PRSOM')
  elseif length(findstr('SOM',file_name))>0;
   
      figure('Name','Carte SOM')
else figure('Name','No Match')
end
donnee_selection=[];

global NbClasse;
nbrli=sizeMap(1);
nbrcol=sizeMap(2);
t=0;
donnee_ben=cell(NbClasse,1);
for i=1:NbClasse
        neurone_classe=find(MyData3==i);
        donnee_selection=[];
        for j=1:length(neurone_classe)       
        donnee_select=find(MyData2==neurone_classe(j)-1);
        donnee_selection=[donnee_selection;donnee_select];
        end
        donnee_ben{i}=donnee_selection;
p_btn = uicontrol(gcf,...
                  'style', 'pushbutton',...
                 'Position',[10 23*i 80 20 ],...
                 'String', ['Classe ' num2str(i) ',N:'num2str(length(donnee_selection)) ''],...
                 'CallBack',[ 'reader2 (' num2str(i) ')'],'BackgroundColor',[U(i,:)])
     end
     
%sliders = uicontrol(gcf,...
 %                'style','slider');