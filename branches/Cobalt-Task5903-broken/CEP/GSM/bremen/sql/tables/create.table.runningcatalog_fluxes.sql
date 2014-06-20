
CREATE TABLE runningcatalog_fluxes(
-- Uniqe combination = runcat_id + band + stokes
    runcat_id INT NOT NULL --reference to the running catalog (positions)
   ,band INT NOT NULL
   ,stokes CHAR(1) NOT NULL DEFAULT 'I'
   ,datapoints INT NOT NULL -- number of observations for this band + stokes
   --Flux information (peak flux)
   ,wm_f_peak double NULL
   ,wm_f_peak_err double NULL
   ,avg_wf_peak double NULL
   ,avg_weight_f_peak double NULL
   --Flux information (peak flux)
   ,wm_f_int double NULL
   ,wm_f_int_err double NULL
   ,avg_wf_int double NULL
   ,avg_weight_f_int double NULL
   --
   ,PRIMARY KEY (runcat_id, band, stokes)

);
