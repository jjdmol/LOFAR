
DSP3=DSP1;
[lig,col]=size(DSP3)

for tot=1:lig
 tot   
trouve=find(DSP3(tot,:)==0);
for i=1:length(trouve) 
    a=trouve(i);
    av=a+1;
    der=a-1;
    if ((DSP3(tot,av))==0)
        while(DSP3(tot,av)==0)
            av=av+1;
        end
    end
    if ((DSP3(tot,der))==0)
        while(DSP3(tot,der)==0)
            der=der+1;
        end
    end
    resu=[DSP3(tot,der);DSP3(tot,av)];
    DSP3(tot,trouve(i))=mean(resu);
end
end

figure
imagesc(DSP3');
colormap(1-gray)   
