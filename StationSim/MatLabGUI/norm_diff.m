%function [distances]=norm_diff(EvectorEVD);
t=0;
diff=[];
Evector=tre(:,1:8);
for i=1:size(Evector,2)
    for j=i+1:size(Evector,2)
        t=t+1;
         diff(:,t)=norm(Evector(:,i)-Evector(:,j));
    end
end

