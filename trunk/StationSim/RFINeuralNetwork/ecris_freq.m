global echantillonnage;


pos=get(gca,'CurrentPoint')
col=pos(1,1);
lin=pos(1,2);
V=colormap(hsv(length(MyData3)));
    gx=find(abs(Xcb1-pos(1,1))<0.05);
    gy=find(abs(Xcb2-pos(1,2))<0.05);
    for lh=1:length(gx)
        for gk=1:length(gy)
        if (gx(lh)==gy(gk) & gx~=[] & gy~=[] )
    disp(['neurone(' num2str(gx(lh)) ') ']);
    point_choix=find(MyData2==gx(lh)-1);
    if (point_choix==[])
            message='neurone non activé';
    title='Attention';
    msgbox(message,title,'warn')
    else 
    for j=1:length(point_choix)
    plot(Xdon1(point_choix(j)),Xdon2(point_choix(j)),'*','MarkerEdgeColor',V(gx(lh),:))    
    text(Xdon1(point_choix(j))+0.005,Xdon2(point_choix(j))+0.005,num2str(point_choix(j)*echantillonnage),'Color',V(gx(lh),:)) 
    end
end
end
end
end

