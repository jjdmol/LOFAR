 function plotspectrum2(px,py,look_dir_phi,look_dir_theta,snapshot_number,SelectedSubBands,WeightVector)

dirpath='data';
load([dirpath '\antenna_signals']);
    
    % First plot spectrum without adaptive nulling        
    w=steerv(px,py,look_dir_phi,look_dir_theta);
    Y=w(:).'*SelectedSubBandSignals(:,:,SelectedSubBands);       % DBF
    N=1024;
    if snapshot_number>N                % Find automatic segment length
        NBINT=fix(snapshot_number/N);
    else
        NBINT=1; 
        N=snapshot_number;
    end
    y=fx(Y, N, NBINT);        % simple autocorrelation
    figure;
    subplot(1,2,1)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y)));
    %plot(x,20*log10(y));
    %axis([0 N/2 10 100])
    title('Spectrum before adaptive nulling');
    
    % Then plot spectrum with adaptive nulling;
    Y=WeightVector(:).'*SelectedSubBandSignals(:,:,SelectedSubBands);%AntennaSignals(:,:); 
    if snapshot_number>N                % Find automatic segment length
        NBINT=fix(snapshot_number/N);
    else
        NBINT=1; 
        N=snapshot_number;
    end
    y=fx(Y, N, NBINT);        % simple autocorrelation
    subplot(1,2,2)
    x=(0:1:N/2-1);
    plot(20*log10(abs(Y)));
    %plot(x,20*log10(y));
    %axis([0 N/2 10 100])
    title('Spectrum after adaptive nulling')