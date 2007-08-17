<?php

	//functie welke het geheel van het vullen van ene select lijst met component types vereenvoudigt
	//hierdoor hoeft er geen lange code meer geschreven te worden
	//$type_selectie wordt als call by reference variable meegegeven, zodat de geselecteerde type teruggegeven kan worden
	function Vul_Component_Types_Select_Box($begin_type, &$type_selectie, $toplevel) {
		//deze plaatsen en daarna dit type als parent gebruiken en zo itereren
		$query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Comp_Type = '.$begin_type;
	  $resultaat = mysql_query($query);
		$data = mysql_fetch_array($resultaat);
		if (($toplevel) || (!$toplevel && $begin_type > 1) ) {
			echo('<option value="'. $data['Comp_Type'] .'"');
			if(isset($type_selectie) && $data['Comp_Type'] == $type_selectie || $type_selectie == 'SELECTED') {
				echo('SELECTED');
				if ($type_selectie == 'SELECTED') $type_selectie = $data['Comp_Type'];
			}
			echo('>'. $data['Type_Naam'] .'</option>'); 
		}
		echo(Component_Types($data['Comp_Type'], $type_selectie));
	}

	//functie om alle type componenten uit de database te lezen en deze hierarchisch op te slaan 
	//voor gebruik in een <select></select> lijst
  function Component_Types($parent, &$selectie) {
  	$Uitkomst = '';
  	$query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	while ($huidige_level = mysql_fetch_array($resultaat)) {

  		$Uitkomst = $Uitkomst . '<option value="'. $huidige_level['Comp_Type'] .'"';
  		if(isset($selectie) && ($huidige_level['Comp_Type'] == $selectie || $selectie == 'SELECTED')) {
  			$Uitkomst = $Uitkomst . 'SELECTED';
  			$selectie = $huidige_level['Comp_Type'];
  		}
  		$Uitkomst = $Uitkomst . '>'. $huidige_level['Type_Naam'] .'</option>';

	  	$num_rows = mysql_num_rows(mysql_query($query));		
	  	if ($num_rows > 0) $Uitkomst = $Uitkomst . Component_Types($huidige_level['Comp_Type'], $selectie);
  	}  
  	return $Uitkomst;	
  }

	//Recursieve functie welke bekijkt (en teruggeeft) of het meegegeven type component in een structuur voorkomt
  //$aant is een variabele die als "call by reference" is meegegeven, dit moet gedaan worden omdat bij normaal returnen de waarde wordt overschreven
  function Check_Comp_Type($parent, $invoer, $aant) {
  	//alle component types selecteren, welke het meegegeven type als parent hebben
  	$query = 'SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent = '.$parent;
	  $resultaat = mysql_query($query);
  	//zolang er childs van het meegegeven type zijn, dan...
  	while ($huidige_level = mysql_fetch_array($resultaat)) {
			//als de child het gezochte record is, dan $aant 1 verhogen
			if($huidige_level['Comp_Type'] == $invoer) $aant = $aant + 1;

			//als het child niet het gevonden record is, dan een niveau dieper kijken
			else {
		  	//kijken of er childs zijn
		  	$num_rows = mysql_num_rows(mysql_query($query));
		  	//er zijn childs, dus een niveau dieper duiken
		  	if ($num_rows > 0) Check_Comp_Type($huidige_level['Comp_Type'], $invoer, &$aant);
		  }
  	}  
  }

  //Functie welke de groepen retouneert (in een array) welke access hebben tot het meegegeven componentType
  function Check_groepen($Gezocht_Component) {
	  $Groepen = array();

		//elk record in de gebruikersgroeprechten tabel langs om te evalueren
	  $query = 'SELECT * FROM gebruikersgroeprechten';
	  $rest = mysql_query($query);
	 	//per record bekijken of de groep toegang heeft tot het componenttype
	 	while ($data = mysql_fetch_array($rest)) {
			//het gezochte type is al gevonden, omdat deze in de gebruikersgroeprechten tabel staat.
			if($data['Comp_Type_ID'] == $Gezocht_Component) array_push($Groepen, $data['Groep_ID']);
			else {
				$aantal = 0;
				if ($data['onderliggende_Data'] == 1) {
					//De componenttypestructuur maken en kijken of het gezochte type in de structuur voorkomt
					Check_Comp_Type($data['Comp_Type_ID'], $Gezocht_Component, &$aantal);
					//gevonden!
					if ($aantal > 0) array_push($Groepen, $data['Groep_ID']);
				}
			}
		}
  	//de (gevonden) groepen retourneren
  	return $Groepen;
	}


	//functie welke een array teruggeeft met component types welke een gebruikersgroep zien mag
	Function Bepaal_Types(){
  	$uitkomst = array();

		function Comp_Type_Selectie($parent) {
			$Collectie = array();
			$query = "SELECT * FROM comp_type WHERE Type_Parent = '".$parent."'";
	  	$result = mysql_query($query);
		 	while ($data = mysql_fetch_array($result)) {
		  	array_push($Collectie, $data['Comp_Type']);
		  	$num_rows = mysql_num_rows(mysql_query($query));			
				if ($num_rows > 0) { 
					$Collectie = array_merge($Collectie, Comp_Type_Selectie($data['Comp_Type']));
				}
			}
			return $Collectie;
		}

  	//het ophalen van het begintype
  	$query = "SELECT Comp_Type_ID, onderliggende_Data FROM gebruikersgroeprechten WHERE Groep_ID = '". $_SESSION['groep_id'] ."'";
  	$result = mysql_query($query);
		$row = mysql_fetch_array($result);
		if ($row['onderliggende_Data'] == 1) {
			if ($row['Comp_Type_ID'] != 1)
				array_push($uitkomst,$row['Comp_Type_ID']);

			$query = "SELECT * FROM comp_type WHERE Type_Parent = '".$row['Comp_Type_ID']."'";
	  	$result = mysql_query($query);
		 	while ($data = mysql_fetch_array($result)) {
		 		array_push($uitkomst,$data['Comp_Type']);
		 		$uitkomst = array_merge($uitkomst, Comp_Type_Selectie($data['Comp_Type']));
		 	}
		}
		return $uitkomst;
	}


  //Functie welke de componenten weergeeft waar de ingelogde gebruiker toegang tot heeft
  //dit gebeurt aan de hand van een array met types
  Function Vul_Componenten_Select_Box($types_array, $selectie) {
		//het ophalen van alle componenten van de types die de gebruiker zien mag
		for ($i = 0; $i < Count($types_array); $i++)
		{
			$query = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID = '".$types_array[$i]."'";
	  	$result = mysql_query($query);
		 	while ($data = mysql_fetch_array($result)) {
				echo("<option value=\"". $data['Comp_Lijst_ID'] ."\"");
				if ($selectie == $data['Comp_Lijst_ID']) echo(" SELECTED");
				echo(">".  $data['Comp_Naam'] ."</option>\n");
			}
		}
  }

?>