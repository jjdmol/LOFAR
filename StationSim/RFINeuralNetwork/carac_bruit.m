[li,col]=size(DSP_Dmoy);
DSP=DSP_Dmoy;
balad=[];
bruit=[];

%On prend pour toutes les fréquences.
for i=1:col
  balad=min(DSP(:,i));
  %balad1=sort(balad);
  %choix=On prend 2% des minima.
  %choix=ceil(0.02*li)-1;
  %balad2=(1:choix);
  bruit=[bruit balad];
end
plot(10*log10(bruit))

X=10*log10(DSP_Dmoy);
k=1;
moy=[];
med=[];
resu=cell(1,col);
    %moy=mean(DSP(:,i));
    %ecart=std(DSP(:,i));
      
    av=-1;
    der=-2;
    tres=[];
    for i=1:col
    A=X(:,i);
    i
    t=0;
    nbr=0;
    while(nbr~=1)        
         t=t+1
         av=length(A)
         moyenne=mean(A);         
         ecart=std(A);       
         g=(A<2.5*ecart+moyenne);
         A=A(find(g==1));
         der=length(A)
         % if (der==0)
         % A=['rien'];  
         if (der-av==0)
             tres=[tres t];
             med=[med median(A)]
             moy=[moy moye];
             resu{i}=A;
             nbr=1;
         end
   end
end
    
