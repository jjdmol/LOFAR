y=ceil(sqrt(NbClasse));                  
x=ceil(NbClasse/y);
nbrli=sizeMap(1);
nbrcol=sizeMap(2);

t=0;
for i=1:nbrli
    for j=1:nbrcol
        t=t+1;
        freq=find(MyData2==t-1);
        referent=DSP(freq*echantillonnage,:);
        subplot(x,y,i)
        plot(referent)
    end
end

        
