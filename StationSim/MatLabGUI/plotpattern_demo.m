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
      load([dirpath '/beam_pattern.mat']);
      load([dirpath '/antenna_signals.mat']);
      load([dirpath '/antenna_config.mat']);
      load([dirpath '/output_options.mat']);
      patend=90;

      
      a=[-1*patend:patstep:patend]*pi/180;
      b=[-1*patend:patstep:patend]*pi/180;
  %
  % contour using angles coordinates. Beam top view
  %
  
if beam_top
    
    
      blah = findobj('Tag','Figure1Holder');
      axes(blah)
      %s = subplot(2,3,1);
      set(blah,'NextPlot','Replace');

      imagesc(a*180/pi,b*180/pi,20*log10(BeamPattern+1e-4));
      hold on;
      xlabel('Azimuth (degrees)')
%      ylabel('Elevation (degrees)')
      title('Top View of Beam Pattern')
      %set(gca,'ButtonDownFcn','arraygene(''separateimage'')');
      %set(gca,'Tag','image')  % restore the value

      %blah = findobj(fighndl,'Tag','contour');
      %axes(blah) 
      set(gca, 'Visible','on');
end
  %
  % 3D beam representation in angles coordinates. 
  %   
if beam_3d

    blah = findobj('Tag','Figure2Holder');
    axes(blah)
    %s = subplot(2,3,4);
    set(blah,'NextPlot','Replace');
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
    %set(gca,'ButtonDownFcn','arraygene(''separate3d'')');
    %set(gca,'Tag','3dplot')  % restore the value
    set(gca, 'Visible','on');

end;