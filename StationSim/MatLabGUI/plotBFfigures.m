function plotBFfigures
% This file plots a number of figures designed to visualize the performance 
% of the beamformer

% Chris Broekema, july 2002


    dirpath='data';
    load([dirpath '\rfi_eigen.mat']);
    load([dirpath '\antenna_signals.mat']);
    load([dirpath '\signal_options.mat']);
    load([dirpath '\output_options.mat']);


    if (bf_power)
        fprintf('\tPower spectra of signals before and after beamforming\n');
        for a=1:NumberOfAntennas
            PowerAnt(a,:)=AntennaSignals(a,:).*conj(AntennaSignals(a,:));
            PowerBF(a,:)=BFSignals(a,:).*conj(BFSignals(a,:));
            PowerWght(a,:)=WeightVector(a,:).*conj(WeightVector(a,:));
        end


        figure;
        subplot(4,1,1),plot(PowerAnt);
        title('Power spectrum of input signal');
    %    subplot(3,1,2),plot(PowerWght);
        subplot(4,1,2),plot(abs(WeightVector));
        title('Real component of Weight vector');
        subplot(4,1,3),plot(imag(WeightVector));
        title('Imaginary component of Weight vector');
        subplot(4,1,4),plot(PowerBF);
        title('Power spectrum of beamformed signal');
    end
  
    
    if (bf_side | bf_3dplot)
        genebeampattern;
        load([dirpath '\beam_pattern.mat']);
        patend=90;
        patstep=1;
      
        a=[-1*patend:patstep:patend]*pi/180;
        b=[-1*patend:patstep:patend]*pi/180;
    end  
 
    if (bf_3dplot)
        fprintf('\t3D plot of weighted beamshape\n');

        figure
        aa = ones(length(a),1)*a;
        bb = ones(length(b),1)*b;
        aa = aa.';
        r = 1 + BeamSignals;
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
    end
    
    if (bf_side)
        fprintf('\tSide view beamformed signal\n');
        figure
        plot(a*180/pi,20*log10(BeamSignals))
        ylim([-20 0]), 
        xlim([-1*patend patend]), grid
        xlabel('Angle (degrees)')
        ylabel('Power (dB)')
        title('Side View of Beam Pattern')
        %set(gca,'ButtonDownFcn','arraygene(''separate'')');
        %set(gca,'Tag','1dproj')  % restore the value
    end;