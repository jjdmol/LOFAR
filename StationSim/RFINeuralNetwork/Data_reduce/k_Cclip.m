
function [resu]=k_Cclip(X)
  [liX,colX]=size(X)
  for i=1:colX
    A=X(:,i);
    i
    t=0;
    nbr=0;
    tres=[];
    while(nbr~=1)        
         t=t+1;
         av=length(A);
         moy=mean(A);         
         ecart=std(A);       
         g=(A<2.5*ecart+moy);
         A=A(find(g==1));
         der=length(A);
         % if (der==0)
         % A=['rien'];  
         if (der-av==0)
             tres=[tres t];
             moy=[moy mean(A)];
             resu{i}=A;
             nbr=1;
         end
   end
end
    
%r=0;
%enf=[];
%for i=1:colX
 
%if (length(resu{i})==0)
 %       r=r+1;
    %    enf=[enf i];
    % end
 %   e%nd