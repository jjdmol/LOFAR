-- creates the OTDB security tables

DROP TABLE OTDBaccess;
DROP TABLE OTDBuser;
DROP SEQUENCE	OTDBuserID;


CREATE SEQUENCE	OTDBuserID;

CREATE TABLE OTDBuser (
	userID		INT4			NOT NULL DEFAULT nextval('OTDBuserID'),
	username	VARCHAR(20)		NOT NULL,
	password	VARCHAR(20)		NOT NULL,
	role		VARCHAR(30),
	lastlogin	timestamp,

	CONSTRAINT	username_uniq	UNIQUE(username)
) WITHOUT OIDS;

INSERT INTO OTDBuser (username, password, role) 
			VALUES ('paulus', 'boskabouter', 'developer');



CREATE TABLE OTDBaccess (
	userID		INT4			NOT NULL REFERENCES OTDBuser(userID),
	task		INT4			NOT NULL DEFAULT 0,
	value		INT4			NOT NULL DEFAULT 0
) WITHOUT OIDS;


