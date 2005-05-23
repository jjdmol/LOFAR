-- creates a log-system;

DROP TABLE log;

CREATE TABLE log (
	msg		TEXT
) WITH OIDS;

CREATE OR REPLACE FUNCTION logmsg(TEXT)
  RETURNS VOID AS '
	BEGIN
		INSERT INTO log VALUES ($1);
		RETURN;
	END;
' language plpgsql;

CREATE OR REPLACE FUNCTION clearlog()
  RETURNS VOID AS '
	BEGIN
	  DELETE FROM log;
	END;
' language plpgsql;

