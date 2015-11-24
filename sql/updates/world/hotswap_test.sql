SET @CREATURE_BEGIN := 5000000;

DELETE FROM `creature_template` WHERE `entry` >= @CREATURE_BEGIN;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES (
@CREATURE_BEGIN, 0, 0, 0, 0, 0, 28641, 0, 0, 0,
'npc_test_script_1', '', '', 0, 83, 83, 2, 190, 0, 4, 2.14286, 1, 3, 0, 1000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 4, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 600, 50, 1, 35, 1, 0, 196, 1, 0, 0,
'npc_test_script_1', 12340);

DELETE FROM `creature` WHERE `id` >= @CREATURE_BEGIN;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES
(@CREATURE_BEGIN + 0, @CREATURE_BEGIN + 0, 1, 0, 0, 1, 0, 0, 16226.8, 16258.3, 13.3215, 0.230069, 300, 0, 0, 13945, 0, 0, 0, 0, 0, 0);
