figure 
moy2=moy;
plot(moy)
resultat=[];
trouve=find(moy2==0);
for i=1:length(trouve) 
    a=trouve(i);
    av=a+1;
    der=a-1;
    if ((moy(av))==0)
        while(moy(av)==0)
            av=av+1
        end
    end
    if ((moy2(der))==0)
        while((moy2der)==0)
            der=der+1
        end
    end
    resu=[moy2(der);moy2(av)];
    moy2(trouve(i))=mean(resu);
    resultat=[resultat moy2(trouve(i))];
end
figure
plot(moy2)
hold on
plot(trouve,resultat,'*r')
hold off
   
