function eigensystem
    dirpath='data';
    load([dirpath '\antenna_signals.mat']);
    load([dirpath '\antenna_config.mat']);
    load([dirpath '\signal_options.mat']);
    load([dirpath '\bf_options.mat']);
    
    % Hardcoded for now. Is used to determine number of RFI sources
    testRFI=1.5;
    
    fprintf('\tACM\n');
    [ Evector, Evalue, rfi_sources ] = acm(AntennaSignals, NumberOfAntennas, snapshot_number,  rfi_phi, rfi_theta, px, py, rfi_number, testRFI);
    fprintf('\tSteerV\n');
    LookingDirection  = steerv(px,py,look_dir_phi,look_dir_theta);            % Direction of looking
    fprintf('\tAWE\n');

    if (rfi_strat == 3)
        % From data
        WeightVector      = awe(Evector, Evalue, LookingDirection, rfi_number, NumberOfAntennas);
    else
        % MDL rfi detection or Thresholding
        WeightVector      = awe(Evector, Evalue, LookingDirection, rfi_sources, NumberOfAntennas);
    end

    % save these values to file
    save([dirpath '\rfi_eigen.mat'],'Evector','Evalue','LookingDirection','WeightVector', 'rfi_sources');
