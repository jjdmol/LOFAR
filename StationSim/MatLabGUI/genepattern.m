% Generate a beam pattern according to the selected antenna configuration 
% and antenna signals

function genepattern
 
      dirpath = 'data';
      
      load([dirpath '\antenna_signals.mat'])
      load([dirpath '\antenna_config.mat'])

      %
      % Generate thea tile beam pattern
      %
      sc=.5;
      xx = [ 0:1:7 ];  
      yy = ones(1,8);
      
      %
      % Generate Digital beamforming window 
      %
      % window type and size is currently hardcoded..
      % Needs to be variable?
      digwin = taper(px, py, 1, 128);
 
      %
      % Generate beam pattern of the array
      %
      
      % hard code some variables. To be validated.
      patend  = 90;
      patstep = 1;
      relfreq = 1;

      
      a=[-1*patend:patstep:patend]*pi/180;
      b=[-1*patend:patstep:patend]*pi/180;
      pat = zeros(length(a),length(b)); % preallocate to speed up
      for j=1:length(a)
         for k=1:length(b)
            pat(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*sin(a(j))*cos(b(k))+py*sin(b(k))))*digwin;
         end
      end
      BeamPattern = abs(pat)/max(max(abs(pat)));
      save([dirpath '\beam_pattern.mat'],'digwin','BeamPattern');