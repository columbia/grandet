-- MySQL dump 10.13  Distrib 5.5.30, for Linux (x86_64)
--
-- Host: localhost    Database: elgg
-- ------------------------------------------------------
-- Server version	5.5.30-log

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

--
-- Table structure for table `elgg_access_collection_membership`
--

USE elgg;

DROP TABLE IF EXISTS `elgg_access_collection_membership`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_access_collection_membership` (
  `user_guid` int(11) NOT NULL,
  `access_collection_id` int(11) NOT NULL,
  PRIMARY KEY (`user_guid`,`access_collection_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_access_collection_membership`
--

LOCK TABLES `elgg_access_collection_membership` WRITE;
/*!40000 ALTER TABLE `elgg_access_collection_membership` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_access_collection_membership` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_access_collections`
--

DROP TABLE IF EXISTS `elgg_access_collections`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_access_collections` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `owner_guid` bigint(20) unsigned NOT NULL,
  `site_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `owner_guid` (`owner_guid`),
  KEY `site_guid` (`site_guid`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_access_collections`
--

LOCK TABLES `elgg_access_collections` WRITE;
/*!40000 ALTER TABLE `elgg_access_collections` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_access_collections` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_annotations`
--

DROP TABLE IF EXISTS `elgg_annotations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_annotations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `entity_guid` bigint(20) unsigned NOT NULL,
  `name_id` int(11) NOT NULL,
  `value_id` int(11) NOT NULL,
  `value_type` enum('integer','text') NOT NULL,
  `owner_guid` bigint(20) unsigned NOT NULL,
  `access_id` int(11) NOT NULL,
  `time_created` int(11) NOT NULL,
  `enabled` enum('yes','no') NOT NULL DEFAULT 'yes',
  PRIMARY KEY (`id`),
  KEY `entity_guid` (`entity_guid`),
  KEY `name_id` (`name_id`),
  KEY `value_id` (`value_id`),
  KEY `owner_guid` (`owner_guid`),
  KEY `access_id` (`access_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_annotations`
--

LOCK TABLES `elgg_annotations` WRITE;
/*!40000 ALTER TABLE `elgg_annotations` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_annotations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_api_users`
--

DROP TABLE IF EXISTS `elgg_api_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_api_users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `site_guid` bigint(20) unsigned DEFAULT NULL,
  `api_key` varchar(40) DEFAULT NULL,
  `secret` varchar(40) NOT NULL,
  `active` int(1) DEFAULT '1',
  PRIMARY KEY (`id`),
  UNIQUE KEY `api_key` (`api_key`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_api_users`
--

LOCK TABLES `elgg_api_users` WRITE;
/*!40000 ALTER TABLE `elgg_api_users` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_api_users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_config`
--

DROP TABLE IF EXISTS `elgg_config`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_config` (
  `name` varchar(255) NOT NULL,
  `value` text NOT NULL,
  `site_guid` int(11) NOT NULL,
  PRIMARY KEY (`name`,`site_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_config`
--

LOCK TABLES `elgg_config` WRITE;
/*!40000 ALTER TABLE `elgg_config` DISABLE KEYS */;
INSERT INTO `elgg_config` VALUES ('view','s:7:\"default\";',1),('language','s:2:\"en\";',1),('default_access','s:1:\"2\";',1),('allow_registration','b:1;',1),('walled_garden','b:0;',1),('allow_user_default_access','s:0:\"\";',1),('default_limit','i:10;',1);
/*!40000 ALTER TABLE `elgg_config` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_datalists`
--

DROP TABLE IF EXISTS `elgg_datalists`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_datalists` (
  `name` varchar(255) NOT NULL,
  `value` text NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_datalists`
--

LOCK TABLES `elgg_datalists` WRITE;
/*!40000 ALTER TABLE `elgg_datalists` DISABLE KEYS */;
INSERT INTO `elgg_datalists` VALUES ('installed','1426799685'),('path','/'),('dataroot','/tmp/elgg_upload/'),('default_site','1'),('version','2014130300'),('simplecache_enabled','1'),('system_cache_enabled','1'),('simplecache_lastupdate','1426799685'),('processed_upgrades','b:0;'),('admin_registered','1'),('__site_secret__','zjEjawBfI0gmY_A97hvkJmki1HF8qxit');
/*!40000 ALTER TABLE `elgg_datalists` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_entities`
--

DROP TABLE IF EXISTS `elgg_entities`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_entities` (
  `guid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `type` enum('object','user','group','site') NOT NULL,
  `subtype` int(11) DEFAULT NULL,
  `owner_guid` bigint(20) unsigned NOT NULL,
  `site_guid` bigint(20) unsigned NOT NULL,
  `container_guid` bigint(20) unsigned NOT NULL,
  `access_id` int(11) NOT NULL,
  `time_created` int(11) NOT NULL,
  `time_updated` int(11) NOT NULL,
  `last_action` int(11) NOT NULL DEFAULT '0',
  `enabled` enum('yes','no') NOT NULL DEFAULT 'yes',
  PRIMARY KEY (`guid`),
  KEY `type` (`type`),
  KEY `subtype` (`subtype`),
  KEY `owner_guid` (`owner_guid`),
  KEY `site_guid` (`site_guid`),
  KEY `container_guid` (`container_guid`),
  KEY `access_id` (`access_id`),
  KEY `time_created` (`time_created`),
  KEY `time_updated` (`time_updated`)
) ENGINE=MyISAM AUTO_INCREMENT=46 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_entities`
--

LOCK TABLES `elgg_entities` WRITE;
/*!40000 ALTER TABLE `elgg_entities` DISABLE KEYS */;
INSERT INTO `elgg_entities` VALUES (1,'site',0,0,1,0,2,1426799685,1426799685,1426799685,'yes'),(2,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(3,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(4,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(5,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(6,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(7,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(8,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(9,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(10,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(11,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(12,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(13,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(14,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(15,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(16,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(17,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(18,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(19,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(20,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(21,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(22,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(23,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(24,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(25,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(26,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(27,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(28,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(29,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(30,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(31,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(32,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(33,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(34,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(35,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(36,'object',1,1,1,1,2,1426799685,1426799685,1426799685,'yes'),(37,'user',0,0,1,0,2,1426799709,1426799709,1426799709,'yes'),(38,'object',3,37,1,37,0,1426799709,1426799709,1426799709,'yes'),(39,'object',3,37,1,37,0,1426799709,1426799709,1426799709,'yes'),(40,'object',3,37,1,37,0,1426799709,1426799709,1426799709,'yes'),(41,'object',3,37,1,37,0,1426799709,1426799709,1426799709,'yes'),(42,'object',3,37,1,37,0,1426799709,1426799709,1426799709,'yes'),(43,'object',6,37,1,37,2,1426799754,1426799754,1426799754,'yes'),(44,'object',2,37,1,37,2,1426799791,1426799791,1426799791,'yes'),(45,'object',2,37,1,37,2,1426799992,1426799992,1426799992,'yes');
/*!40000 ALTER TABLE `elgg_entities` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_entity_relationships`
--

DROP TABLE IF EXISTS `elgg_entity_relationships`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_entity_relationships` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `guid_one` bigint(20) unsigned NOT NULL,
  `relationship` varchar(50) NOT NULL,
  `guid_two` bigint(20) unsigned NOT NULL,
  `time_created` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `guid_one` (`guid_one`,`relationship`,`guid_two`),
  KEY `relationship` (`relationship`),
  KEY `guid_two` (`guid_two`)
) ENGINE=MyISAM AUTO_INCREMENT=24 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_entity_relationships`
--

LOCK TABLES `elgg_entity_relationships` WRITE;
/*!40000 ALTER TABLE `elgg_entity_relationships` DISABLE KEYS */;
INSERT INTO `elgg_entity_relationships` VALUES (1,3,'active_plugin',1,1426799685),(2,4,'active_plugin',1,1426799685),(3,6,'active_plugin',1,1426799685),(4,13,'active_plugin',1,1426799685),(5,14,'active_plugin',1,1426799685),(6,15,'active_plugin',1,1426799685),(7,16,'active_plugin',1,1426799685),(8,17,'active_plugin',1,1426799685),(9,19,'active_plugin',1,1426799685),(10,20,'active_plugin',1,1426799685),(11,21,'active_plugin',1,1426799685),(12,22,'active_plugin',1,1426799685),(13,23,'active_plugin',1,1426799685),(14,24,'active_plugin',1,1426799685),(15,25,'active_plugin',1,1426799685),(16,26,'active_plugin',1,1426799685),(17,27,'active_plugin',1,1426799685),(18,28,'active_plugin',1,1426799685),(19,29,'active_plugin',1,1426799685),(20,32,'active_plugin',1,1426799685),(22,36,'active_plugin',1,1426799685),(23,37,'member_of_site',1,1426799709);
/*!40000 ALTER TABLE `elgg_entity_relationships` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_entity_subtypes`
--

DROP TABLE IF EXISTS `elgg_entity_subtypes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_entity_subtypes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` enum('object','user','group','site') NOT NULL,
  `subtype` varchar(50) NOT NULL,
  `class` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `type` (`type`,`subtype`)
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_entity_subtypes`
--

LOCK TABLES `elgg_entity_subtypes` WRITE;
/*!40000 ALTER TABLE `elgg_entity_subtypes` DISABLE KEYS */;
INSERT INTO `elgg_entity_subtypes` VALUES (1,'object','plugin','ElggPlugin'),(2,'object','file','ElggFile'),(3,'object','widget','ElggWidget'),(4,'object','comment','ElggComment'),(5,'object','elgg_upgrade','ElggUpgrade'),(6,'object','blog','ElggBlog'),(7,'object','discussion_reply','ElggDiscussionReply'),(8,'object','thewire','ElggWire');
/*!40000 ALTER TABLE `elgg_entity_subtypes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_geocode_cache`
--

DROP TABLE IF EXISTS `elgg_geocode_cache`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_geocode_cache` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` varchar(128) DEFAULT NULL,
  `lat` varchar(20) DEFAULT NULL,
  `long` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `location` (`location`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_geocode_cache`
--

LOCK TABLES `elgg_geocode_cache` WRITE;
/*!40000 ALTER TABLE `elgg_geocode_cache` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_geocode_cache` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_groups_entity`
--

DROP TABLE IF EXISTS `elgg_groups_entity`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_groups_entity` (
  `guid` bigint(20) unsigned NOT NULL,
  `name` text NOT NULL,
  `description` text NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `name` (`name`(50)),
  KEY `description` (`description`(50)),
  FULLTEXT KEY `name_2` (`name`,`description`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_groups_entity`
--

LOCK TABLES `elgg_groups_entity` WRITE;
/*!40000 ALTER TABLE `elgg_groups_entity` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_groups_entity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_hmac_cache`
--

DROP TABLE IF EXISTS `elgg_hmac_cache`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_hmac_cache` (
  `hmac` varchar(255) NOT NULL,
  `ts` int(11) NOT NULL,
  PRIMARY KEY (`hmac`),
  KEY `ts` (`ts`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_hmac_cache`
--

LOCK TABLES `elgg_hmac_cache` WRITE;
/*!40000 ALTER TABLE `elgg_hmac_cache` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_hmac_cache` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_metadata`
--

DROP TABLE IF EXISTS `elgg_metadata`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_metadata` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `entity_guid` bigint(20) unsigned NOT NULL,
  `name_id` int(11) NOT NULL,
  `value_id` int(11) NOT NULL,
  `value_type` enum('integer','text') NOT NULL,
  `owner_guid` bigint(20) unsigned NOT NULL,
  `access_id` int(11) NOT NULL,
  `time_created` int(11) NOT NULL,
  `enabled` enum('yes','no') NOT NULL DEFAULT 'yes',
  PRIMARY KEY (`id`),
  KEY `entity_guid` (`entity_guid`),
  KEY `name_id` (`name_id`),
  KEY `value_id` (`value_id`),
  KEY `owner_guid` (`owner_guid`),
  KEY `access_id` (`access_id`)
) ENGINE=MyISAM AUTO_INCREMENT=28 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_metadata`
--

LOCK TABLES `elgg_metadata` WRITE;
/*!40000 ALTER TABLE `elgg_metadata` DISABLE KEYS */;
INSERT INTO `elgg_metadata` VALUES (1,1,1,2,'text',0,2,1426799685,'yes'),(2,37,3,4,'text',37,2,1426799709,'yes'),(3,37,5,4,'text',0,2,1426799709,'yes'),(4,37,6,7,'text',0,2,1426799709,'yes'),(5,43,13,14,'text',37,2,1426799754,'yes'),(6,43,15,16,'text',37,2,1426799754,'yes'),(7,43,17,18,'text',37,2,1426799754,'yes'),(8,44,19,20,'text',37,2,1426799791,'yes'),(9,44,21,22,'text',37,2,1426799791,'yes'),(10,44,23,24,'text',37,2,1426799791,'yes'),(11,44,25,26,'text',37,2,1426799791,'yes'),(12,44,27,28,'text',37,2,1426799791,'yes'),(13,44,29,30,'text',37,2,1426799791,'yes'),(14,44,31,32,'integer',37,2,1426799791,'yes'),(15,44,33,34,'text',37,2,1426799791,'yes'),(16,44,35,36,'text',37,2,1426799791,'yes'),(17,44,37,38,'text',37,2,1426799791,'yes'),(18,45,19,39,'text',37,2,1426799992,'yes'),(19,45,21,40,'text',37,2,1426799992,'yes'),(20,45,23,24,'text',37,2,1426799992,'yes'),(21,45,25,26,'text',37,2,1426799992,'yes'),(22,45,27,28,'text',37,2,1426799992,'yes'),(23,45,29,30,'text',37,2,1426799992,'yes'),(24,45,31,41,'integer',37,2,1426799992,'yes'),(25,45,33,42,'text',37,2,1426799992,'yes'),(26,45,35,43,'text',37,2,1426799992,'yes'),(27,45,37,44,'text',37,2,1426799992,'yes');
/*!40000 ALTER TABLE `elgg_metadata` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_metastrings`
--

DROP TABLE IF EXISTS `elgg_metastrings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_metastrings` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `string` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `string` (`string`(50))
) ENGINE=MyISAM AUTO_INCREMENT=45 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_metastrings`
--

LOCK TABLES `elgg_metastrings` WRITE;
/*!40000 ALTER TABLE `elgg_metastrings` DISABLE KEYS */;
INSERT INTO `elgg_metastrings` VALUES (1,'email'),(2,''),(3,'notification:method:email'),(4,'1'),(5,'validated'),(6,'validated_method'),(7,'admin_user'),(8,'toId'),(9,'37'),(10,'readYet'),(11,'0'),(12,'msg'),(13,'status'),(14,'published'),(15,'comments_on'),(16,'On'),(17,'excerpt'),(18,'bar'),(19,'filename'),(20,'file/142679979116623409210_7e5126c135_z.jpg'),(21,'originalfilename'),(22,'16623409210_7e5126c135_z.jpg'),(23,'mimetype'),(24,'image/jpeg'),(25,'simpletype'),(26,'image'),(27,'filestore::dir_root'),(28,'/tmp/elgg_upload/'),(29,'filestore::filestore'),(30,'ElggDiskFilestore'),(31,'icontime'),(32,'1426799791'),(33,'thumbnail'),(34,'file/thumb142679979116623409210_7e5126c135_z.jpg'),(35,'smallthumb'),(36,'file/smallthumb142679979116623409210_7e5126c135_z.jpg'),(37,'largethumb'),(38,'file/largethumb142679979116623409210_7e5126c135_z.jpg'),(39,'file/1426799992xx.jpg'),(40,'xx.jpg'),(41,'1426799992'),(42,'file/thumb1426799992xx.jpg'),(43,'file/smallthumb1426799992xx.jpg'),(44,'file/largethumb1426799992xx.jpg');
/*!40000 ALTER TABLE `elgg_metastrings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_objects_entity`
--

DROP TABLE IF EXISTS `elgg_objects_entity`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_objects_entity` (
  `guid` bigint(20) unsigned NOT NULL,
  `title` text NOT NULL,
  `description` text NOT NULL,
  PRIMARY KEY (`guid`),
  FULLTEXT KEY `title` (`title`,`description`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_objects_entity`
--

LOCK TABLES `elgg_objects_entity` WRITE;
/*!40000 ALTER TABLE `elgg_objects_entity` DISABLE KEYS */;
INSERT INTO `elgg_objects_entity` VALUES (2,'aalborg_theme',''),(3,'blog',''),(4,'bookmarks',''),(5,'categories',''),(6,'ckeditor',''),(7,'custom_index',''),(8,'dashboard',''),(9,'developers',''),(10,'diagnostics',''),(11,'embed',''),(12,'externalpages',''),(13,'file',''),(14,'garbagecollector',''),(15,'groups',''),(16,'htmlawed',''),(17,'invitefriends',''),(18,'legacy_urls',''),(19,'likes',''),(20,'logbrowser',''),(21,'logrotate',''),(22,'members',''),(23,'messageboard',''),(24,'messages',''),(25,'notifications',''),(26,'pages',''),(27,'profile',''),(28,'reportedcontent',''),(29,'search',''),(30,'site_notifications',''),(31,'tagcloud',''),(32,'thewire',''),(33,'twitter_api',''),(34,'uservalidationbyemail',''),(35,'web_services',''),(36,'zaudio',''),(38,'',''),(39,'',''),(40,'',''),(41,'',''),(42,'',''),(43,'foo','foobar'),(44,'foo','bar'),(45,'xxx','xxx');
/*!40000 ALTER TABLE `elgg_objects_entity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_private_settings`
--

DROP TABLE IF EXISTS `elgg_private_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_private_settings` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `entity_guid` int(11) NOT NULL,
  `name` varchar(128) NOT NULL,
  `value` text NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `entity_guid` (`entity_guid`,`name`),
  KEY `name` (`name`),
  KEY `value` (`value`(50))
) ENGINE=MyISAM AUTO_INCREMENT=57 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_private_settings`
--

LOCK TABLES `elgg_private_settings` WRITE;
/*!40000 ALTER TABLE `elgg_private_settings` DISABLE KEYS */;
INSERT INTO `elgg_private_settings` VALUES (1,2,'elgg:internal:priority','35'),(2,3,'elgg:internal:priority','1'),(3,4,'elgg:internal:priority','2'),(4,5,'elgg:internal:priority','3'),(5,6,'elgg:internal:priority','4'),(6,7,'elgg:internal:priority','5'),(7,8,'elgg:internal:priority','6'),(8,9,'elgg:internal:priority','7'),(9,10,'elgg:internal:priority','8'),(10,11,'elgg:internal:priority','9'),(11,12,'elgg:internal:priority','10'),(12,13,'elgg:internal:priority','11'),(13,14,'elgg:internal:priority','12'),(14,15,'elgg:internal:priority','13'),(15,16,'elgg:internal:priority','14'),(16,17,'elgg:internal:priority','15'),(17,18,'elgg:internal:priority','16'),(18,19,'elgg:internal:priority','17'),(19,20,'elgg:internal:priority','18'),(20,21,'elgg:internal:priority','19'),(21,22,'elgg:internal:priority','20'),(22,23,'elgg:internal:priority','21'),(23,24,'elgg:internal:priority','22'),(24,25,'elgg:internal:priority','23'),(25,26,'elgg:internal:priority','24'),(26,27,'elgg:internal:priority','25'),(27,28,'elgg:internal:priority','26'),(28,29,'elgg:internal:priority','27'),(29,30,'elgg:internal:priority','28'),(30,31,'elgg:internal:priority','29'),(31,32,'elgg:internal:priority','30'),(32,33,'elgg:internal:priority','31'),(33,34,'elgg:internal:priority','32'),(34,35,'elgg:internal:priority','33'),(35,36,'elgg:internal:priority','34'),(36,32,'limit','140'),(37,38,'handler','control_panel'),(38,38,'context','admin'),(39,38,'column','1'),(40,38,'order','0'),(41,39,'handler','admin_welcome'),(42,39,'context','admin'),(43,39,'order','10'),(44,39,'column','1'),(45,40,'handler','online_users'),(46,40,'context','admin'),(47,40,'column','2'),(48,40,'order','0'),(49,41,'handler','new_users'),(50,41,'context','admin'),(51,41,'order','10'),(52,41,'column','2'),(53,42,'handler','content_stats'),(54,42,'context','admin'),(55,42,'order','20'),(56,42,'column','2');
/*!40000 ALTER TABLE `elgg_private_settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_queue`
--

DROP TABLE IF EXISTS `elgg_queue`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_queue` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `data` mediumblob NOT NULL,
  `timestamp` int(11) NOT NULL,
  `worker` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `retrieve` (`timestamp`,`worker`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_queue`
--

LOCK TABLES `elgg_queue` WRITE;
/*!40000 ALTER TABLE `elgg_queue` DISABLE KEYS */;
INSERT INTO `elgg_queue` VALUES (1,'notifications','O:24:\"Elgg\\Notifications\\Event\":5:{s:9:\"\0*\0action\";s:7:\"publish\";s:14:\"\0*\0object_type\";s:6:\"object\";s:17:\"\0*\0object_subtype\";s:4:\"blog\";s:12:\"\0*\0object_id\";i:43;s:13:\"\0*\0actor_guid\";i:37;}',1426799754,NULL),(2,'notifications','O:24:\"Elgg\\Notifications\\Event\":5:{s:9:\"\0*\0action\";s:6:\"create\";s:14:\"\0*\0object_type\";s:6:\"object\";s:17:\"\0*\0object_subtype\";s:4:\"file\";s:12:\"\0*\0object_id\";i:44;s:13:\"\0*\0actor_guid\";i:37;}',1426799791,NULL),(3,'notifications','O:24:\"Elgg\\Notifications\\Event\":5:{s:9:\"\0*\0action\";s:6:\"create\";s:14:\"\0*\0object_type\";s:6:\"object\";s:17:\"\0*\0object_subtype\";s:4:\"file\";s:12:\"\0*\0object_id\";i:45;s:13:\"\0*\0actor_guid\";i:37;}',1426799992,NULL);
/*!40000 ALTER TABLE `elgg_queue` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_river`
--

DROP TABLE IF EXISTS `elgg_river`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_river` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(8) NOT NULL,
  `subtype` varchar(32) NOT NULL,
  `action_type` varchar(32) NOT NULL,
  `access_id` int(11) NOT NULL,
  `view` text NOT NULL,
  `subject_guid` int(11) NOT NULL,
  `object_guid` int(11) NOT NULL,
  `target_guid` int(11) NOT NULL,
  `annotation_id` int(11) NOT NULL,
  `posted` int(11) NOT NULL,
  `enabled` enum('yes','no') NOT NULL DEFAULT 'yes',
  PRIMARY KEY (`id`),
  KEY `type` (`type`),
  KEY `action_type` (`action_type`),
  KEY `access_id` (`access_id`),
  KEY `subject_guid` (`subject_guid`),
  KEY `object_guid` (`object_guid`),
  KEY `target_guid` (`target_guid`),
  KEY `annotation_id` (`annotation_id`),
  KEY `posted` (`posted`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_river`
--

LOCK TABLES `elgg_river` WRITE;
/*!40000 ALTER TABLE `elgg_river` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_river` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_sites_entity`
--

DROP TABLE IF EXISTS `elgg_sites_entity`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_sites_entity` (
  `guid` bigint(20) unsigned NOT NULL,
  `name` text NOT NULL,
  `description` text NOT NULL,
  `url` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`),
  UNIQUE KEY `url` (`url`),
  FULLTEXT KEY `name` (`name`,`description`,`url`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_sites_entity`
--

LOCK TABLES `elgg_sites_entity` WRITE;
/*!40000 ALTER TABLE `elgg_sites_entity` DISABLE KEYS */;
INSERT INTO `elgg_sites_entity` VALUES (1,'Foo Bar','','http://HOST_NAME/');
/*!40000 ALTER TABLE `elgg_sites_entity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_system_log`
--

DROP TABLE IF EXISTS `elgg_system_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_system_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `object_id` int(11) NOT NULL,
  `object_class` varchar(50) NOT NULL,
  `object_type` varchar(50) NOT NULL,
  `object_subtype` varchar(50) NOT NULL,
  `event` varchar(50) NOT NULL,
  `performed_by_guid` int(11) NOT NULL,
  `owner_guid` int(11) NOT NULL,
  `access_id` int(11) NOT NULL,
  `enabled` enum('yes','no') NOT NULL DEFAULT 'yes',
  `time_created` int(11) NOT NULL,
  `ip_address` varchar(46) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `object_id` (`object_id`),
  KEY `object_class` (`object_class`),
  KEY `object_type` (`object_type`),
  KEY `object_subtype` (`object_subtype`),
  KEY `event` (`event`),
  KEY `performed_by_guid` (`performed_by_guid`),
  KEY `access_id` (`access_id`),
  KEY `time_created` (`time_created`),
  KEY `river_key` (`object_type`,`object_subtype`,`event`)
) ENGINE=MyISAM AUTO_INCREMENT=81 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_system_log`
--

LOCK TABLES `elgg_system_log` WRITE;
/*!40000 ALTER TABLE `elgg_system_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_system_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_users_apisessions`
--

DROP TABLE IF EXISTS `elgg_users_apisessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_users_apisessions` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_guid` bigint(20) unsigned NOT NULL,
  `site_guid` bigint(20) unsigned NOT NULL,
  `token` varchar(40) DEFAULT NULL,
  `expires` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_guid` (`user_guid`,`site_guid`),
  KEY `token` (`token`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_users_apisessions`
--

LOCK TABLES `elgg_users_apisessions` WRITE;
/*!40000 ALTER TABLE `elgg_users_apisessions` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_users_apisessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_users_entity`
--

DROP TABLE IF EXISTS `elgg_users_entity`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_users_entity` (
  `guid` bigint(20) unsigned NOT NULL,
  `name` text NOT NULL,
  `username` varchar(128) NOT NULL DEFAULT '',
  `password` varchar(32) NOT NULL DEFAULT '' COMMENT 'Legacy password hashes',
  `salt` varchar(8) NOT NULL DEFAULT '' COMMENT 'Legacy password salts',
  `password_hash` varchar(255) NOT NULL DEFAULT '',
  `email` text NOT NULL,
  `language` varchar(6) NOT NULL DEFAULT '',
  `banned` enum('yes','no') NOT NULL DEFAULT 'no',
  `admin` enum('yes','no') NOT NULL DEFAULT 'no',
  `last_action` int(11) NOT NULL DEFAULT '0',
  `prev_last_action` int(11) NOT NULL DEFAULT '0',
  `last_login` int(11) NOT NULL DEFAULT '0',
  `prev_last_login` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  UNIQUE KEY `username` (`username`),
  KEY `password` (`password`),
  KEY `email` (`email`(50)),
  KEY `last_action` (`last_action`),
  KEY `last_login` (`last_login`),
  KEY `admin` (`admin`),
  FULLTEXT KEY `name` (`name`),
  FULLTEXT KEY `name_2` (`name`,`username`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_users_entity`
--

LOCK TABLES `elgg_users_entity` WRITE;
/*!40000 ALTER TABLE `elgg_users_entity` DISABLE KEYS */;
INSERT INTO `elgg_users_entity` VALUES (37,'foobar','foobar','','','$2y$10$aMEZ2Q12c7WQ5DGeIcTRk.04GJFRh9dPitaTb7imt3gFdiPs7v3re','foo@bar.com','en','no','yes',1426800127,1426800127,1426800120,1426799709);
/*!40000 ALTER TABLE `elgg_users_entity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_users_remember_me_cookies`
--

DROP TABLE IF EXISTS `elgg_users_remember_me_cookies`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_users_remember_me_cookies` (
  `code` varchar(32) NOT NULL,
  `guid` bigint(20) unsigned NOT NULL,
  `timestamp` int(11) unsigned NOT NULL,
  PRIMARY KEY (`code`),
  KEY `timestamp` (`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_users_remember_me_cookies`
--

LOCK TABLES `elgg_users_remember_me_cookies` WRITE;
/*!40000 ALTER TABLE `elgg_users_remember_me_cookies` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_users_remember_me_cookies` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `elgg_users_sessions`
--

DROP TABLE IF EXISTS `elgg_users_sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `elgg_users_sessions` (
  `session` varchar(255) NOT NULL,
  `ts` int(11) unsigned NOT NULL DEFAULT '0',
  `data` mediumblob,
  PRIMARY KEY (`session`),
  KEY `ts` (`ts`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `elgg_users_sessions`
--

LOCK TABLES `elgg_users_sessions` WRITE;
/*!40000 ALTER TABLE `elgg_users_sessions` DISABLE KEYS */;
/*!40000 ALTER TABLE `elgg_users_sessions` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2015-03-19 17:22:13
