y=ceil(sqrt(max(MyData3));                  
x=ceil(MyData3/y);
figure;
t=0;
for i=1:max(MyData3)
        t=t+1;
        neurone_classe=find(MyData3==t);
        for j=1:length(neurone_classe)       
        donnee_select=find(MyData2==neurone_classe(j)-1);
        donnee_selection=[donnee_selection;donnee_select];
        end
        referent=10*log10(mean(DSP(donnee_selection*echantillonnage,:)));
        subplot(x,y,t)
        plot(referent)
        title(['classe ' num2str(t) '']);
    end
end

        %donnee_ben{i}=donnee_selection;
        