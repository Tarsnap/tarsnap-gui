-- Unfortunately, we can't easily import this into QSqlDatabase directly
-- due to the lack of support for executing multiple statements in a single query,
-- thus you need to modify this file to update the db schema, then regenerate
-- dbtemplate.db which is in turn used by the app.

BEGIN TRANSACTION;
CREATE TABLE `jobs` (
	`name`	TEXT NOT NULL,
	`archiveCount`	INTEGER,
	`urls`	TEXT,
	PRIMARY KEY(name)
);
CREATE TABLE `archives` (
	`name`	TEXT NOT NULL,
	`timestamp`	INTEGER NOT NULL,
	`sizeTotal`	INTEGER NOT NULL,
	`sizeCompressed`	INTEGER NOT NULL,
	`sizeUniqueTotal`	INTEGER NOT NULL,
	`sizeUniqueCompressed`	INTEGER NOT NULL,
	`command`	TEXT,
	`contents`	TEXT,
	`jobRef`	TEXT,
	PRIMARY KEY(name)
);
COMMIT;
