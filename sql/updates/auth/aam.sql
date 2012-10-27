
-- aam auth
ALTER TABLE `account_access`
	ALTER `gmlevel` DROP DEFAULT,
    CHANGE COLUMN `gmlevel` `group` TINYINT(3) UNSIGNED NOT NULL AFTER `id`;

CREATE TABLE `account_group` (
	`RealmID` INT(11) NOT NULL DEFAULT '-1',
	`groupId` INT(11) UNSIGNED NOT NULL,
	`inherits` TINYINT(3) UNSIGNED NOT NULL,
	`name` TEXT NULL,
	PRIMARY KEY (`RealmID`, `groupId`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB;

INSERT INTO `account_group` (`RealmID`, `groupId`, `inherits`, `name`) VALUES
(-1, 1, 0, 'Moderator'),
(-1, 2, 1, 'Gamemaster'),
(-1, 3, 2, 'Administrator');

CREATE TABLE `account_group_security` (
	`RealmID` INT(11) NOT NULL DEFAULT '-1',
	`groupId` INT(11) UNSIGNED NOT NULL,
	`security` TINYINT(3) UNSIGNED NOT NULL,
	PRIMARY KEY (`RealmID`, `groupId`, `security`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB;

INSERT INTO `account_group_security` (`RealmID`, `groupId`, `security`) VALUES
(-1, 1, 1),
(-1, 2, 2),
(-1, 3, 3);
