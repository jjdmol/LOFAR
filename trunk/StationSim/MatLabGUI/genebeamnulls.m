% plot the beam in the direction of looking 
% the RFI are nulls after detection with the eigenvector deconposition
% level is a treshold for detection of the eigenvalues more than > LEVEL 
function genebeamnull
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001

    dirpath = 'data';
      
    load([dirpath '\antenna_signals.mat']);
    load([dirpath '\antenna_config.mat']);
    load([dirpath '\rfi_eigen.mat']);
    load([dirpath '\signal_options']);
    %
    % Generate thea tile beam pattern
    %
    sc=.5;
    xx = [ 0:1:7 ];  
    yy = ones(1,8);

    
    digwindow=1;
    patend=90;
    patstep=1;
    relfreq=1;
    
    %
    % Generate Digital beamforming window 
    %
    digwin = taper(px, py, digwindow, 128);
    
    size(digwin);
 
    if NullGrid
        LookingDirection  = steerv(px,py,look_dir_phi,look_dir_theta);  
    end;
    %
    % Generate beam pattern of the array
    %
    a=[-1*patend:patstep:patend]*pi/180;
    b=[-1*patend:patstep:patend]*pi/180;
    pat = zeros(length(a),length(b)); % preallocate to speed up
    for j=1:length(a)
         for k=1:length(b)
            pat(j,k) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*sin(a(j))*cos(b(k))+py*sin(b(k))))*LookingDirection;
         end
    end
    BeamPattern = abs(pat)/max(max(abs(pat)));
    save([dirpath '\beam_pattern.mat'],'digwin','BeamPattern');
