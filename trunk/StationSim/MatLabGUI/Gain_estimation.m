 %Gain estimation method :
Number_antenna=8;
Dnoise=rand(Number_antenna,1);
g=[1,2,3,18,5,6,7,8];
Dnoise=rand(length(g),length(g));
R=g*g'+Dnoise;
test=[];
fin=0;
i=0
while(fin==0)
i=i+1
if (i==1)
D=diag(R)*ones(1,Number_antenna).*eye(Number_antenna,Number_antenna)
Gdec=R-D;
else Rest=Gestimate*Gestimate';
     D=diag(Rest)*ones(1,Number_antenna).*eye(Number_antenna,Number_antenna);
     Gdec=R-D;
end
[eig_vector,eig_value]=eig(Gdec);
interm=[diag(eig_value) eig_vector];
interm=sortrows(interm,1)
Gestimate=interm(size(interm,1),2:size(interm,2)).';
test=[test;interm(size(interm,1),2:size(interm,2))];
if i>2
if test(:,i-2)-test(:,i)<1e-13
    fin=1;
end
end
end
R
Gcomp=[Gestimate g.']
Dnoise_fin=(diag(R-Gdec))
Noise_est=[diag(Dnoise) Dnoise_fin]
