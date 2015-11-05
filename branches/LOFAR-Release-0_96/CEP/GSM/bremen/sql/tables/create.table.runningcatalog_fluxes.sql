
CREATE TABLE runningcatalog_fluxes(
-- Uniqe combination = runcat_id + band + stokes
    runcat_id INT NOT NULL --reference to the running catalog (positions)
   ,band INT NOT NULL
   ,stokes CHAR(1) NOT NULL DEFAULT 'I'
   ,datapoints INT NOT NULL -- number of observations for this band + stokes
   --Flux information
   ,avg_f_peak DOUBLE NULL
   ,avg_f_peak_sq DOUBLE NULL
   ,avg_weight_f_peak DOUBLE NULL
   ,avg_weighted_f_peak DOUBLE NULL
   ,avg_weighted_f_peak_sq DOUBLE NULL
);
