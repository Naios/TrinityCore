-- AUTH
-- Defines which group (former 'gmlevel') has which security level on which realm.
DROP TABLE IF EXISTS `account_group_access`;
CREATE TABLE `account_group_access`(
    `group` INT(11) UNSIGNED NOT NULL DEFAULT 0,
    `realm` INT(11) NOT NULL DEFAULT -1,
	`security` INT(11) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY(`id`,`realm`,`security`)
);

-- Some test values
INSERT INTO `account_group_access`(`group`,`realm`,`security`) VALUES
(-1,1,1), -- Group 1 can execute commands with security level 1 on every realm.
(-1,2,2), -- Group 2 can execute commands with security level 2 on every realm.
( 1,2,3); -- Group 2 can execute commands with security level 2 on realm 1.

-- Alter account access
ALTER TABLE `account_access` DROP PRIMARY KEY, CHANGE COLUMN `gmlevel` `group` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 PRIMARY KEY AFTER `id`, DROP `RealmID`, RENAME TO `account_group`;

-- WORLD
-- Defines which command can be executed by which security level
DROP TABLE IF EXISTS `command_security`;
CREATE TABLE `command_security`(
    `security` INT(11) UNSIGNED NOT NULL DEFAULT 0,
    `command` INT(11) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY(`security`,`command`)
);

-- Create command id;
ALTER TABLE `command` ADD `id` INT(11) UNSIGNED NOT NULL DEFAULT 0 FIRST, DROP PRIMARY KEY, ADD PRIMARY KEY(`id`);

-- Give unique numbers to commands
SET @COUNT = 0;
UPDATE `command` SET `id`=@COUNT:=(@COUNT+1);

