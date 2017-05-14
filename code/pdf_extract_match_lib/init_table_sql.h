#pragma once



static char s_init_table[] = 
"-- MySQL Administrator dump 1.4\
--\
-- ------------------------------------------------------\
-- Server version	5.6.21-log\
\
\
/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;\
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;\
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;\
/*!40101 SET NAMES utf8 */;\
\
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;\
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;\
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;\
\
\
--\
-- Create schema pdf_hit_detector\
--\
\
CREATE DATABASE IF NOT EXISTS pdf_hit_detector;\
USE pdf_hit_detector;\
\
--\
-- Definition of table `pdf_description`\
--\
\
DROP TABLE IF EXISTS `pdf_description`;\
CREATE TABLE `pdf_description` (\
								`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
								`description` varchar(256) NOT NULL,\
								PRIMARY KEY (`id`)\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
\
--\
-- Dumping data for table `pdf_description`\
--\
\
/*!40000 ALTER TABLE `pdf_description` DISABLE KEYS */;\
/*!40000 ALTER TABLE `pdf_description` ENABLE KEYS */;\
\
\
--\
-- Definition of table `pdf_hash`\
--\
\
DROP TABLE IF EXISTS `pdf_hash`;\
CREATE TABLE `pdf_hash` (\
						 `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
						 `hash_1` int(10) unsigned NOT NULL,\
						 `hash_2` int(10) unsigned NOT NULL,\
						 `hash_3` int(10) unsigned NOT NULL,\
						 `hash_4` int(10) unsigned NOT NULL,\
						 PRIMARY KEY (`id`)\
						 ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
\
--\
-- Dumping data for table `pdf_hash`\
--\
\
/*!40000 ALTER TABLE `pdf_hash` DISABLE KEYS */;\
/*!40000 ALTER TABLE `pdf_hash` ENABLE KEYS */;\
\
\
--\
-- Definition of table `pdf_hash_description`\
--\
\
DROP TABLE IF EXISTS `pdf_hash_description`;\
CREATE TABLE `pdf_hash_description` (\
									 `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
									 `pdf_hash_id` int(10) unsigned NOT NULL,\
									 `pdf_description_id` int(10) unsigned NOT NULL,\
									 PRIMARY KEY (`id`),\
									 KEY `FK_pdf_hash_description_1` (`pdf_hash_id`),\
									 KEY `FK_pdf_hash_description_2` (`pdf_description_id`),\
									 CONSTRAINT `FK_pdf_hash_description_1` FOREIGN KEY (`pdf_hash_id`) REFERENCES `pdf_hash` (`id`),\
									 CONSTRAINT `FK_pdf_hash_description_2` FOREIGN KEY (`pdf_description_id`) REFERENCES `pdf_description` (`id`)\
									 ) ENGINE=InnoDB DEFAULT CHARSET=utf8;\
\
--\
-- Dumping data for table `pdf_hash_description`\
--\
\
/*!40000 ALTER TABLE `pdf_hash_description` DISABLE KEYS */;\
/*!40000 ALTER TABLE `pdf_hash_description` ENABLE KEYS */;\
\
\
\
\
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;\
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;\
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;\
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;\
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;\
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;\
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;\
";