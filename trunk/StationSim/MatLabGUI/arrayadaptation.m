%
% 1. Multi beamformer
% 2. Selection of the high power beams
% 3. Correlation with the incoming data
% 4. Subtraction of the correlated signal 
%
function B=arrayadaptation(signal,px,py,snapshot_number)%(signal)
%
% Author       : Sylvain Alliot
% Organisation : ASTRON (Netherlands Foundation for research in Astronomy)
% Date         : 25 November 2001
%
    dirpath = 'data';
    signal=squeeze(signal(:,:,15)).';
    %load([dirpath '/antenna_config']);     
    %oad([dirpath '/signal_options']);
    xg=mean(px);
    yg=mean(py);
    GridSize=64;
    N_fft=GridSize*4;   % FFT grid interpolation of 4
    ndim=8;
% reshape data for FFT grip
    mat = zeros(ndim^2,ndim^2);
    px= px + abs(min(px));
    py= py + abs(min(py));
    B= zeros(snapshot_number,ndim^2);
    pxy= zeros(ndim,ndim);
    ssss=size(px);  
    
    for n=1:snapshot_number
        for k = 1 : ndim
            for l = 1 : ndim   
                test=0;     
                for m = 1 : ssss(2)
                    if (px(m)== l*xg)&(py(m)==k*yg) 
                        test=1 ;
                        indexantenna=m;
                    end
                end
            end    
    
            if test==1
                pxy(l,k)     = signal(n,indexantenna);
            else
                pxy(l,k)     = 0.;
            end
        end
    % Fourier beamformer
%     size(pxy)
%     size(reshape(fft2(pxy),ndim^2,1))
    size(B);
    B(n,:) = reshape(fft2(pxy),ndim^2,1).';
end
