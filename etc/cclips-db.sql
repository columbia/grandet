-- MySQL dump 10.13  Distrib 5.5.30, for Linux (x86_64)
--
-- Host: localhost    Database: magento
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
-- Table structure for table `cc_categories`
--

USE cclips;

DROP TABLE IF EXISTS `cc_categories`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_categories` (
  `category_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(252) NOT NULL,
  `slug` varchar(255) NOT NULL,
  PRIMARY KEY (`category_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_categories`
--

LOCK TABLES `cc_categories` WRITE;
/*!40000 ALTER TABLE `cc_categories` DISABLE KEYS */;
INSERT INTO `cc_categories` VALUES (1,'General','general');
/*!40000 ALTER TABLE `cc_categories` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_comments`
--

DROP TABLE IF EXISTS `cc_comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_comments` (
  `comment_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) NOT NULL DEFAULT '0',
  `video_id` bigint(20) NOT NULL,
  `parent_id` bigint(20) DEFAULT '0',
  `comments` longtext NOT NULL,
  `date_created` datetime NOT NULL,
  `status` varchar(255) NOT NULL,
  `released` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`comment_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_comments`
--

LOCK TABLES `cc_comments` WRITE;
/*!40000 ALTER TABLE `cc_comments` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_comments` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_flags`
--

DROP TABLE IF EXISTS `cc_flags`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_flags` (
  `flag_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `object_id` bigint(20) NOT NULL,
  `type` varchar(255) NOT NULL,
  `user_id` bigint(20) NOT NULL,
  `date_created` date NOT NULL,
  `status` varchar(255) DEFAULT 'pending',
  PRIMARY KEY (`flag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_flags`
--

LOCK TABLES `cc_flags` WRITE;
/*!40000 ALTER TABLE `cc_flags` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_flags` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_messages`
--

DROP TABLE IF EXISTS `cc_messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_messages` (
  `message_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) NOT NULL,
  `recipient` bigint(20) NOT NULL DEFAULT '0',
  `subject` text NOT NULL,
  `message` text NOT NULL,
  `status` varchar(255) NOT NULL,
  `date_created` datetime NOT NULL,
  PRIMARY KEY (`message_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_messages`
--

LOCK TABLES `cc_messages` WRITE;
/*!40000 ALTER TABLE `cc_messages` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_messages` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_pages`
--

DROP TABLE IF EXISTS `cc_pages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_pages` (
  `page_id` bigint(11) NOT NULL AUTO_INCREMENT,
  `title` text NOT NULL,
  `content` longtext NOT NULL,
  `slug` text NOT NULL,
  `layout` varchar(255) NOT NULL DEFAULT 'default',
  `date_created` datetime NOT NULL,
  `status` varchar(255) NOT NULL,
  PRIMARY KEY (`page_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_pages`
--

LOCK TABLES `cc_pages` WRITE;
/*!40000 ALTER TABLE `cc_pages` DISABLE KEYS */;
INSERT INTO `cc_pages` VALUES (1,'Sample Page','<p>This is a sample page. You can create custom static pages like this in the Admin Panel.</p>','sample-page','default','2011-09-05 06:28:49','published');
/*!40000 ALTER TABLE `cc_pages` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_playlist_entries`
--

DROP TABLE IF EXISTS `cc_playlist_entries`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_playlist_entries` (
  `playlist_entry_id` int(11) NOT NULL AUTO_INCREMENT,
  `playlist_id` int(11) NOT NULL,
  `video_id` int(11) NOT NULL,
  `date_created` date NOT NULL,
  PRIMARY KEY (`playlist_entry_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_playlist_entries`
--

LOCK TABLES `cc_playlist_entries` WRITE;
/*!40000 ALTER TABLE `cc_playlist_entries` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_playlist_entries` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_playlists`
--

DROP TABLE IF EXISTS `cc_playlists`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_playlists` (
  `playlist_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  `user_id` int(11) NOT NULL,
  `type` varchar(20) NOT NULL DEFAULT 'list',
  `public` tinyint(1) DEFAULT '1',
  `date_created` date NOT NULL,
  PRIMARY KEY (`playlist_id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_playlists`
--

LOCK TABLES `cc_playlists` WRITE;
/*!40000 ALTER TABLE `cc_playlists` DISABLE KEYS */;
INSERT INTO `cc_playlists` VALUES (1,NULL,1,'favorites',0,'2015-03-11'),(2,NULL,1,'watch_later',0,'2015-03-11');
/*!40000 ALTER TABLE `cc_playlists` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_privacy`
--

DROP TABLE IF EXISTS `cc_privacy`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_privacy` (
  `privacy_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) NOT NULL,
  `video_comment` tinyint(1) NOT NULL DEFAULT '1',
  `new_message` tinyint(1) NOT NULL DEFAULT '1',
  `new_video` tinyint(1) NOT NULL DEFAULT '1',
  `video_ready` tinyint(1) NOT NULL DEFAULT '1',
  `comment_reply` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`privacy_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_privacy`
--

LOCK TABLES `cc_privacy` WRITE;
/*!40000 ALTER TABLE `cc_privacy` DISABLE KEYS */;
INSERT INTO `cc_privacy` VALUES (1,1,1,1,1,1,1);
/*!40000 ALTER TABLE `cc_privacy` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_ratings`
--

DROP TABLE IF EXISTS `cc_ratings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_ratings` (
  `rating_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `video_id` bigint(20) NOT NULL,
  `user_id` bigint(20) NOT NULL,
  `date_created` datetime NOT NULL,
  `rating` tinyint(4) NOT NULL,
  PRIMARY KEY (`rating_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_ratings`
--

LOCK TABLES `cc_ratings` WRITE;
/*!40000 ALTER TABLE `cc_ratings` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_ratings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_settings`
--

DROP TABLE IF EXISTS `cc_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_settings` (
  `setting_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `value` longtext NOT NULL,
  PRIMARY KEY (`setting_id`)
) ENGINE=InnoDB AUTO_INCREMENT=39 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_settings`
--

LOCK TABLES `cc_settings` WRITE;
/*!40000 ALTER TABLE `cc_settings` DISABLE KEYS */;
INSERT INTO `cc_settings` VALUES (1,'active_theme','default'),(2,'active_mobile_theme','mobile-default'),(3,'default_language','english'),(4,'active_languages','[{\"system_name\":\"english\",\"lang_name\":\"English\",\"native_name\":\"English\"}]'),(5,'installed_plugins','[]'),(6,'enabled_plugins','[]'),(7,'roles','{\"admin\":{\"name\":\"Administrator\",\"permissions\":[\"admin_panel\"]},\"user\":{\"name\":\"User\",\"permissions\":[]}}'),(8,'debug_conversion','1'),(9,'video_size_limit','102000000'),(10,'keep_original_video','1'),(11,'enable_encoding','1'),(12,'mobile_site','1'),(13,'h264_encoding_options','-vcodec libx264 -vf \"scale=min(640\\,iw):trunc(ow/a/2)*2\" -vb 800k -acodec aac -strict -2 -ab 96k -ar 44100 -f mp4'),(14,'webm_encoding_enabled','0'),(15,'webm_encoding_options','-vcodec libvpx -vf \"scale=min(640\\,iw):trunc(ow/a/2)*2\" -vb 800k -acodec libvorbis -ab 96k -ar 44100 -f webm'),(16,'theora_encoding_enabled','0'),(17,'theora_encoding_options','-vcodec libtheora -vf \"scale=min(640\\,iw):trunc(ow/a/2)*2\" -qscale 8 -vb 800k -acodec libvorbis -ab 96k -ar 44100 -f ogg'),(18,'mobile_encoding_enabled','1'),(19,'mobile_encoding_options','-vcodec libx264 -vf \"scale=min(480\\,iw):trunc(ow/a/2)*2\" -vb 600k -ac 2 -ab 96k -strict -2 -ar 44100 -f mp4'),(20,'thumb_encoding_options','-vf \"scale=min(640\\,iw):trunc(ow/a/2)*2\" -t 1 -r 1 -f mjpeg'),(21,'auto_approve_users','1'),(22,'auto_approve_videos','1'),(23,'auto_approve_comments','1'),(24,'alerts_videos','1'),(25,'alerts_comments','1'),(26,'alerts_users','1'),(27,'alerts_flags','1'),(28,'from_name',''),(29,'from_address',''),(30,'smtp','{\"enabled\":false,\"host\":\"\",\"port\":25,\"username\":\"\",\"password\":\"\"}'),(31,'base_url','http://HOST_NAME'),(32,'secret_key','b7f032a6e8ff0e07db8d1309eb8a54a0'),(33,'sitename','foo site'),(34,'admin_email','foo@bar.com'),(35,'enable_uploads','1'),(36,'ffmpeg','/sbin/ffmpeg'),(37,'php','/usr/bin/php'),(38,'version','2.1.0');
/*!40000 ALTER TABLE `cc_settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_subscriptions`
--

DROP TABLE IF EXISTS `cc_subscriptions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_subscriptions` (
  `subscription_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) NOT NULL,
  `member` bigint(20) NOT NULL,
  `date_created` datetime NOT NULL,
  PRIMARY KEY (`subscription_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_subscriptions`
--

LOCK TABLES `cc_subscriptions` WRITE;
/*!40000 ALTER TABLE `cc_subscriptions` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_subscriptions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_users`
--

DROP TABLE IF EXISTS `cc_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_users` (
  `user_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `username` varchar(80) NOT NULL,
  `email` varchar(80) NOT NULL,
  `password` varchar(32) NOT NULL,
  `status` varchar(255) NOT NULL,
  `role` varchar(255) NOT NULL DEFAULT 'user',
  `date_created` date NOT NULL,
  `first_name` varchar(255) DEFAULT NULL,
  `last_name` varchar(255) DEFAULT NULL,
  `about_me` text,
  `website` text,
  `confirm_code` varchar(255) DEFAULT NULL,
  `views` bigint(20) DEFAULT '0',
  `last_login` date DEFAULT NULL,
  `avatar` varchar(255) DEFAULT NULL,
  `released` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  KEY `username` (`username`),
  KEY `email` (`email`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_users`
--

LOCK TABLES `cc_users` WRITE;
/*!40000 ALTER TABLE `cc_users` DISABLE KEYS */;
INSERT INTO `cc_users` VALUES (1,'foo','foo@bar.com','37b51d194a7513e45b56f6524f2d51f2','active','admin','2015-03-11',NULL,NULL,NULL,NULL,NULL,1,'1969-12-31',NULL,1);
/*!40000 ALTER TABLE `cc_users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `cc_videos`
--

DROP TABLE IF EXISTS `cc_videos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cc_videos` (
  `video_id` bigint(20) NOT NULL AUTO_INCREMENT,
  `filename` varchar(255) NOT NULL,
  `user_id` bigint(20) NOT NULL,
  `category_id` bigint(20) NOT NULL,
  `title` text NOT NULL,
  `description` text NOT NULL,
  `tags` text NOT NULL,
  `date_created` datetime NOT NULL,
  `duration` varchar(255) DEFAULT NULL,
  `status` varchar(255) NOT NULL,
  `views` bigint(20) NOT NULL DEFAULT '0',
  `featured` tinyint(1) NOT NULL DEFAULT '0',
  `original_extension` varchar(255) DEFAULT NULL,
  `job_id` bigint(20) DEFAULT NULL,
  `released` tinyint(1) NOT NULL DEFAULT '0',
  `disable_embed` tinyint(1) NOT NULL DEFAULT '0',
  `gated` tinyint(1) NOT NULL DEFAULT '0',
  `private` tinyint(1) NOT NULL DEFAULT '0',
  `private_url` varchar(255) DEFAULT NULL,
  `comments_closed` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`video_id`),
  KEY `user_id` (`user_id`),
  FULLTEXT KEY `title_description_tags` (`title`,`description`,`tags`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cc_videos`
--

LOCK TABLES `cc_videos` WRITE;
/*!40000 ALTER TABLE `cc_videos` DISABLE KEYS */;
/*!40000 ALTER TABLE `cc_videos` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2015-03-11 17:57:24
