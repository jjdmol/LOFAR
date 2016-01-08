--table for image statistics
CREATE TABLE image_stats(
  image_id int not null,
  run_id int not null,
  kind int not null,
  lr_method int not null,
  value int not null
);
