
-- OTDBlogin (username, passwd)
-- Check is to given combination of username and password is valid
CREATE OR REPLACE FUNCTION OTDBlogin(VARCHAR(80), VARCHAR(80))
  RETURNS INT4 AS '
	DECLARE
		userNumber		INT4;

	BEGIN
	  -- determine parameterID
	  SELECT userID
	  INTO	 userNumber
	  FROM   OTDBuser
	  WHERE  username = $1
	  AND    password = $2;
	  IF NOT FOUND THEN
		RETURN 0;
	  END IF;

	  UPDATE OTDBuser
	  SET	 lastLogin = \'now\'
	  WHERE	 username = $1;

	  RETURN userNumber;
	END;
' LANGUAGE plpgsql;


-- OTDBauthenticate (userID, task, value)
-- Check if the user is allowed to perform a task
CREATE OR REPLACE FUNCTION OTDBauthenticate(INTEGER, INTEGER, INTEGER)
  RETURNS INT4 AS '
	DECLARE

	BEGIN
	  -- determine parameterID
	  SELECT userID
	  FROM   OTDBaccess
	  WHERE  userID = $1
	  AND    task   = $2
	  AND    value  = $3;
	  IF NOT FOUND THEN
		RETURN 0;
	  END IF;

	  RETURN 1;
	END;
' LANGUAGE plpgsql;

