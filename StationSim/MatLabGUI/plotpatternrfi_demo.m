%
%
% Draw the plots of the pattern
%
function plotpatternrfi (pos)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
% Expanded     : Chris Broekema (sept 2002)
    dirpath = 'data';
    patend=90;

    
    load([dirpath '/beam_pattern.mat']);
    load([dirpath '/signal_options']);
    load([dirpath '/antenna_signals.mat']);
    load([dirpath '/antenna_config.mat']);
    load([dirpath '/rfi_eigen.mat']);
    load([dirpath '/output_options.mat']);
   
    
    a=[-1*patend:patstep:patend]*pi/180;
    b=[-1*patend:patstep:patend]*pi/180;
    rfi_phi*180/pi;
    rfi_theta*180/pi;
    
    if ad_beam_top
       if (pos == 1)
            blah = findobj('Tag','Figure3Holder');
            axes(blah)
        else
            blah = findobj('Tag','Figure5Holder');
            axes(blah)
        end;
        set(blah,'NextPlot','Replace');
        %imagesc(a*180/pi,(acos(sin(b)))*180/pi,20*log10(pat+1e-4));
        imagesc(a*180/pi,-b*180/pi,20*log10(BeamPattern+1e-4));
        hold on;
        plot3(-rfi_theta*180/pi,rfi_phi*180/pi,[1:rfi_number],'o')

        %      plot3(thetaRFI*180/pi,acos(sin(phiRFI))*180/pi,[1:nrfi],'o')
        xlabel('Azimuth (degrees)')
        ylabel('Elevation (degrees)')
        title('Top View of Beam Pattern')
        %set(gca,'ButtonDownFcn','arraygene(''separateimage'')');
        %set(gca,'Tag','image')  % restore the value

        %blah = findobj(fighndl,'Tag','contour');
       %axes(blah) 
        hold off;
        set(gca,'Visible','on');
    end

    if ad_beam_3d
        if (pos == 1) 
            blah = findobj('Tag','Figure4Holder');
            axes(blah)
        else
            blah = findobj('Tag','Figure6Holder');
            axes(blah)
        end
        set(blah,'NextPlot','Replace');
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
        view(90,-45)
        shading interp
        set(gca,'color',[.8 .8 .8]);
        set(gca,'xcolor',[.8 .8 .8]);
        set(gca,'ycolor',[.8 .8 .8]);
        set(gca,'zcolor',[.8 .8 .8]);
        title('3d view of beam pattern');
        set(gca, 'Visible','on');
        %set(gca,'ButtonDownFcn','arraygene(''separate3d'')');
        %set(gca,'Tag','3dplot')  % restore the value
        hold off;
    end;