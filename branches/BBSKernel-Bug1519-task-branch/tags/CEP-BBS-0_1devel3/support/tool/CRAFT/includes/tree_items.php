<?php

	session_start();

  include("type_object.php");
  $Types_Objecten = array();

	//functie om alle extra velden uit de database te lezen en deze op te slaan 
  function Extra_Velden_Lijst() {
  	$Collectie = array();
  	$query = "SELECT Kolom_ID, Veld_Naam FROM extra_velden WHERE Type_Beschrijving = '-1'";
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Kolom_ID'],$huidige_level['Veld_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }
		
	//functie om alle externe contacten uit de database te lezen en deze op te slaan 
  function Locaties_Lijst() {
  	$Collectie = array();
  	$query = 'SELECT Locatie_ID, Loc_Naam FROM comp_locatie';
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Locatie_ID'],$huidige_level['Loc_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	//if ($num_rows > 0) $Comp_Type->Add(Contacten_Lijst($huidige_level['Contact_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }


	//functie om alle gebruikers uit de database te lezen en deze hierarchisch (onder hun groep) op te slaan 
  function Gebruikersgroepen_Lijst($parent) {
  	$Collectie = array();
  	$query = 'SELECT Groep_ID, Groeps_Naam FROM gebruikers_groepen WHERE Groep_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Groep_ID'],$huidige_level['Groeps_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Comp_Type->Add(Gebruikersgroepen_Lijst($huidige_level['Groep_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }
  

	//functie om alle gebruikers uit de database te lezen en deze hierarchisch (onder hun groep) op te slaan 
  function Gebruikers_Lijst() {
  	$Collectie = array();
  	$query = 'SELECT Werknem_ID, inlognaam FROM gebruiker';
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Werknem_ID'],$huidige_level['inlognaam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	//if ($num_rows > 0) $Comp_Type->Add(Contacten_Lijst($huidige_level['Contact_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }


	//functie om alle externe contacten uit de database te lezen en deze hierarchisch op te slaan 
  function Contacten_Lijst($parent) {
  	$Collectie = array();
  	$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Contact_ID'],$huidige_level['Contact_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Comp_Type->Add(Contacten_Lijst($huidige_level['Contact_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }


	//functie om alle type meldingen uit de database te lezen en deze hierarchisch op te slaan 
  function Melding_Type_Lijst() {
  	$Collectie = array();
  	$query = 'SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type';
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Meld_Type_ID'],$huidige_level['Melding_Type_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	//if ($num_rows > 0) $Comp_Type->Add(Comp_Lijst($huidige_level['Meld_Type_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }
  
  
  //functie om alle componenten uit de database te lezen en deze hierarchisch op te slaan
  function Comp_Lijst($parent) {
  	$Collectie = array();
  	$query = 'SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Parent = '.$parent;

  	//$query = 'SELECT a.Comp_Lijst_ID, a.Comp_Naam FROM comp_lijst a, comp_type b WHERE a.Comp_Type_ID = b.Comp_Type AND a.Comp_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Comp_Lijst_ID'],$huidige_level['Comp_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Comp_Type->Add(Comp_Lijst($huidige_level['Comp_Lijst_ID']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }
  
	//functie om alle type componenten uit de database te lezen en deze hierarchisch op te slaan 
  function Comp_Type_Lijst($parent) {
  	$Collectie = array();
  	$query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Comp_Type'],$huidige_level['Type_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Comp_Type->Add(Comp_Type_Lijst($huidige_level['Comp_Type']));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
  	}  
  	return $Collectie;	
  }
  
  //functie om alle componenten uit de database te lezen en deze hierarchisch op te slaan
  function Historische_Comp_Lijst($parent, $datum) {
	
  	$Collectie = array();
 		$query = "SELECT c.Comp_Lijst_ID, c.Comp_Naam FROM comp_lijst c, melding_lijst m WHERE c.Comp_Lijst_ID = m.Comp_Lijst_ID AND m.Voorgaande_Melding = 1 AND m.Meld_Datum < '".$datum."' AND c.Comp_Parent = '".$parent ."'";

	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
  		$Comp_Type = new Type_Object();
  		$Comp_Type->Set_ID($huidige_level['Comp_Lijst_ID'],$huidige_level['Comp_Naam']);
	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Comp_Type->Add(Historische_Comp_Lijst($huidige_level['Comp_Lijst_ID'], $datum));
 	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
  	return $Collectie;	
  }  
  
  function Recursieve_Uitlees_Methode($type_object) {
  	$uitkomst = '';
  	for ($i = 0; $i < count($type_object);$i++) {
			$uitkomst = $uitkomst . "['". $type_object[$i]->Get_Naam()."', ";

			//geen c maar n wanneer p = 3 en type_overzicht = 2
			if (isset($_SESSION['main_deel']) && isset($_SESSION['type_overzicht']) && $_SESSION['main_deel'] == 3 && $_SESSION['type_overzicht'] == 2)
				$uitkomst = $uitkomst . "'". $_SESSION['huidige_pagina']. "&b=". $type_object[$i]->Get_ID() ."'";
			else
				$uitkomst = $uitkomst . "'". $_SESSION['huidige_pagina']. "&c=". $type_object[$i]->Get_ID() ."'";

  		$temp = $type_object[$i]->Get_Childarray();
			if (count($temp) > 0) $uitkomst = $uitkomst . ", ";
  		$uitkomst = $uitkomst . Recursieve_Uitlees_Methode($temp);
    	$uitkomst = $uitkomst . "],";
  	}
  	return $uitkomst;
  }
  
  //Functie welke bepaalt wat voor componenten zichtbaar zijn voor de ingelogde gebruiker
  function Bepaal_Historische_Comp_Lijst($datum) {
  	$Collectie = array();

  	$query = "SELECT Comp_Type_ID, onderliggende_Data FROM gebruikersgroeprechten WHERE Groep_ID = '". $_SESSION['groep_id'] ."'";
  	$result = mysql_query($query);
		$row = mysql_fetch_array($result);
  	
		if ($row['Comp_Type_ID'] != 1) {
	//  	$query = 'SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Parent = '.$parent ." AND Voorgaande_Melding = 1 AND Meld_Datum < '".$datum."'";
			$query = "SELECT c.Comp_Lijst_ID, c.Comp_Naam FROM comp_lijst c, melding_lijst m WHERE c.Comp_Lijst_ID = m.Comp_Lijst_ID AND m.Voorgaande_Melding = 1 AND m.Meld_Datum < '".$datum."' AND c.Comp_Type_ID = '".$row['Comp_Type_ID']."'";

  		$resultaat = mysql_query($query);
    	while ($data = mysql_fetch_array($resultaat)) {
				$Comp_Type = new Type_Object();
				$Comp_Type->Set_ID($data['Comp_Lijst_ID'],$data['Comp_Naam']);
		  	if ($row['onderliggende_Data'] == 1) {
		  		$num_rows = mysql_num_rows(mysql_query($query));		
		  		if ($num_rows > 0) $Comp_Type->Add(Historische_Comp_Lijst($data['Comp_Lijst_ID'], $datum));
		  	}
	  		array_push($Collectie, $Comp_Type);
	  		$Comp_Type = NULL;
    	}
	  }
	  else $Collectie = Historische_Comp_Lijst(1, $datum);
	  return $Collectie;
  }
  
  //Functie welke bepaalt wat voor componenten zichtbaar zijn voor de ingelogde gebruiker
  function Bepaal_Comp_Lijst() {
  	$Collectie = array();

  	$query = "SELECT Comp_Type_ID, onderliggende_Data FROM gebruikersgroeprechten WHERE Groep_ID = '". $_SESSION['groep_id'] ."'";
  	$result = mysql_query($query);
		$row = mysql_fetch_array($result);
  	
		if ($row['Comp_Type_ID'] != 1) {
	  	$query = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID = '".$row['Comp_Type_ID']."'";
  		$resultaat = mysql_query($query);
    	while ($data = mysql_fetch_array($resultaat)) {
				$Comp_Type = new Type_Object();
				$Comp_Type->Set_ID($data['Comp_Lijst_ID'],$data['Comp_Naam']);
		  	if ($row['onderliggende_Data'] == 1) {
		  		$num_rows = mysql_num_rows(mysql_query($query));		
		  		if ($num_rows > 0) $Comp_Type->Add(Comp_Lijst($data['Comp_Lijst_ID']));
		  	}
	  		array_push($Collectie, $Comp_Type);
	  		$Comp_Type = NULL;
    	}
	  }
	  else $Collectie = Comp_Lijst(1);
	  return $Collectie;
  }
  
  //Functie welke bepaalt wat voor types componenten zichtbaar zijn voor de ingelogde gebruiker
  function Bepaal_Comp_Type_Lijst() {
  	$Collectie = array();

  	$query = "SELECT Comp_Type_ID, onderliggende_Data FROM gebruikersgroeprechten WHERE Groep_ID = '". $_SESSION['groep_id'] ."'";
  	$result = mysql_query($query);
		$rechten = mysql_fetch_array($result);

  	$query = "SELECT Comp_Type, Type_Naam FROM comp_type WHERE Comp_Type = '".$rechten['Comp_Type_ID']."'";
  	$result = mysql_query($query);
		$row = mysql_fetch_array($result);

		if ($row['Comp_Type'] != 1) {
			$Comp_Type = new Type_Object();
			$Comp_Type->Set_ID($row['Comp_Type'],$row['Type_Naam']);
	  	$query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent = '.$row['Comp_Type'];
	  	if($rechten['onderliggende_Data'] == 1) {
	  		$num_rows = mysql_num_rows(mysql_query($query));		
		  	if ($num_rows > 0) $Comp_Type->Add(Comp_Type_Lijst($row['Comp_Type']));
		  }
	  	array_push($Collectie, $Comp_Type);
	  	$Comp_Type = NULL;
	  }
	  else $Collectie = Comp_Type_Lijst(1);
	  return $Collectie;
  }
  
  include("vars.php");
  
  $query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent = 1';
  $rest = mysql_query($query);
  
  //Component types
  if ($_SESSION['admin_deel'] == 1)      $Types_Objecten = Bepaal_Comp_Type_Lijst();
  //componenten
  else if ($_SESSION['admin_deel'] == 2) $Types_Objecten = Bepaal_Comp_Lijst();
  //type meldingen
  else if ($_SESSION['admin_deel'] == 3) $Types_Objecten = Melding_Type_Lijst();
	//meldingen
	else if ($_SESSION['admin_deel'] == 4) $Types_Objecten = Bepaal_Comp_Lijst();
	//extra velden
	else if ($_SESSION['admin_deel'] == 5) $Types_Objecten = Extra_Velden_Lijst();
	//gebruikersgroepen
	else if ($_SESSION['admin_deel'] == 6) $Types_Objecten = Gebruikersgroepen_Lijst(1);
	//gebruikers
	else if ($_SESSION['admin_deel'] == 7) $Types_Objecten = Gebruikers_Lijst();
	//contacten
	else if ($_SESSION['admin_deel'] == 8) $Types_Objecten = Contacten_Lijst(1);
	//Locaties
	else if ($_SESSION['admin_deel'] == 9) $Types_Objecten = Locaties_Lijst();
  //Het admin scherm is niet geselecteerd...
  else if ($_SESSION['admin_deel'] == 0) {
  	//componenten
  	if ($_SESSION['main_deel'] == 2) {
  		if ($_SESSION['type_overzicht'] == 1) 		 $Types_Objecten = Bepaal_Comp_Lijst();
  		else if ($_SESSION['type_overzicht'] == 2) $Types_Objecten = Bepaal_Comp_Type_Lijst();
  	}
  	//meldingen
  	else if ($_SESSION['main_deel'] == 3) {
  		if ($_SESSION['type_overzicht'] == 1) 		 $Types_Objecten = Bepaal_Comp_Lijst();
  		else if ($_SESSION['type_overzicht'] == 2) $Types_Objecten = Melding_Type_Lijst();
  	}
  	//statistieken
  	else if ($_SESSION['main_deel'] == 4) {
  		if ($_SESSION['type_overzicht'] == 1) 		 $Types_Objecten = Bepaal_Comp_Type_Lijst();
  		else if ($_SESSION['type_overzicht'] == 2) $Types_Objecten = Bepaal_Comp_Lijst();
  		else if ($_SESSION['type_overzicht'] == 3) $Types_Objecten = Melding_Type_Lijst();
  		else $Types_Objecten = Bepaal_Historische_Comp_Lijst($_SESSION['type_overzicht']);
  	}
  }
  
	echo ("var TREE_ITEMS = [");	
	echo(Recursieve_Uitlees_Methode($Types_Objecten));
	echo ("];");

?>