DROP TABLE IF EXISTS `phase_template`;
CREATE TABLE `phase_template` (
  `zoneId` mediumint(7) unsigned NOT NULL DEFAULT '0',
  `entry` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `phasemask` bigint(20) unsigned NOT NULL DEFAULT '1',
  `phaseId` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `terrainswapmap` smallint(5) unsigned NOT NULL DEFAULT '0',
  `flags` tinyint(3) unsigned DEFAULT '0',
  `comment` text,
  PRIMARY KEY (`entry`,`zoneId`)
)
AUTO_INCREMENT=1
ENGINE=MyISAM
COLLATE='utf8_general_ci';

INSERT INTO `phase_template` (`zoneId`, `entry`, `phasemask`, `phaseId`, `terrainswapmap`, `flags`, `comment`) VALUES
(1519, 1, 129,  0,   0,   0, 'Stormwind: [A] Heros Call: Vashj''ir'),
(1519, 2, 257,  0,   0,   0, 'Stormwind: [A] Heros Call: Hyjal'),
(1519, 3, 513,  0,   0,   0, 'Stormwind: [A] Heros Call: Deepholm'),
(1519, 4, 1025, 0,   0,   0, 'Stormwind: [A] Heros Call: Uldum'),
(1519, 5, 2049, 0,   0,   0, 'Stormwind: [A] Heros Call: Twilight Highlands'),
(1637, 1, 129,  0,   0,   0, 'Orgrimmar: [H] Warchiefs Command: Vashj''ir'),
(1637, 2, 257,  0,   0,   0, 'Orgrimmar: [H] Warchiefs Command: Hyjal'),
(1637, 3, 513,  0,   0,   0, 'Orgrimmar: [H] Warchiefs Command: Deepholm'),
(1637, 4, 1025, 0,   0,   0, 'Orgrimmar: [H] Warchiefs Command: Uldum'),
(1637, 5, 2049, 0,   0,   0, 'Orgrimmar: [H] Warchiefs Command: Twilight Highlands'),
(4755, 1, 0,    102, 638, 0, 'Gilneas: Default Terrainswap'),
( 616, 1, 0,    165, 719, 0, 'Mount Hyjal: Default Terrainswap');

DELETE FROM `trinity_string` WHERE `entry` BETWEEN 176 AND 182;
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES
(176, '|cffFF0000Phasing: Report for player: %s, zoneId: %u, level: %u, team: %u, phaseupdateflag: %u|r'),
(177, '|cffFF0000Phasing: There are no definitions defined for zoneId %u.|r'),
(178, '|cffFF0000Phasing: Success - added %s %u to the players phase.|r'),
(179, '|cffFF0000Phasing: Condition for phase %u (entry: %u, zoneId: %u) failed.|r'),
(180, '|cffFF0000Phasing: Condition for phase %u (entry: %u, zoneId: %u) has last phasemask flag. Skipped other definitions.|r'),
(181, '|cffFF0000Phasing: The player gets phasemask %u through definitions, %u through phasing auras, and phase %u through custom phase.|r'),
(182, '|cffFF0000Phasing: The player has phasemask %u (real: %u).|r');

DELETE FROM `command` WHERE `name` IN('debug phase', 'debug send setphaseshift');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('debug phase', 1, 'Syntax: .debug phase\r\n\r\nSends a phase debug report of a player to you.');

-- Test Condition (SourceGroup -> ZoneId, SourceEntry -> Entry)
DELETE FROM `conditions` WHERE  `SourceTypeOrReferenceId`=23 AND `SourceGroup`=1519 AND `SourceEntry`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(23, 1519, 1, 0, 0, 6, 0, 469, 0, 0, 0, 0, '', 'Phase Condition: Vashj''ir phase only visible for alliance members (for debug)');
