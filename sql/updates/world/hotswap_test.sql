SET @CREATURE_BEGIN := 5000000;

DELETE FROM `creature_template` WHERE `entry` >= @CREATURE_BEGIN;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(@CREATURE_BEGIN, 0, 0, 0, 53183, 0, 26713, 0, 0, 0,
'npc_test_script_1', '', '', '', 0, 76, 77, 2, 0, 14, 0, 1.44444, 1.5873, 1, 0, 0, 2000, 2000, 1, 1, 2, 0, 2048, 0, 0, 0, 0, 0, 2, 0, 0, 28378, 0, 70209, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 7, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
'npc_test_script_1', 20173);

DELETE FROM `creature` WHERE `id` >= @CREATURE_BEGIN;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(@CREATURE_BEGIN + 0, @CREATURE_BEGIN + 0, 1, 0, 0, 1, 0, 0, 16226.8, 16258.3, 13.3215, 0.230069, 300, 0, 0, 13945, 0, 0, 0, 0, 0, 0),
(@CREATURE_BEGIN + 1, @CREATURE_BEGIN + 1, 1, 0, 0, 1, 0, 0, 16283.8, 16314.4, 12.5323, 4.23324, 300, 0, 0, 13945, 0, 0, 0, 0, 0, 0),
(@CREATURE_BEGIN + 2, @CREATURE_BEGIN + 2, 1, 0, 0, 1, 0, 0, 16271.3, 16239.7, 28.7183, 2.15114, 300, 0, 0, 13945, 0, 0, 0, 0, 0, 0);
