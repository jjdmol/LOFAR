%
%
% Draw the plots of the pattern
%
function plotpatternrfi
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
    dirpath = 'data';
    patend=90;
    patstep=1;
    
    load([dirpath '\beam_pattern.mat']);
    load([dirpath '\signal_options']);
    load([dirpath '\antenna_signals.mat']);
    load([dirpath '\antenna_config.mat']);
    load([dirpath '\rfi_eigen.mat']);
    load([dirpath '\output_options.mat']);
   
    
    a=[-1*patend:patstep:patend]*pi/180;
    b=[-1*patend:patstep:patend]*pi/180;
    rfi_phi*180/pi;
    rfi_theta*180/pi;
    
    if ad_beam_top
        figure;
      
        %imagesc(a*180/pi,(acos(sin(b)))*180/pi,20*log10(pat+1e-4));
        imagesc(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4));
        hold on;
        plot3(rfi_theta*180/pi,rfi_phi*180/pi,[1:rfi_number],'o')

      
       
        %      plot3(thetaRFI*180/pi,acos(sin(phiRFI))*180/pi,[1:nrfi],'o')
        xlabel('Azimuth (degrees)')
        ylabel('Elevation (degrees)')
        title('Top View of Beam Pattern')
        %set(gca,'ButtonDownFcn','arraygene(''separateimage'')');
        %set(gca,'Tag','image')  % restore the value

        %blah = findobj(fighndl,'Tag','contour');
       %axes(blah) 
        hold off;
    end
    if ad_beam_contour
        figure;
        cont = [-3 -3];
        if isempty(cont)
            cs=contour(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4),[0 -3 -6 -12 -24 -48]);
        else
            cs=contour(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4),cont);
            clabel(cs), grid
        end
        hold on;
        plot3(rfi_theta*180/pi,-rfi_phi*180/pi,[1:rfi_number],'o')
        xlabel('Azimuth (degrees)')
        ylabel('Elevation (degrees)')
        title('Contours of Beam Pattern')
        %set(gca,'ButtonDownFcn','arraygene(''separate'')');
        %set(gca,'Tag','contour')  % restore the value
        hold off;
    end;
      
    if ad_beam_side
        figure;
        plot(a*180/pi,20*log10(abs(BeamPattern)))
        ylim([-20 0]), 
        xlim([-1*patend patend]), grid
        xlabel('Angle (degrees)')
        ylabel('Power (dB)')
        title('Side View of Beam Pattern')
        %set(gca,'ButtonDownFcn','arraygene(''separate'')');
        %set(gca,'Tag','1dproj')  % restore the value
    end;
    if ad_beam_3d
        figure;
        %blah = findobj(fighndl,'Tag','3dplot');
        %axes(blah), cla
        aa = ones(length(a),1)*a;
        bb = ones(length(b),1)*b;
        aa = aa.';
        r = 1 + BeamPattern;
        u = r .* cos(bb) .* sin(aa);
        v = r .* sin(bb);
        w = r .* cos(aa) .* cos(bb);
        surface(u,v,w,sqrt(u.^2+v.^2+w.^2))
        xlim([-2 2]);
        ylim([-2 2]);
        view(0,0)
        shading interp
        set(gca,'color',[.8 .8 .8]);
        set(gca,'xcolor',[.8 .8 .8]);
        set(gca,'ycolor',[.8 .8 .8]);
        set(gca,'zcolor',[.8 .8 .8]);
        %set(gca,'ButtonDownFcn','arraygene(''separate3d'')');
        %set(gca,'Tag','3dplot')  % restore the value
    end;