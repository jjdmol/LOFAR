function plotspectrum

dirpath='data';
load([dirpath '\output_options']);

if signal_spectrum
    load([dirpath '\signal_options']);
    load([dirpath '\antenna_config']);
    load([dirpath '\antenna_signals']);
    load([dirpath '\rfi_eigen']);

    w = WeightVector;  
    
    %w=steerv(px,py,look_dir_phi,look_dir_theta);
    Y=w(:).'*AntennaSignals(:,:);       % DBF
    N=1024;
    if snapshot_number>N                % Find automatic segment length
        NBINT=fix(snapshot_number/N);
    else
        NBINT=1; 
        N=snapshot_number;
    end
    y=fx(Y, N, NBINT);        % simple autocorrelation
    figure;
    x=(0:1:N/2-1);
    plot(x,20*log10(y));

end;