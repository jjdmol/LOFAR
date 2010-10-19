<?php

	function Extra_Velden_Controle() {
		//bestaande extra velden
		$aantal_velden = $_POST['aantal'];
		
		for($i = 0; $i < $aantal_velden; $i++) {
			//verplichte velden ingevuld??
			if ($_POST['v' . $i] == '1') {
				//bestanden
				if ($_POST['t' . $i] == '5') {
					if ($_SESSION['bestand' . $i] == '-1') return false;
				}
				//overige
				else {
					if ($_POST[$i] == '') return false;
				}
			}
			//datum controle bij datum velden
			if ($_POST['t' . $i] == '4a') {
				if (Valideer_Datum($_POST[$i]) == false) return false;
			}
			//tijd controle bij datum velden
			if ($_POST['t' . $i] == '4b') {
				if (Valideer_Tijd($_POST[$i]) == false) return false;
			}
		}
		
		//aan te maken extra velden
		$aan_te_maken = $_POST['aantemaken'];
		
		for($i = 0; $i < $aan_te_maken; $i++) {
			//verplichte velden ingevuld??
			if ($_POST['av' . $i] == '1') {
				//bestanden
				if ($_POST['at' . $i] == '5') {				
					if ($_SESSION['abestand' . $i] == '-1') return false;
				}
				//overige
				else {
					if ($_POST['a' . $i] == '') return false;
				}
			}
			//datum controle bij datum velden
			if ($_POST['at' . $i] == '4a') {
				if (Valideer_Datum($_POST['a'.$i]) == false) return false;
			}
			//tijd controle bij datum velden
			if ($_POST['at' . $i] == '4b') {
				if (Valideer_Tijd($_POST['a'.$i]) == false) return false;
			}
		}
		
		return true;
	}
	
	function Melding_Velden_Controle() {
		//extra velden controle!!!
		$aantal_velden = $_POST['meld_aantal'];
		
		for($i = 0; $i < $aantal_velden; $i++) {

			//verplichte velden ingevuld??
			if ($_POST['m_v' . $i] == '1') {
				//bestanden
				if ($_POST['m_t' . $i] == '5') {
					if ($_SESSION['m_bestand' . $i] == '-1') return false;
				}
				//overige
				else {
					if ($_POST['m_' . $i] == '') return false;
				}
			}
			//datum controle bij datum velden
			if ($_POST['m_t' . $i] == '4a') {
				if (Valideer_Datum($_POST['m_' . $i]) == false) return false;
			}
			//tijd controle bij datum velden
			if ($_POST['m_t' . $i] == '4b') {
				if (Valideer_Tijd($_POST['m_' . $i]) == false) return false;
			}
		}
		return true;
	}

	
	function Child_Controle() {
		$query = "SELECT Count(Comp_Type_ID) FROM comp_lijst WHERE Comp_Parent_ID = '".$_GET['c']."' GROUP BY Comp_Type_ID";
		$resultaat = mysql_query($query);
		if ($resultaat != null) {
			$data = mysql_fetch_array($resultaat);
			//er hangen componenten onder, dus false retourneren om de bewerking te stoppen
			if (isset($data[0]))
				return false;
		}	
		else return true;
	}
	
	function Type_Controle() {
		//wanneer het parent component veranderd wordt...
		if (isset($_POST['comp_huidige_type']) && isset($_POST['comp_nieuwe_type']) && $_POST['comp_huidige_type'] != $_POST['comp_nieuwe_type']){
			//parent!! kijken of er componenten onder hangen. is dit zo, dan niet verplaatsen
			if (!Child_Controle()) return false;
		}
		return true;
	}

	function Parent_Controle() {
		//wanneer het parent component veranderd wordt...
		if (isset($_POST['comp_huidige_parent']) && isset($_POST['comp_nieuwe_parent']) && $_POST['comp_huidige_parent'] != $_POST['comp_nieuwe_parent']){
			//parent!! kijken of er componenten onder hangen. is dit zo, dan niet verplaatsen
			if (!Child_Controle()) return false;
		}					
		return true;
	}

	//validatie functie om te kijken of er opgeslagen mag worden.
	function Validatie_Opslaan(){
		if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
			return false;

		if (isset($_POST['comp_naam'])) {
			if ($_POST['comp_naam'] == '')
				return false;
		} else return false;

		//controleren of er wel een PARENT voor dit component geselecteerd is
		if (isset($_POST['hidden_parent']) && $_POST['hidden_parent'] =='') 
			return false;
		//er is een PARENT ingevoerd, dus kijken of het maximum aantal niet overschreden wordt...
		else {
			//kijken of deze PARENT een schaduwtype is
			$query = "SELECT Schaduw_Vlag FROM comp_lijst WHERE Comp_Lijst_ID = '".$_POST['hidden_parent']."'";
		  $resultaat = mysql_query($query);
	  	$data = mysql_fetch_array($resultaat);
	  	
	  	//Parent is een schaduwtype, dus maximumaantal geldt niet. 
	  	//wel controleren of dit niet zelf een parent is, in dit geval mag dit type maar 1 x voorkomen
	  	if ($data['Schaduw_Vlag'] == 1) {
	  		$query = "SELECT * FROM comp_type WHERE Type_Parent = '".$_POST['comp_type']."'";
				$num_rows = mysql_num_rows(mysql_query($query));
				if($num_rows > 0 && isset($_POST['hidden_schaduw']) && $_POST['hidden_schaduw'] == '0')
					return false;
	  	}
	  	//geen schaduw_vlag, dus controleren of het maximum aantal niet overschreden wordt
	  	else {
				//kijken wat het maximum aantal voor dit type is
				$query = "SELECT Max_Aantal FROM comp_type WHERE Comp_Type = '". $_POST['comp_type'] ."'";
			  $resultaat = mysql_query($query);
		  	$data = mysql_fetch_array($resultaat);
			  $maximum = $data['Max_Aantal'];
				
				//kijken welke parent dit is en hoeveel comps hier al van zijn aangemaakt +1
				$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Parent = '".$_POST['hidden_parent']."' AND Comp_Type_ID = '".$_POST['comp_type']."'";
			  $resultaat = mysql_query($query);
		  	$data = mysql_fetch_array($resultaat);
			  $aantal = $data[0];
				
				//alleen bij wijzigen van een parent, 
				//moet er gecontroleerd worden of het maximum aantal overschreden wordt
				$query = "SELECT Comp_Parent FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
			  $resultaat = mysql_query($query);
		  	$data = mysql_fetch_array($resultaat);
				if ($data['Comp_Parent'] != $_POST['hidden_parent'])  {
				
					//met elkaar vergelijken			  
				  if(($aantal + 1) > $maximum) 
				  	return false;
		  	}
			}
		}

		if (!Type_Controle()) return false;
		if (!Parent_Controle()) return false;

		//de statusdatum controleren
		if (isset($_POST['statusdatum'])) {
			//wanneer de statusdatum gevuld is, dan...
			if($_POST['statusdatum'] !='') {
				
				//controleren op de juiste samenstelling van de statusdatum
				if (Valideer_Datum($_POST['statusdatum']) == false)
				return false;
			
				//controleren of de tijd correct ingevoerd is
				if(isset($_POST['statustijd'])) {
				  if (Valideer_Tijd($_POST['statustijd']) == false)
				  	return false;
				}
			}
		} 
		
		//de leverdatum controleren
		if (isset($_POST['leverdatum'])) {
			//wanneer de leverdatum gevuld is, dan...
			if($_POST['leverdatum'] !='') {
				
				//controleren op de juiste samenstelling van de leverdatum
				if (Valideer_Datum($_POST['leverdatum']) == false)
				return false;
			
				//controleren of de tijd correct ingevoerd is
				if(isset($_POST['levertijd'])) {
				  if (Valideer_Tijd($_POST['levertijd']) == false)
				  	return false;
				}
			}
		} 

		//de fabricagedatum controleren
		if (isset($_POST['fabricagedatum'])) {
			//wanneer de fabricagedatum ingevuld is, dan... 
			if($_POST['fabricagedatum'] !='') {

				//controleren op de juiste samenstelling van de fabricagedatum
				if (Valideer_Datum($_POST['fabricagedatum']) == false)
					return false;
				//controleren of de tijd correct ingevoerd is
				if(isset($_POST['fabricagetijd'])) {
					if (Valideer_Tijd($_POST['fabricagetijd']) == false)
				 		return false;
				}
			}
		}
		
		if (Extra_Velden_Controle() == false)   return false;
		if (Melding_Velden_Controle() == false) return false;

		return true;
	}

	//eerst een validatie doen om de ingevoerde gegevens te controleren en te kijken of er opgeslagen mag worden...
	if(Validatie_Opslaan()) {

		//Een nieuwe melding van het component toevoegen, dit is zeer waarschijnlijk een bewerkenmelding oid
		$query_melding = "INSERT INTO melding_lijst (Meld_Type_ID, Comp_Lijst_ID, Comp_Parent, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Behandeld_Door, Gemeld_Door, Melding_Locatie)";
		$query_melding = $query_melding . "VALUES ('". $_POST['type_melding'] ."', '".$_GET['c']."', '". $_POST['hidden_parent'] ."'";

		//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
		if (isset($_POST['statusdatum']) && $_POST['statusdatum'] != '')
			$query_melding = $query_melding . ", '". Datum_Tijd_Naar_DB_Conversie($_POST['statusdatum'],$_POST['statustijd']) ."'";
		else $query_melding = $query_melding . ", NOW()";
		$query_melding = $query_melding . ", '". $_POST['hidden_status'] ."', '". $_POST['Voorgaande_Melding'] ."', '". htmlentities($_POST['hidden_melding'], ENT_QUOTES);
		$query_melding = $query_melding . "', '". $_SESSION['gebr_id'] . "', '". $_SESSION['gebr_id'] ."', '" . $_POST['comp_locatie'] ."') ";

		mysql_query($query_melding);
		$melding_id = mysql_insert_id();

		//opslaan van het component
		$query = "UPDATE comp_lijst SET Comp_Naam = '". htmlentities($_POST['comp_naam'], ENT_QUOTES) . "', Comp_Parent = '". $_POST['hidden_parent'] . "', Laatste_Melding ='". $melding_id;
		$query = $query . "', Comp_Locatie = '". $_POST['comp_locatie'] ."', Comp_Verantwoordelijke = '". $_POST['hidden_verantwoordelijke'] . "'";
		
		//de waarde voor de leverdatum aan de query toevoegen
		if (isset($_POST['leverdatum']) && $_POST['leverdatum'] != '')
			$query = $query . ", Lever_Datum = '". Datum_Tijd_Naar_DB_Conversie($_POST['leverdatum'],$_POST['levertijd']) ."'";

		//de waarde voor de fabricagedatum aan de query toevoegen
		if (isset($_POST['fabricagedatum']) && $_POST['fabricagedatum'] != '')
			$query = $query . ", Fabricatie_Datum = '". Datum_Tijd_Naar_DB_Conversie($_POST['fabricagedatum'],$_POST['fabricagetijd']) ."'";
	
		$query = $query . ", Contact_Leverancier='".$_POST['leverancier']."', Contact_Fabricant='".$_POST['fabricant']."' WHERE Comp_Lijst_ID = '" . $_GET['c'] . "'";

		$errorlevel = 0;
		if (mysql_query($query)) {
			$errorlevel = 1;
			
			//aantal velden controle.. meer dan 0 dan dit doen, anders overslaan
			if (isset($_POST['aantal']) && $_POST['aantal'] > 0) {
				//voor elk veld het record updaten
				for ($i = 0; $i < $_POST['aantal']; $i++){
    			
    			$query  = "SELECT Data_Kolom_ID FROM extra_velden WHERE Kolom_ID ='".$_POST['i'. $i]."'";
					$result = mysql_query($query);
					$veld   = mysql_fetch_array($result);
    			
    			$waarde = $_POST[$i];
    			//1 = integer, 2 = double, 3 = text, 4 = datumtijd, 5 = bestandsverwijzing
					if ($_POST['t'.$i] == 1) {
						$query = "UPDATE datatabel SET Type_Integer ='" . $waarde . "', Type_TinyText = NULL, Type_Double = NULL, Type_Text = NULL, Type_DateTime = NULL WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					}
					else if ($_POST['t'.$i] == 2) {
						$query = "UPDATE datatabel SET Type_Double ='" . $waarde . "', Type_TinyText = NULL, Type_Integer = NULL, Type_Text = NULL, Type_DateTime = NULL WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					}
					else if ($_POST['t'.$i] == 3) {
						$query = "UPDATE datatabel SET Type_Text ='" . $waarde . "', Type_TinyText = NULL, Type_Integer = NULL, Type_Double = NULL, Type_DateTime = NULL WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					}
					else if ($_POST['t'.$i] == '4b') {
						$waarde = Datum_Tijd_Naar_DB_Conversie($_POST[$i -1],$_POST[$i]);
						$query = "UPDATE datatabel SET Type_DateTime ='" . $waarde . "', Type_TinyText = NULL, Type_Integer = NULL, Type_Double = NULL, Type_Text = NULL WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					}
					else if ($_POST['t'.$i] == 5) {
						//geen nieuwe waarde, want bewerken van een opgeslagen is niet mogelijk
						//dus de opgeslagen waarde ophalen en deze opslaan
						$query = "SELECT Type_TinyText FROM datatabel WHERE Data_Kolom_ID = '".$veld['Data_Kolom_ID']."'";
						$rest = mysql_query($query);
						$uitkomst = mysql_fetch_array($rest);
						$waarde = $uitkomst[0];

						$query = "UPDATE datatabel SET Type_TinyText ='" . $waarde . "', Type_DateTime = NULL, Type_Integer = NULL, Type_Double = NULL, Type_Text = NULL WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					}
 					else $query = '';

					//$query = "UPDATE datatabel SET " . $type . " ='" . $waarde . "' WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
					
					if (($_POST['t' . $i] != '4a') && mysql_query($query)) {
						$errorlevel = 2;
					}
				}
			}
			
			$error_extra = 0;
			//de extra velden, welke nog niet aangemaakt zijn, in de database toevoegen
			if (isset($_POST['aantemaken']) && $_POST['aantemaken'] > 0) {

				for ($i = 0; $i < $_POST['aantemaken']; $i++){
 				
	 				//DataTabel entry maken
	  			//integer
	  			if ($_POST['at' . $i] == '1') 		   $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST['a'.$i]."')";
	  			//double
	  			else if ($_POST['at' . $i] == '2')  $query = "INSERT INTO datatabel (Type_Double) VALUES('".$_POST['a'.$i]."')"; 
	  			//text
	  			else if ($_POST['at' . $i] == '3')  $query = "INSERT INTO datatabel (Type_Text) VALUES('".$_POST['a'.$i]."')"; 
	  			//datumtijd
	  			else if ($_POST['at' . $i] == '4b') $query = "INSERT INTO datatabel (Type_DateTime) VALUES('". Datum_Tijd_Naar_DB_Conversie($_POST['a'.($i -1)],$_POST['a'.$i]) ."')"; 
	  			//bestandsverwijzing
	  			else if ($_POST['at' . $i] == '5')  $query = "INSERT INTO datatabel (Type_TinyText) VALUES('".$_SESSION['abestand'.$i]."')"; 
	  			else $query = "";
	
					//uitvoeren van de datatabel record
					if (($_POST['at' . $i] != '4a') && mysql_query($query)) {
		 				$error_extra = 1;
	
    				$Veld_ID = mysql_insert_id();
    				
    				if ($_POST['at'.$i] == '4b') $datatype = 4;
    				else $datatype = $_POST['at'.$i];
    				
    				$query = "INSERT INTO extra_velden (Data_Kolom_ID, Veld_Naam, Aangemaakt_Door, DataType, Type_Beschrijving, Tabel_Type, Is_Verplicht)";
    				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['an'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['ai'.$i] ."', '3', '".$_POST['av'.$i]."')";

	    			if (mysql_query($query)) {
			 				$error_extra = 2;

	    				$Veld_ID = mysql_insert_id();	  				
	  					$query = "INSERT INTO comp_koppel_extra (Kolom_ID, Comp_Lijst_ID) VALUES('".$Veld_ID. "', '". $_GET['c'] ."')";
		    			
		    			if (mysql_query($query)) {
				 				$error_extra = 3;
  		  			}
	    			}
					}
				}
			}
			
			//extra velden van de melding toevoegen
			$aantal_velden = $_POST['meld_aantal'];
			
			$error_extra = 0;
			for($i = 0; $i < $_POST['meld_aantal']; $i++) {
 				//DataTabel entry maken
  			//integer
  			if ($_POST['m_t' . $i] == '1') 		   $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST['m_' . $i]."')";
  			//double
  			else if ($_POST['m_t' . $i] == '2')  $query = "INSERT INTO datatabel (Type_Double) VALUES('".$_POST['m_' . $i]."')"; 
  			//text
  			else if ($_POST['m_t' . $i] == '3')  $query = "INSERT INTO datatabel (Type_Text) VALUES('".$_POST['m_' . $i]."')"; 
  			//datumtijd
  			else if ($_POST['m_t' . $i] == '4b') $query = "INSERT INTO datatabel (Type_DateTime) VALUES('". Datum_Tijd_Naar_DB_Conversie($_POST['m_' . $i -1],$_POST['m_' . $i]) ."')"; 
  			//bestandsverwijzing
  			else if ($_POST['m_t' . $i] == '5')  $query = "INSERT INTO datatabel (Type_TinyText) VALUES('".$_SESSION['m_bestand' . $i]."')"; 
  			else $query = "";

				//uitvoeren van de datatabel record
				if (($_POST['m_t' . $i] != '4a') && mysql_query($query)) {
	 				$error_extra = 1;

  				$Veld_ID = mysql_insert_id();
  				
  				if ($_POST['m_t'.$i] == '4b') $datatype = 4;
  				else $datatype = $_POST['m_t'.$i];
  				
  				$query = "INSERT INTO extra_velden (Data_Kolom_ID, Veld_Naam, Aangemaakt_Door, DataType, Type_Beschrijving, Tabel_Type, Is_Verplicht)";
  				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['m_n'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['m_i'.$i] ."', '3', '".$_POST['m_v'.$i]."')";

    			if (mysql_query($query)) {
		 				$error_extra = 2;

    				$Veld_ID = mysql_insert_id();	  				
  					$query = "INSERT INTO melding_koppel_extra (Kolom_ID, Meld_Lijst_ID) VALUES('".$Veld_ID. "', '". $melding_id ."')";

	    			if (mysql_query($query)) {
			 				$error_extra = 3;
		  			}
    			}
				}
			}
		}
		
		if ($errorlevel == 2) echo("Het gewijzigde component \"". $_POST['comp_naam'] ."\" is (inclusief extra velden)in het systeem bijgewerkt<br>");
		else if ($errorlevel == 0) echo("Er is iets mis gegaan met het opslaan van het component \"". $_POST['comp_naam'] ."\"!! Het component is niet bijgewerkt!");
		else if ($errorlevel == 1) echo("Het gewijzigde component \"". $_POST['comp_naam'] ."\" is in het systeem bijgewerkt<br>");
		echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige component of selecteer links een component uit de treeview.</a>');
							
	}
	else {
		//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
		date_default_timezone_set ("Europe/Amsterdam");

		if (isset($_GET['c']) && $_GET['c'] != 0 ) {
			$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID ='". $_GET['c'] ."'";
			$resultaat = mysql_query($query);
	  	$row = mysql_fetch_array($resultaat);
	?>
  	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
			<table>
				<tr>
					<td>Component ID:</td>
					<td> <?php echo($row['Comp_Lijst_ID']); ?></td>
				</tr>
				<tr>
					<td>Naam component:</td>
					<td><input name="comp_naam" type="text" value="<?php if (isset($_POST['comp_naam'])) echo(htmlentities($_POST['comp_naam'], ENT_QUOTES)); else echo($row['Comp_Naam']); ?>">
	    		<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['comp_naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>');?></td>
				</tr>
				<tr>
					<td>Type component:</td>
					<td>
						<?php 
							$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type = " . $row['Comp_Type_ID'];
							$result = mysql_query($query);
							$data = mysql_fetch_array($result);
							echo($data[0] .  "<input type=\"hidden\" name=\"comp_type\" id=\"comp_type\" value=\"".$row['Comp_Type_ID']."\">");
					 ?></td>
				</tr>
				<tr>
					<td>Parent component:</td>
					<?php
						$selected = $row['Comp_Lijst_ID']; //is het geselecteerde component welke bewerkt gaat worden
						$selectie = "&n=" .$row['Comp_Parent']; //parent component
					?>
					
					<td><iframe id="frame_parent" name="frame_parent" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_bewerken_parent.php?c=<?php echo($selected . $selectie); ?>" width="550" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
				</tr>

  			<tr>
  				<td>Reden bewerking:</td>
  				<td><select name="type_melding" onchange="document.theForm.submit();">
  					<?php
  						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
	    			  $resultaat = mysql_query($query);

				  		if (isset($_POST['type_melding'])) $meld_selectie = $_POST['type_melding'];
				  		else $meld_selectie = 'SELECTED';

					  	while ($data = mysql_fetch_array($resultaat)) {
	  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
		  	  			if ($data['Meld_Type_ID'] == $meld_selectie || $meld_selectie == 'SELECTED') {
		  	  				echo('SELECTED');
		  	  				$meld_selectie = $data['Meld_Type_ID'];
		  	  			}
		  	  			echo('>'. $data['Melding_Type_Naam'] .'</option>');
							}

  						$query = "SELECT Meld_Datum FROM melding_lijst WHERE Meld_Lijst_ID = '".$row['Laatste_Melding']."'";
	    			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);

    					$gedeeldveld=split(" ",$data['Meld_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
							$tijd = split(":",$gedeeldveld[1]);
							
  					?>
  					</select> Aangemaakt:
						<input name="statusdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['statusdatum'])) echo($_POST['statusdatum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
						<input name="statustijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['statustijd'])) echo($_POST['statustijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
					  <?php if(isset($_POST['statusdatum']) && (!Valideer_Datum($_POST['statusdatum']) || !Valideer_Tijd($_POST['statustijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?></td>
  				</td>
  			</tr>
				<tr>
					<td>Beschrijving:</td>
					<td><iframe id="frame_melding" name="frame_melding" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_melding.php?c=<?php echo($meld_selectie); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="580" height="72" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
					</td>
				</tr>
				<tr>
					<td>Locatie component:</td>
					<td><select name="comp_locatie">
						<?php
							$query2 = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
							$result = mysql_query($query2);
							while ($data = mysql_fetch_array($result)) {
								echo("<option value=\"". $data['Locatie_ID'] ."\"");
								if ($data['Locatie_ID'] == $row['Comp_Locatie']) echo(" SELECTED");
								echo(">". $data['Loc_Naam'] ."</option>\r\n");
							}
						?>
					</select> Verantwoordelijke: 
						<?php
							if (isset($_POST['hidden_verantwoordelijke'])) 
								$verantwoordelijke = $_POST['hidden_verantwoordelijke'];
							else $verantwoordelijke = $row['Comp_Verantwoordelijke'];
						?>
	 					<iframe id="frame_contact" name="frame_contact" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/type_verantwoordelijke.php?c=<?php echo($row['Comp_Type_ID'] . "&s=" . $verantwoordelijke);?>" width="300" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
					</td>
				</tr>
				<tr>
					<td>Fabricant:</td>
					<td><select name="fabricant">				    				
						<?php
		    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
	    			  $resultaat = mysql_query($query);
  						if (isset($_POST['fabricant'])) $selectie = $_POST['fabricant'];
  						else $selectie = $row['Contact_Fabricant'];
							
					  	while ($data = mysql_fetch_array($resultaat)) {
					  		echo('<option value="'. $data['Contact_ID'] .'"');
					  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
					  			echo('SELECTED');
					  		echo('>'. $data['Contact_Naam'] .'</option>');
					  	}
				    ?></select> Fabr. datum: 						
				    <?php 
							//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
    					$gedeeldveld=split(" ",$row['Fabricatie_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
							$tijd = split(":",$gedeeldveld[1]);
						 ?>
						<input name="fabricagedatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['fabricagedatum'])) echo($_POST['fabricagedatum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
						<input name="fabricagetijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['fabricagetijd'])) echo($_POST['fabricagetijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
					  <?php if(isset($_POST['fabricagedatum']) && (!Valideer_Datum($_POST['fabricagedatum']) || !Valideer_Tijd($_POST['fabricagetijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?></td>

					</td>
				</tr>
				<tr>
					<td>Leverancier:</td>
					<td><select name="leverancier">									
						<?php
		    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
	    			  $resultaat = mysql_query($query);
  						if (isset($_POST['leverancier'])) $selectie = $_POST['leverancier'];
  						else $selectie = $row['Contact_Leverancier'];
							
					  	while ($data = mysql_fetch_array($resultaat)) {
					  		echo('<option value="'. $data['Contact_ID'] .'"');
					  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
					  			echo('SELECTED');
					  		echo('>'. $data['Contact_Naam'] .'</option>');
					  	}
				    ?></select> Leverdatum:
						<?php 
							//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
    					$gedeeldveld=split(" ",$row['Lever_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
							$tijd = split(":",$gedeeldveld[1]);
						 ?>
						<input name="leverdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['leverdatum'])) echo($_POST['leverdatum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
						<input name="levertijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['levertijd'])) echo($_POST['levertijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
					  <?php if(isset($_POST['leverdatum']) && (!Valideer_Datum($_POST['leverdatum']) || !Valideer_Tijd($_POST['levertijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?>
					</td>
				</tr>
				<tr>
  				<td>Extra velden:<br>(* = verplicht)</td>
  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/component_bewerken_extra_velden.php?c=<?php echo($_GET['c']); ?>" width="400" height="110" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
  					<?php
  					  if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && !Extra_Velden_Controle()) echo("<b>* Foutieve waardes!</b>");
  					?>
  				</td>
				</tr>
    		<tr>
    			<td>
  					<?php
		 					//eerst bepalen hoeveel velden er zijn, ivm met het datumveld dat uit 2 input boxen bestaat
							$query = "SELECT Kolom_ID FROM comp_koppel_extra WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
							$resultaat = mysql_query($query);
							$aangemaakte_velden = array();
							$aantal_velden = 0;
							$bestands_velden = 0;
							while ($data = mysql_fetch_row($resultaat)) {
								$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data[0]  ."'";
								$result = mysql_query($query);
								$velden = mysql_fetch_array($result);
								//4 = datumtijd
								if ($velden['DataType'] == 4)
									$aantal_velden++;
								//4 = bestanden
								if ($velden['DataType'] == 5)
									$bestands_velden++;

								$aantal_velden++;								
			 	  			array_push($aangemaakte_velden, $velden['Type_Beschrijving']);
							}
 							//het aantal onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"aantal\" name=\"aantal\" type=\"hidden\" value=\"".$aantal_velden."\">\n");
 							echo("<input id=\"bestands_velden\" name=\"bestands_velden\" type=\"hidden\" value=\"".$bestands_velden."\">\n");
							
  						//4 hidden velden aanmaken voor elk extra veld: 1 voor de waarde, 1 voor het type en 1 voor de verplichtheid
							for($i = 0; $i < $aantal_velden; $i++){
	  						echo("<input id=\"".$i."\" name=\"".$i."\" type=\"hidden\" value=\"\">");
  							echo("<input id=\"t".$i."\" name=\"t".$i."\" type=\"hidden\" value=\"\">");
  							echo("<input id=\"v".$i."\" name=\"v".$i."\" type=\"hidden\" value=\"\">");
  							echo("<input id=\"i".$i."\" name=\"i".$i."\" type=\"hidden\" value=\"\">");
  							echo("<input id=\"n".$i."\" name=\"n".$i."\" type=\"hidden\" value=\"\">\n");
							}
							
							//de hidden velden voor de nog niet aangemaakte extra velden aanmaken
							$aan_te_maken = 0;
							$bestand_aanmaken = 0;
							//nog niet aangemaakte extra velden (die later toegevoegd zijn) toevoegen
							$query = "SELECT Kolom_ID FROM type_comp_koppel_extra WHERE Comp_Type_ID in (SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."')";
							$rest = mysql_query($query);
							while ($data = mysql_fetch_array($rest)) {
								$gevonden = false;
								//bekijken of dit record al is aangemaakt, dus array met aangemaakte waarden door itereren
								//$aan_te_maken verhogen
						  	for ($j = 0; $j < count($aangemaakte_velden);$j++) {
			   					if ($data['Kolom_ID'] == $aangemaakte_velden[$j]) $gevonden = true;
								}
								
								if($gevonden == false) {
									$query = "SELECT DataType FROM extra_velden WHERE Kolom_ID = '". $data['Kolom_ID'] ."'";
									$resultt = mysql_query($query);
									$uitkomst = mysql_fetch_array($resultt);

									if ($uitkomst['DataType'] == 4)
										$aan_te_maken++;
									if ($uitkomst['DataType'] == 5)
										$bestand_aanmaken++;

									$aan_te_maken++;									
								}
							}
 							//het aantal aan te maken velden onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"aantemaken\" name=\"aantemaken\" type=\"hidden\" value=\"".$aan_te_maken."\">\n");
 							echo("<input id=\"bestand_aanmaken\" name=\"bestand_aanmaken\" type=\"hidden\" value=\"".$bestand_aanmaken."\">\n");
							
							//niet aangemaakte extra velden, dus hidden fields hiervoor aan maken					
							for($i = 0; $i < $aan_te_maken; $i++){
			
								echo("<input type=\"hidden\" name=\"a" . $i ."\" id=\"a" . $i ."\" value=\"\">");
								echo("<input type=\"hidden\" name=\"at". $i ."\" id=\"at". $i ."\" value=\"\">");
								echo("<input type=\"hidden\" name=\"av". $i ."\" id=\"av". $i ."\" value=\"\">");
								echo("<input type=\"hidden\" name=\"ai". $i ."\" id=\"ai". $i ."\" value=\"\">");
								echo("<input type=\"hidden\" name=\"an". $i ."\" id=\"an". $i ."\" value=\"\">\n");
							}
							
							//extra velden (welke bij de melding horen) bepalen
		 					//eerst bepalen hoeveel velden er zijn, ivm met het datumveld dat uit 2 input boxen bestaat
							$query = "SELECT Kolom_ID FROM type_melding_koppel_extra WHERE Meld_Type_ID = '". $meld_selectie ."'";
							$resultaat = mysql_query($query);
							$meld_aantal = 0;
							$meld_bestand = 0;
							while ($data = mysql_fetch_row($resultaat)) {
								$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data[0]  ."'";
								$result = mysql_query($query);
								$meld_velden = mysql_fetch_array($result);
								//4 = datumtijd
								if ($meld_velden['DataType'] == 4)
									$meld_aantal++;
								if ($meld_velden['DataType'] == 5)
									$meld_bestand++;
								
								$meld_aantal++;
							}
 							//het aantal onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"meld_aantal\" name=\"meld_aantal\" type=\"hidden\" value=\"".$meld_aantal."\">\n");
 							//het aantal bestandsvelden onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"meld_bestand\" name=\"meld_bestand\" type=\"hidden\" value=\"".$meld_bestand."\">\n");
							
  						//5 hidden velden aanmaken voor elk extra veld: 1 voor de waarde, 1 voor het type en 1 voor de verplichtheid en 1 voor de ID van de parent record en 1 voor de veldnaam
							for($i = 0; $i < $meld_aantal; $i++){
	  						echo("<input id=\"m_".$i."\" name=\"m_".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST["m_".$i])) echo($_POST["m_".$i]);
	  						echo("\">");
  							echo("<input id=\"m_t".$i."\" name=\"m_t".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['m_t'.$i])) echo($_POST['m_t'.$i]);
	  						echo("\">");
  							echo("<input id=\"m_v".$i."\" name=\"m_v".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['m_v'.$i])) echo($_POST['m_v'.$i]);
	  						echo("\">");
  							echo("<input id=\"m_i".$i."\" name=\"m_i".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['m_i'.$i])) echo($_POST['m_i'.$i]);
	  						echo("\">");
  							echo("<input id=\"m_n".$i."\" name=\"m_n".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['m_n'.$i])) echo($_POST['m_n'.$i]);
	  						echo("\">\n");
							}
							
					 ?>
  					<input id="hidden_parent" name="hidden_parent" type="hidden" value="">
  					<input id="hidden_melding" name="hidden_melding" type="hidden" value="">
  					<input id="hidden_status" name="hidden_status" type="hidden" value="">
    				<input id="opslaan" name="opslaan" type="hidden" value="1">
    				<input id="hidden_verantwoordelijke" name="hidden_verantwoordelijke" type="hidden" value="-1">
    				<input id="Voorgaande_Melding" name="Voorgaande_Melding" type="hidden" value="<?php echo($row['Laatste_Melding']); ?>">
    			</td><td><a href="javascript:SubmitComponentBewerken();">Opslaan</a></td>
    		</tr>
			</table>
		</form>
	
	<?php
		}
		else echo('Er is geen component geselecteerd om te wijzigen.<br>Selecteer hiernaast een component.'); 
	}
?>