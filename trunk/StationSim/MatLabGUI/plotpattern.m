%
% Draw the plots of the pattern
%
% The pattern to be draws is raw, straight from the array without post-processing.
%
function plotpattern
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
% Adapted for LOFAR StationSim
% P.C. Broekema july 2002
  
      

      dirpath = 'data';
      load([dirpath '\beam_pattern.mat']);
      load([dirpath '\antenna_signals.mat']);
      load([dirpath '\antenna_config.mat']);
      load([dirpath '\output_options.mat']);
      patend=90;
      patstep=1;
      
      a=[-1*patend:patstep:patend]*pi/180;
      b=[-1*patend:patstep:patend]*pi/180;
  %
  % contour using angles coordinates. Beam top view
  %
  
if beam_top
      figure(2);
      imagesc(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4));
      hold on;
      xlabel('Azimuth (degrees)')
      ylabel('Elevation (degrees)')
      title('Top View of Beam Pattern')
      %set(gca,'ButtonDownFcn','arraygene(''separateimage'')');
      %set(gca,'Tag','image')  % restore the value

      %blah = findobj(fighndl,'Tag','contour');
      %axes(blah) 
  %
  % contour using angles coordinates. 3 DB contour top view
  %
end
if beam_contour  
      cont = [-3 -3];
      figure;
      if isempty(cont)
         cs=contour(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4),[0 -3 -6 -12 -24 -48]);
      else
         cs=contour(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4),cont);
         clabel(cs), grid
      end
      xlabel('Azimuth (degrees)')
      ylabel('Elevation (degrees)')
      title('Contours of Beam Pattern')
      %set(gca,'ButtonDownFcn','arraygene(''separate'')');
      %set(gca,'Tag','contour')  % restore the value
end;
  %
  % Side view using angles coordinates. 
  %    
if beam_side
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
  %
  % 3D beam representation in angles coordinates. 
  %   
  if beam_3d
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
      title('3d view of beam pattern');     
      %set(gca,'ButtonDownFcn','arraygene(''separate3d'')');
      %set(gca,'Tag','3dplot')  % restore the value
  end;