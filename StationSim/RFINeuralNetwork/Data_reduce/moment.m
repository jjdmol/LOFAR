%function [sortie]=moment(Variable);
Variable=DSP;
li=0;
col=0;
sortie=[];
sortie1=[];
kurtosis=[];
skewness=[];
[li,col]=size(Variable)

        for i=1:li
        Y=Variable(i,:);
        moy=mean(Y);
        var=power(std(Y),2);
        skew=power(((Y(:)-moy)/sqrt(var)),3);        
        skewness=mean(skew);
        kurt=power(((Y(:)-moy)/sqrt(var)),4);       
        kurtosis=mean(kurt)-3;
        sortie1=[moy,var,skewness,kurtosis];
        sortie=[sortie;sortie1];
        end

dim=4
fid=fopen('sauvegarde1.txt','w');
fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0}\n',dim,li);
fprintf(fid,'%6.6e %6.6e %6.6e %6.6e\n',sortie');
status=fclose(fid);

    