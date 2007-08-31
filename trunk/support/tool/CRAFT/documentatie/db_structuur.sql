-- phpMyAdmin SQL Dump
-- version 2.10.1
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generatie Tijd: 31 Aug 2007 om 16:28
-- Server versie: 4.1.22
-- PHP Versie: 5.2.2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `lofar-craft`
-- 

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `comp_koppel_extra`
-- 

DROP TABLE IF EXISTS `comp_koppel_extra`;
CREATE TABLE IF NOT EXISTS `comp_koppel_extra` (
  `Comp_Lijst_ID` int(11) NOT NULL default '0',
  `Kolom_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Comp_Lijst_ID`,`Kolom_ID`),
  KEY `Kolom_ID` (`Kolom_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `comp_koppel_extra`
-- 

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `comp_koppel_regels`
-- 

DROP TABLE IF EXISTS `comp_koppel_regels`;
CREATE TABLE IF NOT EXISTS `comp_koppel_regels` (
  `Regels_ID` int(11) NOT NULL default '0',
  `Comp_Lijst_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Regels_ID`,`Comp_Lijst_ID`),
  KEY `Comp_Lijst_ID` (`Comp_Lijst_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `comp_koppel_regels`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `comp_lijst`
-- 

DROP TABLE IF EXISTS `comp_lijst`;
CREATE TABLE IF NOT EXISTS `comp_lijst` (
  `Comp_Lijst_ID` int(11) NOT NULL auto_increment,
  `Comp_Naam` tinytext,
  `Comp_Type_ID` int(11) default NULL,
  `Comp_Parent` int(11) default NULL,
  `Comp_Locatie` int(11) default NULL,
  `Comp_Verantwoordelijke` int(11) default NULL,
  `Contact_Fabricant` int(11) default NULL,
  `Contact_Leverancier` int(11) default NULL,
  `Lever_Datum` datetime default NULL,
  `Fabricatie_Datum` datetime default NULL,
  `Laatste_Melding` int(11) NOT NULL default '0',
  `Schaduw_Vlag` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`Comp_Lijst_ID`),
  KEY `Comp_Type_ID` (`Comp_Type_ID`),
  KEY `Comp_Locatie` (`Comp_Locatie`),
  KEY `Comp_Verantwoordelijke` (`Comp_Verantwoordelijke`),
  KEY `Contact_Fabricant` (`Contact_Fabricant`),
  KEY `Contact_Leverancier` (`Contact_Leverancier`),
  KEY `Comp_Parent` (`Comp_Parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=162 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `comp_lijst`
-- 

INSERT INTO `comp_lijst` (`Comp_Lijst_ID`, `Comp_Naam`, `Comp_Type_ID`, `Comp_Parent`, `Comp_Locatie`, `Comp_Verantwoordelijke`, `Contact_Fabricant`, `Contact_Leverancier`, `Lever_Datum`, `Fabricatie_Datum`, `Laatste_Melding`, `Schaduw_Vlag`) VALUES 
(1, 'ASTRON', 1, NULL, 1, 1, NULL, NULL, NULL, NULL, 1, 0);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `comp_locatie`
-- 

DROP TABLE IF EXISTS `comp_locatie`;
CREATE TABLE IF NOT EXISTS `comp_locatie` (
  `Locatie_ID` int(11) NOT NULL auto_increment,
  `Loc_Naam` text,
  `Loc_Adres1` text,
  `Loc_Adres2` text,
  `Loc_Postcode` text,
  `Loc_Plaats` text,
  `Long_Graden` int(11) default NULL,
  `Long_Min` int(11) default NULL,
  `Long_Sec` int(11) default NULL,
  `Lat_Graden` int(11) default NULL,
  `Lat_Min` int(11) default NULL,
  `Lat_Sec` int(11) default NULL,
  PRIMARY KEY  (`Locatie_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=24 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `comp_locatie`
-- 

INSERT INTO `comp_locatie` (`Locatie_ID`, `Loc_Naam`, `Loc_Adres1`, `Loc_Adres2`, `Loc_Postcode`, `Loc_Plaats`, `Long_Graden`, `Long_Min`, `Long_Sec`, `Lat_Graden`, `Lat_Min`, `Lat_Sec`) VALUES 
(1, 'Lhee', NULL, NULL, NULL, NULL, 6, 39, 51, 52, 81, 28),
(16, 'CS001', '', '', '', '', 0, 0, 0, 0, 0, 0),
(17, 'CS008', '', '', '', '', 0, 0, 0, 0, 0, 0),
(18, 'CS010', '', '', '', '', 0, 0, 0, 0, 0, 0),
(19, 'CS016', '', '', '', '', 0, 0, 0, 0, 0, 0),
(20, 'CS001T', '', '', '', '', 0, 0, 0, 0, 0, 0),
(21, 'RS002', '', '', '', '', 0, 0, 0, 0, 0, 0),
(22, 'ISGE01', '', '', '', '', 0, 0, 0, 0, 0, 0),
(23, 'Neways', '', '', '', 'Leeuwarden', 0, 0, 0, 0, 0, 0);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `comp_type`
-- 

DROP TABLE IF EXISTS `comp_type`;
CREATE TABLE IF NOT EXISTS `comp_type` (
  `Comp_Type` int(11) NOT NULL auto_increment,
  `Type_Naam` text,
  `Type_Parent` int(11) default NULL,
  `Aangemaakt_Door` int(11) default NULL,
  `Aanmaak_Datum` datetime default NULL,
  `Structuur_Entry` tinyint(1) default NULL,
  `Gefabriceerd_Door` int(11) default NULL,
  `Geleverd_Door` int(11) default NULL,
  `Min_Aantal` int(11) default NULL,
  `Max_Aantal` int(11) default NULL,
  `Reserve_Minimum` int(11) default NULL,
  `Type_Verantwoordelijke` int(11) default NULL,
  PRIMARY KEY  (`Comp_Type`),
  KEY `Aangemaakt_Door` (`Aangemaakt_Door`),
  KEY `Type_Verantwoordelijke` (`Type_Verantwoordelijke`),
  KEY `Gefabriceerd_Door` (`Gefabriceerd_Door`),
  KEY `Geleverd_Door` (`Geleverd_Door`),
  KEY `Type_Parent` (`Type_Parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=43 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `comp_type`
-- 

INSERT INTO `comp_type` (`Comp_Type`, `Type_Naam`, `Type_Parent`, `Aangemaakt_Door`, `Aanmaak_Datum`, `Structuur_Entry`, `Gefabriceerd_Door`, `Geleverd_Door`, `Min_Aantal`, `Max_Aantal`, `Reserve_Minimum`, `Type_Verantwoordelijke`) VALUES 
(1, 'Faciliteit', NULL, 1, '2007-06-18 13:29:26', 1, 1, 1, 1, 1, 0, 1);
-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `contact`
-- 

DROP TABLE IF EXISTS `contact`;
CREATE TABLE IF NOT EXISTS `contact` (
  `Contact_ID` int(11) NOT NULL auto_increment,
  `Contact_Naam` text,
  `Contact_Adres1` text,
  `Contact_Adres2` text,
  `Contact_Postcode` text,
  `Contact_Woonplaats` text,
  `Contact_Telefoon_Vast` varchar(20) default NULL,
  `Contact_Telefoon_Mobiel` varchar(20) default NULL,
  `Contact_Email` text,
  `Contact_Fax` varchar(20) default NULL,
  `Contact_Parent` int(11) default NULL,
  `Contact_Functie` text,
  `Contact_Parent_Gegevens` tinyint(4) default '0',
  PRIMARY KEY  (`Contact_ID`),
  KEY `Contact_Parent` (`Contact_Parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=5 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `contact`
-- 

INSERT INTO `contact` (`Contact_ID`, `Contact_Naam`, `Contact_Adres1`, `Contact_Adres2`, `Contact_Postcode`, `Contact_Woonplaats`, `Contact_Telefoon_Vast`, `Contact_Telefoon_Mobiel`, `Contact_Email`, `Contact_Fax`, `Contact_Parent`, `Contact_Functie`, `Contact_Parent_Gegevens`) VALUES 
(1, 'Nieuw bedrijf toevoegen', NULL, NULL, NULL, NULL, NULL, '0', NULL, NULL, NULL, 'Structuur entry', 0),
(2, 'ASTRON', 'Oude hoogeveensedijk 4', '', '1234 aa', 'Lhee', '', '', '', '', 1, 'eerste contact', 0),
(4, 'Neways', 'Harlingerstraatweg 111', '', '8914 AZ', 'Leeuwarden', '', '', 'info@neways-leeuwarden.nl', '', 1, '', 0);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `datatabel`
-- 

DROP TABLE IF EXISTS `datatabel`;
CREATE TABLE IF NOT EXISTS `datatabel` (
  `Data_Kolom_ID` int(11) NOT NULL auto_increment,
  `Type_TinyText` tinytext,
  `Type_Integer` int(11) default NULL,
  `Type_Double` double default NULL,
  `Type_Text` text,
  `Type_DateTime` datetime default NULL,
  PRIMARY KEY  (`Data_Kolom_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=372 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `datatabel`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `extra_velden`
-- 

DROP TABLE IF EXISTS `extra_velden`;
CREATE TABLE IF NOT EXISTS `extra_velden` (
  `Kolom_ID` int(11) NOT NULL auto_increment,
  `Data_Kolom_ID` int(11) NOT NULL default '0',
  `Aangemaakt_Door` int(11) NOT NULL default '0',
  `Veld_Naam` text NOT NULL,
  `Is_verplicht` tinyint(1) NOT NULL default '0',
  `DataType` int(11) NOT NULL default '0',
  `Type_Beschrijving` int(11) NOT NULL default '0',
  `Tabel_Type` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`Kolom_ID`),
  KEY `Aangemaakt_Door` (`Aangemaakt_Door`),
  KEY `Data_Kolom_ID` (`Data_Kolom_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=190 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `extra_velden`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `gebruiker`
-- 

DROP TABLE IF EXISTS `gebruiker`;
CREATE TABLE IF NOT EXISTS `gebruiker` (
  `Werknem_ID` int(11) NOT NULL auto_increment,
  `inlognaam` text NOT NULL,
  `Wachtwoord` text NOT NULL,
  `Start_Alg` int(11) NOT NULL default '0',
  `Start_Comp` int(11) NOT NULL default '0',
  `Start_Melding` int(11) NOT NULL default '0',
  `Start_Stats` int(11) NOT NULL default '0',
  `Groep_ID` int(11) NOT NULL default '0',
  `Gebruiker_Taal` int(11) NOT NULL default '0',
  `Emailadres` text NOT NULL,
  `Laatst_Ingelogd` datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`Werknem_ID`),
  KEY `Groep_ID` (`Groep_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=20 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `gebruiker`
-- 

INSERT INTO `gebruiker` (`Werknem_ID`, `inlognaam`, `Wachtwoord`, `Start_Alg`, `Start_Comp`, `Start_Melding`, `Start_Stats`, `Groep_ID`, `Gebruiker_Taal`, `Emailadres`, `Laatst_Ingelogd`) VALUES 
(1, 'sysadmin', '21232f297a57a5a743894a0e4a801fc3', 1, 1, 1, 1, 4, 1, 'admin@astron.nl', '2007-08-27 21:14:57');

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `gebruikersgroeprechten`
-- 

DROP TABLE IF EXISTS `gebruikersgroeprechten`;
CREATE TABLE IF NOT EXISTS `gebruikersgroeprechten` (
  `Groep_ID` int(11) NOT NULL default '0',
  `Comp_Type_ID` int(11) NOT NULL default '0',
  `Zichtbaar` tinyint(1) NOT NULL default '0',
  `Toevoegen` tinyint(1) NOT NULL default '0',
  `Bewerken` tinyint(1) NOT NULL default '0',
  `Verwijderen` tinyint(1) NOT NULL default '0',
  `onderliggende_Data` tinyint(4) NOT NULL default '1',
  PRIMARY KEY  (`Groep_ID`,`Comp_Type_ID`),
  KEY `Comp_Type_ID` (`Comp_Type_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `gebruikersgroeprechten`
-- 

INSERT INTO `gebruikersgroeprechten` (`Groep_ID`, `Comp_Type_ID`, `Zichtbaar`, `Toevoegen`, `Bewerken`, `Verwijderen`, `onderliggende_Data`) VALUES 
(2, 1, 0, 0, 0, 0, 1),
(3, 1, 0, 0, 0, 0, 1),
(4, 1, 0, 0, 0, 0, 1);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `gebruikersrechten`
-- 

DROP TABLE IF EXISTS `gebruikersrechten`;
CREATE TABLE IF NOT EXISTS `gebruikersrechten` (
  `Werknem_ID` int(11) NOT NULL default '0',
  `Comp_Type_ID` int(11) NOT NULL default '0',
  `Zichtbaar` tinyint(1) NOT NULL default '0',
  `Toevoegen` tinyint(1) NOT NULL default '0',
  `Bewerken` tinyint(1) NOT NULL default '0',
  `Verwijderen` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`Werknem_ID`,`Comp_Type_ID`),
  KEY `Comp_Type_ID` (`Comp_Type_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `gebruikersrechten`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `gebruikers_groepen`
-- 

DROP TABLE IF EXISTS `gebruikers_groepen`;
CREATE TABLE IF NOT EXISTS `gebruikers_groepen` (
  `Groep_ID` int(11) NOT NULL auto_increment,
  `Groep_Parent` int(11) default NULL,
  `Groeps_Naam` text NOT NULL,
  `Intro_Zichtbaar` tinyint(1) NOT NULL default '0',
  `Comp_Zichtbaar` tinyint(1) NOT NULL default '0',
  `Melding_Zichtbaar` tinyint(1) NOT NULL default '0',
  `Stats_Zichtbaar` tinyint(1) NOT NULL default '0',
  `Instel_Zichtbaar` tinyint(1) NOT NULL default '0',
  `Toevoegen` tinyint(1) NOT NULL default '0',
  `Bewerken` tinyint(1) NOT NULL default '0',
  `Verwijderen` tinyint(1) NOT NULL default '0',
  `Vaste_gegevens` tinyint(1) NOT NULL default '0',
  `Admin_Rechten` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`Groep_ID`),
  KEY `gebruikers_groepen_ibfk_1` (`Groep_Parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=9 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `gebruikers_groepen`
-- 

INSERT INTO `gebruikers_groepen` (`Groep_ID`, `Groep_Parent`, `Groeps_Naam`, `Intro_Zichtbaar`, `Comp_Zichtbaar`, `Melding_Zichtbaar`, `Stats_Zichtbaar`, `Instel_Zichtbaar`, `Toevoegen`, `Bewerken`, `Verwijderen`, `Vaste_gegevens`, `Admin_Rechten`) VALUES 
(1, NULL, 'Toplevel', 1, 1, 1, 1, 1, 1, 1, 1, 1, 0),
(2, 1, 'Gebruiker', 0, 1, 1, 1, 0, 1, 1, 1, 1, 0),
(3, 1, 'Gast', 0, 1, 0, 0, 0, 0, 0, 0, 1, 0),
(4, 1, 'Administrator', 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `melding_koppel_extra`
-- 

DROP TABLE IF EXISTS `melding_koppel_extra`;
CREATE TABLE IF NOT EXISTS `melding_koppel_extra` (
  `Kolom_ID` int(11) NOT NULL default '0',
  `Meld_Lijst_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Kolom_ID`,`Meld_Lijst_ID`),
  KEY `Meld_Lijst_ID` (`Meld_Lijst_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `melding_koppel_extra`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `melding_koppel_regels`
-- 

DROP TABLE IF EXISTS `melding_koppel_regels`;
CREATE TABLE IF NOT EXISTS `melding_koppel_regels` (
  `Meld_Lijst_ID` int(11) NOT NULL default '0',
  `Regels_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Meld_Lijst_ID`,`Regels_ID`),
  KEY `Regels_ID` (`Regels_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `melding_koppel_regels`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `melding_lijst`
-- 

DROP TABLE IF EXISTS `melding_lijst`;
CREATE TABLE IF NOT EXISTS `melding_lijst` (
  `Meld_Lijst_ID` int(11) NOT NULL auto_increment,
  `Meld_Type_ID` int(11) default '0',
  `Comp_Lijst_ID` int(11) default '0',
  `Meld_Datum` datetime default '0000-00-00 00:00:00',
  `Gemeld_Door` int(11) default '0',
  `Huidige_Status` tinyint(11) default '0',
  `Prob_Beschrijving` text,
  `Prob_Oplossing` text,
  `Behandeld_Door` int(11) default '0',
  `Afgehandeld` tinyint(1) default '0',
  `Voorgaande_Melding` int(11) default '0',
  `Melding_Locatie` int(11) NOT NULL default '0',
  `Comp_Parent` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Meld_Lijst_ID`),
  KEY `melding_lijst_ibfk_1` (`Gemeld_Door`),
  KEY `melding_lijst_ibfk_2` (`Behandeld_Door`),
  KEY `melding_lijst_ibfk_3` (`Comp_Lijst_ID`),
  KEY `melding_lijst_ibfk_4` (`Meld_Type_ID`),
  KEY `Huidige_Status` (`Huidige_Status`),
  KEY `Voorgaande_Melding` (`Voorgaande_Melding`),
  KEY `melding_lijst_ibfk_8` (`Melding_Locatie`),
  KEY `melding_lijst_ibfk_9` (`Comp_Parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=198 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `melding_lijst`
-- 

INSERT INTO `melding_lijst` (`Meld_Lijst_ID`, `Meld_Type_ID`, `Comp_Lijst_ID`, `Meld_Datum`, `Gemeld_Door`, `Huidige_Status`, `Prob_Beschrijving`, `Prob_Oplossing`, `Behandeld_Door`, `Afgehandeld`, `Voorgaande_Melding`, `Melding_Locatie`, `Comp_Parent`) VALUES 
(1, 3, NULL, '1982-07-01 09:00:00', 1, NULL, 'Bewerkt hooraaa', 'Bewerken (dus text typen) en daarna op opslaan drukken...', 1, 0, NULL, 1, 1);

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `melding_type`
-- 

DROP TABLE IF EXISTS `melding_type`;
CREATE TABLE IF NOT EXISTS `melding_type` (
  `Meld_Type_ID` int(11) NOT NULL auto_increment,
  `Melding_Type_Naam` tinytext,
  `Huidige_Status` tinyint(11) NOT NULL default '0',
  `Algemene_Melding` tinyint(1) NOT NULL default '0',
  `Stand_Beschrijving` text,
  `Stand_Oplossing` text,
  PRIMARY KEY  (`Meld_Type_ID`),
  KEY `melding_type_ibfk_1` (`Huidige_Status`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=8 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `melding_type`
-- 

INSERT INTO `melding_type` (`Meld_Type_ID`, `Melding_Type_Naam`, `Huidige_Status`, `Algemene_Melding`, `Stand_Beschrijving`, `Stand_Oplossing`) VALUES 
(1, 'Plaatsing component', 1, 1, 'Het plaatsen van het component', NULL),
(2, 'Verplaatsen component', 1, 1, 'Component verplaatst en hoe ;)', ''),
(3, 'Bewerken component', 1, 1, 'Gegevens van het component zijn bewerkt', 'Bewerken (dus text typen) en daarna op opslaan drukken...');
-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `prog_data`
-- 

DROP TABLE IF EXISTS `prog_data`;
CREATE TABLE IF NOT EXISTS `prog_data` (
  `ID` int(11) NOT NULL auto_increment,
  `Ned_Vert` text NOT NULL,
  `Eng_Vert` text,
  PRIMARY KEY  (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `prog_data`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `regels`
-- 

DROP TABLE IF EXISTS `regels`;
CREATE TABLE IF NOT EXISTS `regels` (
  `Regels_ID` int(11) NOT NULL auto_increment,
  `Voorwaardes` text NOT NULL,
  `Acties` text NOT NULL,
  PRIMARY KEY  (`Regels_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `regels`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `status`
-- 

DROP TABLE IF EXISTS `status`;
CREATE TABLE IF NOT EXISTS `status` (
  `Status_ID` tinyint(4) NOT NULL auto_increment,
  `Status` tinytext NOT NULL,
  PRIMARY KEY  (`Status_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=6 ;

-- 
-- Gegevens worden uitgevoerd voor tabel `status`
-- 

INSERT INTO `status` (`Status_ID`, `Status`) VALUES 
(1, 'Active'),
(2, 'Faulty'),
(3, 'In repair'),
(4, 'In stock'),
(5, 'Obsolete');

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `type_comp_koppel_extra`
-- 

DROP TABLE IF EXISTS `type_comp_koppel_extra`;
CREATE TABLE IF NOT EXISTS `type_comp_koppel_extra` (
  `Kolom_ID` int(11) NOT NULL default '0',
  `Comp_Type_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Kolom_ID`,`Comp_Type_ID`),
  KEY `Comp_Type_ID` (`Comp_Type_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `type_comp_koppel_extra`
-- 

-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `type_comp_koppel_regel`
-- 

DROP TABLE IF EXISTS `type_comp_koppel_regel`;
CREATE TABLE IF NOT EXISTS `type_comp_koppel_regel` (
  `Comp_Type_ID` int(11) NOT NULL default '0',
  `Regels_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Comp_Type_ID`,`Regels_ID`),
  KEY `Regels_ID` (`Regels_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `type_comp_koppel_regel`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `type_koppel`
-- 

DROP TABLE IF EXISTS `type_koppel`;
CREATE TABLE IF NOT EXISTS `type_koppel` (
  `Meld_Type_ID` int(11) NOT NULL default '0',
  `Comp_Type_ID` int(11) NOT NULL default '0',
  `Losstaand` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`Meld_Type_ID`,`Comp_Type_ID`),
  KEY `Comp_Type_ID` (`Comp_Type_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `type_koppel`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `type_melding_koppel_extra`
-- 

DROP TABLE IF EXISTS `type_melding_koppel_extra`;
CREATE TABLE IF NOT EXISTS `type_melding_koppel_extra` (
  `Kolom_ID` int(11) NOT NULL default '0',
  `Meld_Type_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Kolom_ID`,`Meld_Type_ID`),
  KEY `Meld_Type_ID` (`Meld_Type_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `type_melding_koppel_extra`
-- 


-- --------------------------------------------------------

-- 
-- Tabel structuur voor tabel `type_melding_regels`
-- 

DROP TABLE IF EXISTS `type_melding_regels`;
CREATE TABLE IF NOT EXISTS `type_melding_regels` (
  `Meld_Type_ID` int(11) NOT NULL default '0',
  `Regels_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Meld_Type_ID`,`Regels_ID`),
  KEY `Regels_ID` (`Regels_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- 
-- Gegevens worden uitgevoerd voor tabel `type_melding_regels`
-- 


-- 
-- Beperkingen voor gedumpte tabellen
-- 

-- 
-- Beperkingen voor tabel `comp_koppel_extra`
-- 
ALTER TABLE `comp_koppel_extra`
  ADD CONSTRAINT `comp_koppel_extra_ibfk_1` FOREIGN KEY (`Comp_Lijst_ID`) REFERENCES `comp_lijst` (`Comp_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_koppel_extra_ibfk_2` FOREIGN KEY (`Kolom_ID`) REFERENCES `extra_velden` (`Kolom_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `comp_koppel_regels`
-- 
ALTER TABLE `comp_koppel_regels`
  ADD CONSTRAINT `comp_koppel_regels_ibfk_1` FOREIGN KEY (`Regels_ID`) REFERENCES `regels` (`Regels_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_koppel_regels_ibfk_2` FOREIGN KEY (`Comp_Lijst_ID`) REFERENCES `comp_lijst` (`Comp_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `comp_lijst`
-- 
ALTER TABLE `comp_lijst`
  ADD CONSTRAINT `comp_lijst_ibfk_1` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_lijst_ibfk_3` FOREIGN KEY (`Comp_Locatie`) REFERENCES `comp_locatie` (`Locatie_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_lijst_ibfk_4` FOREIGN KEY (`Comp_Verantwoordelijke`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_lijst_ibfk_5` FOREIGN KEY (`Contact_Fabricant`) REFERENCES `contact` (`Contact_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_lijst_ibfk_6` FOREIGN KEY (`Contact_Leverancier`) REFERENCES `contact` (`Contact_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_lijst_ibfk_7` FOREIGN KEY (`Comp_Parent`) REFERENCES `comp_lijst` (`Comp_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `comp_type`
-- 
ALTER TABLE `comp_type`
  ADD CONSTRAINT `comp_type_ibfk_1` FOREIGN KEY (`Aangemaakt_Door`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_type_ibfk_2` FOREIGN KEY (`Type_Verantwoordelijke`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_type_ibfk_3` FOREIGN KEY (`Gefabriceerd_Door`) REFERENCES `contact` (`Contact_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_type_ibfk_4` FOREIGN KEY (`Geleverd_Door`) REFERENCES `contact` (`Contact_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `comp_type_ibfk_5` FOREIGN KEY (`Type_Parent`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `contact`
-- 
ALTER TABLE `contact`
  ADD CONSTRAINT `contact_ibfk_1` FOREIGN KEY (`Contact_Parent`) REFERENCES `contact` (`Contact_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `extra_velden`
-- 
ALTER TABLE `extra_velden`
  ADD CONSTRAINT `extra_velden_ibfk_1` FOREIGN KEY (`Aangemaakt_Door`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `extra_velden_ibfk_2` FOREIGN KEY (`Data_Kolom_ID`) REFERENCES `datatabel` (`Data_Kolom_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `gebruiker`
-- 
ALTER TABLE `gebruiker`
  ADD CONSTRAINT `gebruiker_ibfk_1` FOREIGN KEY (`Groep_ID`) REFERENCES `gebruikers_groepen` (`Groep_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `gebruikersgroeprechten`
-- 
ALTER TABLE `gebruikersgroeprechten`
  ADD CONSTRAINT `gebruikersgroeprechten_ibfk_1` FOREIGN KEY (`Groep_ID`) REFERENCES `gebruikers_groepen` (`Groep_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `gebruikersgroeprechten_ibfk_2` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `gebruikersrechten`
-- 
ALTER TABLE `gebruikersrechten`
  ADD CONSTRAINT `gebruikersrechten_ibfk_1` FOREIGN KEY (`Werknem_ID`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `gebruikersrechten_ibfk_2` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `gebruikers_groepen`
-- 
ALTER TABLE `gebruikers_groepen`
  ADD CONSTRAINT `gebruikers_groepen_ibfk_1` FOREIGN KEY (`Groep_Parent`) REFERENCES `gebruikers_groepen` (`Groep_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `melding_koppel_extra`
-- 
ALTER TABLE `melding_koppel_extra`
  ADD CONSTRAINT `melding_koppel_extra_ibfk_1` FOREIGN KEY (`Kolom_ID`) REFERENCES `extra_velden` (`Kolom_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_koppel_extra_ibfk_2` FOREIGN KEY (`Meld_Lijst_ID`) REFERENCES `melding_lijst` (`Meld_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `melding_koppel_regels`
-- 
ALTER TABLE `melding_koppel_regels`
  ADD CONSTRAINT `melding_koppel_regels_ibfk_1` FOREIGN KEY (`Meld_Lijst_ID`) REFERENCES `melding_lijst` (`Meld_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_koppel_regels_ibfk_2` FOREIGN KEY (`Regels_ID`) REFERENCES `regels` (`Regels_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `melding_lijst`
-- 
ALTER TABLE `melding_lijst`
  ADD CONSTRAINT `melding_lijst_ibfk_9` FOREIGN KEY (`Comp_Parent`) REFERENCES `comp_lijst` (`Comp_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_1` FOREIGN KEY (`Gemeld_Door`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_2` FOREIGN KEY (`Behandeld_Door`) REFERENCES `gebruiker` (`Werknem_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_3` FOREIGN KEY (`Comp_Lijst_ID`) REFERENCES `comp_lijst` (`Comp_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_4` FOREIGN KEY (`Meld_Type_ID`) REFERENCES `melding_type` (`Meld_Type_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_6` FOREIGN KEY (`Huidige_Status`) REFERENCES `status` (`Status_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_7` FOREIGN KEY (`Voorgaande_Melding`) REFERENCES `melding_lijst` (`Meld_Lijst_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `melding_lijst_ibfk_8` FOREIGN KEY (`Melding_Locatie`) REFERENCES `comp_locatie` (`Locatie_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `melding_type`
-- 
ALTER TABLE `melding_type`
  ADD CONSTRAINT `melding_type_ibfk_1` FOREIGN KEY (`Huidige_Status`) REFERENCES `status` (`Status_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `type_comp_koppel_extra`
-- 
ALTER TABLE `type_comp_koppel_extra`
  ADD CONSTRAINT `type_comp_koppel_extra_ibfk_1` FOREIGN KEY (`Kolom_ID`) REFERENCES `extra_velden` (`Kolom_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `type_comp_koppel_extra_ibfk_2` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `type_comp_koppel_regel`
-- 
ALTER TABLE `type_comp_koppel_regel`
  ADD CONSTRAINT `type_comp_koppel_regel_ibfk_1` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `type_comp_koppel_regel_ibfk_2` FOREIGN KEY (`Regels_ID`) REFERENCES `regels` (`Regels_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `type_koppel`
-- 
ALTER TABLE `type_koppel`
  ADD CONSTRAINT `type_koppel_ibfk_1` FOREIGN KEY (`Meld_Type_ID`) REFERENCES `melding_type` (`Meld_Type_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `type_koppel_ibfk_2` FOREIGN KEY (`Comp_Type_ID`) REFERENCES `comp_type` (`Comp_Type`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `type_melding_koppel_extra`
-- 
ALTER TABLE `type_melding_koppel_extra`
  ADD CONSTRAINT `type_melding_koppel_extra_ibfk_1` FOREIGN KEY (`Kolom_ID`) REFERENCES `extra_velden` (`Kolom_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `type_melding_koppel_extra_ibfk_2` FOREIGN KEY (`Meld_Type_ID`) REFERENCES `melding_type` (`Meld_Type_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;

-- 
-- Beperkingen voor tabel `type_melding_regels`
-- 
ALTER TABLE `type_melding_regels`
  ADD CONSTRAINT `type_melding_regels_ibfk_1` FOREIGN KEY (`Meld_Type_ID`) REFERENCES `melding_type` (`Meld_Type_ID`) ON DELETE NO ACTION ON UPDATE CASCADE,
  ADD CONSTRAINT `type_melding_regels_ibfk_2` FOREIGN KEY (`Regels_ID`) REFERENCES `regels` (`Regels_ID`) ON DELETE NO ACTION ON UPDATE CASCADE;