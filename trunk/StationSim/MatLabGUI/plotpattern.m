%
% Draw the plots of the pattern
%
% The pattern to be draws is raw, straight from the array without post-processing.
%
function plotpattern(px,py,dirpath)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
% Adapted for LOFAR StationSim
% P.C. Broekema july 2002

      load([dirpath 'data\beam_pattern.mat']);
      load([dirpath 'data\output_options.mat']);
      u=[0:N];
      
     
      if beam_contour
    % Beamshape using angles coordinates. Beam top view
      figure(5)
      subplot(1,3,1)
      imagesc(a,b,20*log10(Rect_pattern))
      xlabel('Theta')
      ylabel('Phi')
      title('Top View of steered Beam Pattern in angle coordinate')
      end
      
      if beam_top
      % Beamshape : Top view using cartesian coordinate
      figure(6);
%       cur_scr = get(s,'Position');
%       root_scr = get(0,'Screensize');
%       set(s,'Position',[root_scr(1)+10, cur_scr(2)*9/10, root_scr(3)*9/10, cur_scr(4)*9/10]);
      subplot(1,3,1)
      imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
      caxis([-70 0]);
      title('Top View of Beam Pattern in cartesian coordinates')
      %set(gca,'ButtonDownFcn','arraygene(''separateimage'')');
      %set(gca,'Tag','image')  % restore the value
      Xlabel('U normalised')
      Ylabel('V normalised')
      %blah = findobj(fighndl,'Tag','contour');
      %axes(blah) 
   
      end
  
      if beam_side
      % contour using angles coordinates. 
      figure(7);
      %set(s2,'Position',[root_scr(1)+10, cur_scr(2)*9/10, root_scr(3)*9/10, cur_scr(4)*9/10]);
      subplot(1,3,1)
      plot(b*180/pi,20*log10(abs(Rect_pattern.')))
      ylim([-70 0]),
      %xlim([0 patend]), grid
      xlabel('Angle (degrees)')
      ylabel('Power (dB)')
      title('Side View of Beam Pattern')
      hold off
      %set(gca,'ButtonDownFcn','arraygene(''separate'')');
      %set(gca,'Tag','1dproj')  % restore the value
      end

      if beam_3d
      % 3D beam representation in angles coordinates.     
      figure(8)
      subplot(1,3,1)
      aa = ones(length(a),1)*a;
      bb = ones(length(b),1)*b;
      aa = aa.';
      r = 1 + Rect_pattern;
      u = r .* sin(bb).* cos(aa);
      v = r .* sin(bb).* sin(aa) ;
      w = r .* cos(bb);
      
      %Alliot et Hampson Coordinate
      %u = r .* cos(bb) .* sin(aa);
      %v = r .* sin(bb);
      %w = r .* cos(aa) .* cos(bb);

      surface(u,v,w,sqrt(u.^2+v.^2+w.^2))
      xlim([-2 2]);
      ylim([-2 2]);
      view(0,0)
      shading interp
      set(gca,'color',[.8 .8 .8]);
      set(gca,'xcolor',[.8 .8 .8]);
      set(gca,'ycolor',[.8 .8 .8]);
      set(gca,'zcolor',[.8 .8 .8]);
      title('3d view of Beam Pattern');     
      %set(gca,'ButtonDownFcn','arraygene(''separate3d'')');
      %set(gca,'Tag','3dplot')  % restore the value
      end