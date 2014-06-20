CREATE SEQUENCE "seq_runs" AS INTEGER;

CREATE TABLE runs (
  runid INT DEFAULT NEXT VALUE FOR "seq_runs",
  start_date timestamp not null default current_timestamp,
  end_date timestamp null,
  status int null, --0-started, 1-finished, 2-broken
  user_id char(100) not null,
  process_id int not null
);
