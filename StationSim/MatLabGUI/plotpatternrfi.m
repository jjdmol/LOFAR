%
%
% Draw the plots of the pattern
%
function plotpatternrfi(dirpath,px,py,Phi_theta,position_start,position_stop)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
% Expanded     : Chris Broekema (sept 2002)

    
    load([dirpath 'data/beam_pattern.mat']);
    load([dirpath 'data/antenna_signals.mat']);
    load([dirpath 'data/rfi_eigen.mat']);
    load([dirpath 'data/output_options.mat']);
    u=[0:N];
    color=hsv(size(Phi_theta,3));

    if ad_beam_contour  
        
    figure(5)
    %Plot steered pattern in angle coordinate
    subplot(1,3,2)
    imagesc(a,b,20*log10(Rect_pattern_steered))
    xlabel('Theta')
    ylabel('Phi')
    title('Top View of steered Beam Pattern in angle coordinate')
    hold on
    for i =1:size(Phi_theta,3)
    plot(Phi_theta(2,position_start:position_stop,i),Phi_theta(1,position_start:position_stop,i),'*','MarkerFaceColor',color(i,:))
    end
    caxis([-70 0]);
    hold off
    
    %Plot pattern nulled in angle coordinate
    subplot(1,3,3)
    imagesc(a,b,20*log10(Rect_pattern_null))
    xlabel('Theta')
    ylabel('Phi')
    title('Top View of adapted Beam Pattern in angle coordinates')
    hold on
    for i =1:size(Phi_theta,3)
    plot(Phi_theta(2,position_start:position_stop,i),Phi_theta(1,position_start:position_stop,i),'*','MarkerFaceColor',color(i,:))
    end
    caxis([-70 0]);
    hold off
    end
    
    if ad_beam_top
        
    figure(6);
    %Plot steered pattern in cartesian coordinate
    subplot(1,3,2)
    imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(SteeredPattern));
    caxis([-70 0]);
    hold on;
    %plot3(sin(rfi_theta*180/pi)*cos(rfi_phi*180/pi),sin(rfi_theta*180/pi)*sin(rfi_phi*180/pi),[1:rfi_number],'o')
    %plot3(sin(look_dir_theta*180/pi)*cos(look_dir_phi*180/pi),sin(look_dir_theta*180/pi)*sin(look_dir_phi*180/pi),1,'y*')  
    %squint_factor=1/(signal_freq+0.5);
    %plot(squint_factor*sin(look_dir_theta*180/pi)*cos(look_dir_phi*180/pi),squint_factor*sin(look_dir_theta*180/pi)*sin(look_dir_phi*180/pi),'y+')
    xlabel('U normalised')
    ylabel('V normalised')
    title('Top View of steered Beam Pattern in cartesian coordinates')
    for i =1:size(Phi_theta,3)
    plot(cos(Phi_theta(1,position_start:position_stop,i)).*sin(Phi_theta(2,position_start:position_stop,i)),sin(Phi_theta(2,position_start:position_stop,i)).*sin(Phi_theta(1,position_start:position_stop,i)),'*','MarkerFaceColor',color(i,:))
    end
    hold off;
    
    %Plot pattern nulled in cartesian coordinate
    subplot(1,3,3)
    imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
    hold on;
    caxis([-70 0]);
    for i =1:size(Phi_theta,3)
    plot(cos(Phi_theta(1,position_start:position_stop,i)).*sin(Phi_theta(2,position_start:position_stop,i)),sin(Phi_theta(2,position_start:position_stop,i)).*sin(Phi_theta(1,position_start:position_stop,i)),'*','MarkerFaceColor',color(i,:))
    end
    %plot3(rfi_theta*180/pi,rfi_phi*180/pi,[1:rfi_number],'o')
    %plot3(look_dir_theta*180/pi,look_dir_phi*180/pi,1,'y*')
    %plot(squint_factor*sin(look_dir_theta*180/pi)*cos(look_dir_phi*180/pi),squint_factor*sin(look_dir_theta*180/pi)*sin(look_dir_phi*180/pi),'y+')
    xlabel('U normalised')
    ylabel('V normalised')
    title('Top View of adapted Beam Pattern in cartesian coordinates')
    hold off;
    end    

    if ad_beam_side
    %Plot side view of the steered pattern in angle coordinate   
    figure(7);
    subplot(1,3,2)
    plot(b*180/pi,20*log10(abs(Rect_pattern_steered.')))
    ylim([-70 2]), 
    %xlim([0 patend]), grid
    xlabel('Angle (degrees)')
    ylabel('Power (dB)')
    title('Side View of steered Beam Pattern')   
    
    %Plot side view of the adapted pattern in angle coordinate  
    subplot(1,3,3)
    plot(b*180/pi,20*log10(abs(Rect_pattern_null.')))
    ylim([-70 2]), 
    % xlim([0 patend]), grid
    xlabel('Angle (degrees)')
    ylabel('Power (dB)')
    title('Side View of nulled Beam Pattern')    
    end

    if ad_beam_3d 
        
    figure(8);
    %3D view of the steered pattern 
    subplot(1,3,2)
    aa = ones(length(a),1)*a;
    bb = ones(length(b),1)*b;
    aa = aa.';
    r = 1 + Rect_pattern_steered;
    u = r .* cos(aa) .* sin(bb);
    w = r .* cos(bb);
    v = r .* sin(aa) .* sin(bb);
    surface(u,v,w,sqrt(u.^2+v.^2+w.^2))
    xlim([-2 2]);
    ylim([-2 2]);
    view(0,0)
    shading interp
    set(gca,'color',[.8 .8 .8]);
    set(gca,'xcolor',[.8 .8 .8]);
    set(gca,'ycolor',[.8 .8 .8]);
    set(gca,'zcolor',[.8 .8 .8]);
    title('3d view of steered Beam Pattern')
    
    %3D view of the adapted pattern
    subplot(1,3,3)
    aa = ones(length(a),1)*a;
    bb = ones(length(b),1)*b;
    aa = aa.';
    r = 1 + Rect_pattern_null;
    u = r .* cos(aa) .* sin(bb);
    w = r .* cos(bb);
    v = r .* sin(aa) .* sin(bb);
    surface(u,v,w,sqrt(u.^2+v.^2+w.^2))
    xlim([-2 2]);
    ylim([-2 2]);
    view(0,0)
    shading interp
    set(gca,'color',[.8 .8 .8]);
    set(gca,'xcolor',[.8 .8 .8]);
    set(gca,'ycolor',[.8 .8 .8]);
    set(gca,'zcolor',[.8 .8 .8]);
    title('3d view of nulled Beam Pattern');
    end
    
    