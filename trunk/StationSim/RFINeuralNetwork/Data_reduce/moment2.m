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
        moy0=mean(Z)/mmi1(i);
        var0=std(Z)/mmi1(i);
        Y=Variable(i,:);
        moy=mean(Y);
        var=std(Y);
        skew=power(((Y(:)-moy)/var),3);        
        skewness=mean(skew);
        kurt=power(((Y(:)-moy)/var),4);       
        kurtosis=mean(kurt)-3;
        sortie1=[moy0/(var0*sqrt(63)),moy/(var*sqrt(63)),skewness,kurtosis,moy];
        sortie=[sortie;sortie1];
 end

%dim=4
%id=fopen('sauvegarde1.txt','w');
%fprintf(fid,'# setDim=%d setSize=%d setStat={0.0,0.0,0.0,0.0}\n',dim,li);
%fprintf(fid,'%6.6e %6.6e %6.6e %6.6e %6.6e\n',sortie');
%status=fclose(fid);

    