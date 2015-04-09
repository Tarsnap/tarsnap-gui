-- Modify this file to update the db schema, then regenerate dbtemplate.db

BEGIN TRANSACTION;
CREATE TABLE `jobs` (
	`name`	TEXT NOT NULL,
	`archiveCount`	INTEGER,
	`urls`	TEXT,
	PRIMARY KEY(name)
);
CREATE TABLE "archives" (
	`name`	TEXT NOT NULL,
	`timestamp`	INTEGER NOT NULL,
	`sizeTotal`	INTEGER NOT NULL,
	`sizeCompressed`	INTEGER NOT NULL,
	`sizeUniqueTotal`	INTEGER NOT NULL,
	`sizeUniqueCompressed`	INTEGER NOT NULL,
	`command`	TEXT NOT NULL,
	`contents`	TEXT,
	`jobRef`	TEXT
	PRIMARY KEY(name)
);
COMMIT;
