function [fin]=my_reshape(OutputSignal)
fin=[];
int=[];
[li,co,prof]=size(OutputSignal);
for i=1:prof
    int=OutputSignal(:,:,i);
    fin=[fin;int'];
    clear OutputSignal(:,:,i);
end
