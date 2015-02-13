-- MySQL dump 10.13  Distrib 5.6.9-rc, for Win64 (x86_64)
--
-- Host: localhost    Database: hotfixes
-- ------------------------------------------------------
-- Server version	5.6.9-rc

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

-- Dump completed on 2014-10-19 23:50:46

-- Updates base tables
DROP TABLE IF EXISTS `updates`;
CREATE TABLE `updates` (
    `name` VARCHAR(200) NOT NULL COMMENT 'filename with extension of the update.',
    `hash` CHAR(32) NULL DEFAULT '' COMMENT 'md5 hash of the sql file.',
    `state` ENUM('RELEASED','ARCHIVED') NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if an update is released or archived.',
    `timestamp` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'timestamp when the query was applied.',
    `speed` INT(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'time the query takes to apply in ms.',
    PRIMARY KEY (`name`)
)
COMMENT='List of all applied updates in this database.'
COLLATE='utf8_general_ci'
ENGINE=MyISAM;

DROP TABLE IF EXISTS `updates_include`;
CREATE TABLE `updates_include` (
    `path` VARCHAR(200) NOT NULL COMMENT 'directory to include. $ means relative to the source directory.',
    `state` ENUM('RELEASED','ARCHIVED') NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if the directory contains released or archived updates.',
    PRIMARY KEY (`path`)
)
COMMENT='List of directories where we want to include sql updates.'
COLLATE='utf8_general_ci'
ENGINE=MyISAM;

-- Hotfixes database update data
TRUNCATE TABLE `updates_include`;
INSERT INTO `updates_include` (`path`, `state`) VALUES
('$/sql/updates/hotfixes', 'RELEASED'),
('$/sql/custom/hotfixes', 'RELEASED');

INSERT IGNORE INTO `updates` (`name`, `hash`) VALUES
('2014_10_19_01_hotfixes_area_poi.sql', 'E398971C6FE10664CD91B16E1935F232'),
('2014_10_19_02_hotfixes_area_poi_state.sql', '9DDC66F51F0B8BA4F8B4A43BAD3724AA'),
('2014_10_19_03_hotfixes_creature_difficulty.sql', '95EC006DE519FE76D66A3FA704E407EB'),
('2014_10_19_04_hotfixes_creature.sql', '0DB725A146C7914BC5464459A979CB05'),
('2014_10_19_05_hotfixes_broadcast_text.sql', '9BF0C4B20C208DBAFAD6C030BD6FD130'),
('2014_10_19_06_hotfixes_broadcast_text.sql', '6685F345BC69730F96B7662651072AF7'),
('2014_10_20_00_hotfixes_gameobjects.sql', '421F977BAEAEC9476A74AD07153C5E11'),
('2014_10_24_00_hotfixes_taxi_path_node.sql', 'BA3FF4526622B656BAC07D7491FD439A'),
('2014_10_24_01_hotfixes_broadcast_text.sql', '69BCA11FB5A5ED36DB10ABDA4B2EE90D'),
('2014_12_25_00_hotfixes_locale_broadcast_text.sql', '6A4E9D5E03772820212FFB5F5E4D50DD'),
('2014_12_26_00_hotfixes_hotfix_data.sql', '4E23EA19595494667010434E98BBA38E'),
('2015_02_20_00_hotfixes.sql', '96B9DB091B33E9D58B90CBB961B06D88'),
('2015_02_20_01_hotfixes.sql', '50DD72AC3A700CF8634126DCFDD3EBC4');
