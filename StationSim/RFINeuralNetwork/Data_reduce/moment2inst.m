function [sortie]=moment2(Variable,mmi1,resu);
%Variable=DSP;
li=0;
col=0;
sortie=[];
sortie1=[];
kurtosis=[];
skewness=[];
[li,col]=size(Variable)
 for i=1:li
        Z=resu{i};
        moy0=mean(Z/mmi1(i));
        var0=std(Z/mmi1(i));
        Y=Variable(i,:);
        moy=mean(Y);
        var=std(Y);
        median0=median(Z);
        median1=median(Y);
        skew=power(((Y(:)-moy)/var),3);        
        skewness=mean(skew);
        kurt=power(((Y(:)-moy)/var),4);       
        kurtosis=mean(kurt)-3;
        min0=min(Z/mmi1(i));
        max0=max(Z/mmi1(i));
        min1=min(Y);
        max1=max(Y);
        sortie1=[moy0/(var0),moy/(var),skewness,kurtosis,moy,median0,median1,min1,max0,max1];
        sortie=[sortie;sortie1];
 end

%dim=4
%id=fopen('sauvegarde1.txt','w');
%fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0}\n',dim,li);
%fprintf(fid,'%6.6e %6.6e %6.6e %6.6e %6.6e\n',sortie');
%status=fclose(fid);

    