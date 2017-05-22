-- Unfortunately, we can't easily import this into QSqlDatabase directly
-- due to the lack of support for executing multiple statements in a single 
-- query, thus you need to modify this file to update the db schema, then 
-- regenerate dbtemplate.db which is in turn used by the app.

-- NB: UPDATE the version column below whenever you change this schema

BEGIN TRANSACTION;
CREATE TABLE `version` (
	`version`	INTEGER NOT NULL
);
INSERT INTO version VALUES (4);
CREATE TABLE `jobs` (
	`name`	TEXT NOT NULL,
	`urls`	TEXT,
	`optionScheduledEnabled`	INTEGER,
	`optionPreservePaths`		INTEGER,
	`optionTraverseMount`		INTEGER,
	`optionFollowSymLinks`		INTEGER,
	`optionSkipNoDump`			INTEGER,
	`optionSkipFilesSize`		INTEGER,
	`optionSkipFiles`			INTEGER,
	`optionSkipFilesPatterns`	TEXT,
	`settingShowHidden`			INTEGER,
	`settingShowSystem`			INTEGER,
	`settingHideSymlinks`		INTEGER,
	PRIMARY KEY(name)
);
CREATE TABLE `archives` (
	`name`	TEXT NOT NULL,
	`timestamp`	INTEGER NOT NULL,
	`truncated`	INTEGER NOT NULL,
	`truncatedInfo`	TEXT NOT NULL,
	`sizeTotal`	INTEGER NOT NULL,
	`sizeCompressed`	INTEGER NOT NULL,
	`sizeUniqueTotal`	INTEGER NOT NULL,
	`sizeUniqueCompressed`	INTEGER NOT NULL,
	`command`	TEXT,
	`contents`	TEXT,
	`jobRef`	TEXT,
	PRIMARY KEY(name),
	FOREIGN KEY(jobRef) REFERENCES jobs(name)
);
CREATE TABLE `journal` (
	`timestamp`	INTEGER NOT NULL,
	`log`	TEXT
);
COMMIT;
