function blah

    dirpath = 'data';
    patend=90;
    patstep=1;
    
    load([dirpath '/beam_pattern.mat']);
    load([dirpath '/signal_options']);
    load([dirpath '/antenna_signals.mat']);
    load([dirpath '/antenna_config.mat']);
    load([dirpath '/rfi_eigen.mat']);
    load([dirpath '/output_options.mat']);

    figure;
    
    a=[-1*patend:patstep:patend]*pi/180;
    b=[-1*patend:patstep:patend]*pi/180;
    pat = zeros(length(a),length(b)); % preallocate to speed up
            
    r = sqrt(px(1)^2 + py(1)^2);

    for j=1:length(a)
        for k=1:length(b)
            pat(j,k) = exp(-1*sqrt(-1)*2*pi* ...
                      (px*sin(a(j))*cos(b(k))+py*sin(b(k))))*LookingDirection;
%             for m=1:NumberOfAntennas
%                 pat(j,k)=pat(j,k) + r * sin(b(k)) * cos(a(j) - m*2*pi/(NumberOfAntennas-1));
%             end;
        end;
    end;    
    %contour(a*180/pi,b*180/pi,20*log10(pat+1e-4));
    surface(abs(pat));      
    xlim([0 180]);
    ylim([0 180]);

    xlabel('Azimuth (degrees)')
    ylabel('Elevation (degrees)')
    title('Contour plot showing theoretical position of grating lobes');
    shading interp
    set(gca,'color',[.8 .8 .8]);
    set(gca,'xcolor',[.8 .8 .8]);
    set(gca,'ycolor',[.8 .8 .8]);
    set(gca,'zcolor',[.8 .8 .8]);
  
    hold off; 
