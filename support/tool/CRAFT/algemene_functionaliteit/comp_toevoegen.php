<?php
		  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');

			function Extra_Velden_Controle() {
				//extra velden controle!!!
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



			//het valideren van de invoer, dus controleren of de ingevoerde gegevens opgeslagen mogen worden
			function Valideer_Invoer() {
				if (!isset($_POST['opslaan']))
					return false; 
					
				if (isset($_POST['opslaan']) && $_POST['opslaan'] != 1)
					return false;


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
						
						//met elkaar vergelijken			  
	  			  if(($aantal + 1) > $maximum) 
	  			  	return false;
	  			 }
				}
					
				//controleren of er wel een naam voor dit component ingevoerd is
				if (isset($_POST['hidden_naam'])) {
					if ($_POST['hidden_naam'] == '')
						return false;
				} else return false;

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

				//alle controles zijn goed doorlopen
				return true;
			}
			
			
			//controleren of er opgeslagen kan worden, of dat de invoervelden getoond moeten worden
			if (Valideer_Invoer()) {

				//het eerste gedeelte van de query
				$query = "INSERT INTO comp_lijst (Comp_Naam, Comp_Type_ID, Comp_Parent, Comp_Locatie, Comp_Verantwoordelijke, Contact_Fabricant, Contact_Leverancier";
				//als er een leverdatum ingevoerd is, dan dit veld ook toevoegen aan de query
				if (isset($_POST['hidden_leverdatum']) && $_POST['hidden_leverdatum'] != '')
					$query = $query . ", Lever_datum";
				//als er een fabricagedatum ingevoerd is, dan dit veld ook toevoegen aan de query
				if (isset($_POST['hidden_fabricagedatum']) && $_POST['hidden_fabricagedatum'] != '')
					$query = $query . ", Fabricatie_Datum";
				
				//als dit het admingedeelte is, dan is het schaduw_vlag checkbox aanwezig
				if(isset($_SESSION['admin_deel']) && 	$_SESSION['admin_deel'] > 0)
					$query = $query . ", Schaduw_Vlag";
				
				//de waardes, welke opgeslagen moeten worden in de database
				$query = $query . ") VALUES ('". htmlentities($_POST['hidden_naam'], ENT_QUOTES) ."', '". $_POST['comp_type'] ."', '". $_POST['hidden_parent'] ."', '";
				$query = $query .  $_POST['comp_locatie'] ."', '".  $_POST['hidden_verantwoordelijke']."', '".$_POST['hidden_fabricant']."', '".$_POST['hidden_leverancier']."'";
				    				
				//de waarde voor de leverdatum aan de query toevoegen
				if (isset($_POST['hidden_leverdatum']) && $_POST['hidden_leverdatum'] != '') {
  				$datum=split("-",$_POST['hidden_leverdatum']);
  				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['hidden_levertijd'] .":00'";
  			}
				//de waarde voor de fabricagedatum aan de query toevoegen
				if (isset($_POST['hidden_fabricagedatum']) && $_POST['hidden_fabricagedatum'] != '') {
  				$query = $query . ", '". Datum_Tijd_Naar_DB_Conversie($_POST['hidden_fabricagedatum'], $_POST['hidden_fabricagetijd']) . "'";
				}
				
				//waarde voor schaduw_vlag aan de query toevoegen
				if(isset($_SESSION['admin_deel']) && 	$_SESSION['admin_deel'] > 0) {
					if (isset($_POST['hidden_schaduw']) && ($_POST['hidden_schaduw'] == '1' || $_POST['hidden_schaduw'] == 'on')) 
						$query = $query . ", '1'";
					else $query = $query . ",'0'";
				}				
				
				//de query afsluiten met een haakje
				$query = $query . ')';
				
				//variabele om bij te houden hoeveel querys goed uitgevoerd zijn
				//dit is bedoeld voor het geven van de juiste foutmelding
				$errorLevel = 0;

				//wanneer het toevoegen van het component goed gegaan is, dan een melding toevoegen
				if (mysql_query($query)) {
					$errorLevel = 1;

					//de toegevoegde component_ID opslaan voor later gebruik
					$Comp_ID = mysql_insert_id();

					//De eerste melding van het component toevoegen, dit is zeer waarschijnlijk het plaatsen van het component oid
					$query = "INSERT INTO melding_lijst (Melding_Locatie, Meld_Type_ID, Comp_Lijst_ID, Comp_Parent, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Behandeld_Door, Gemeld_Door)";
					$query = $query . "VALUES ('". $_POST['comp_locatie'] ."', '". $_POST['type_melding'] ."', '". $Comp_ID ."', '". $_POST['hidden_parent'] ."'";

  				//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
  				if (isset($_POST['statusdatum']) && $_POST['statusdatum'] != '') {
    				$datum=split("-",$_POST['statusdatum']);
	  				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['statustijd'] .":00'";							
					}
					else $query = $query . ", NOW()";
					$query = $query . ", '". $_POST['hidden_status'] ."', '1', '". htmlentities($_POST['hidden_melding'], ENT_QUOTES) ."', '". $_SESSION['gebr_id'] ."', '". $_SESSION['gebr_id'] ."') ";
										
					//de melding is goed toegevoegd, dus nu een verwijzing naar de laatste melding bij het component voegen
					if (mysql_query($query)) {
						$errorLevel = 2;
						//de ID van de toegevoegde melding ophalen
						$Meld_ID = mysql_insert_id();
						
						$query = "UPDATE comp_lijst SET Laatste_Melding = '". $Meld_ID ."' WHERE Comp_Lijst_ID='". $Comp_ID ."'";
						if (mysql_query($query)) {
							$errorLevel = 3;
							
			 				//extra velden toevoegen
			 				$aantal_velden = $_POST['aantal'];
			 				
			 				$error_extra = 0;
			 				for($i = 0; $i < $_POST['aantal']; $i++) {
				 				//DataTabel entry maken
			    			//integer
			    			if ($_POST['t' . $i] == '1') 		   $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST[$i]."')";
			    			//double
			    			else if ($_POST['t' . $i] == '2')  $query = "INSERT INTO datatabel (Type_Double) VALUES('".$_POST[$i]."')"; 
			    			//text
			    			else if ($_POST['t' . $i] == '3')  $query = "INSERT INTO datatabel (Type_Text) VALUES('".$_POST[$i]."')"; 
			    			//datumtijd
			    			else if ($_POST['t' . $i] == '4b') $query = "INSERT INTO datatabel (Type_DateTime) VALUES('". Datum_Tijd_Naar_DB_Conversie($_POST[$i -1],$_POST[$i]) ."')"; 
			    			//bestandsverwijzing
			    			else if ($_POST['t' . $i] == '5')  $query = "INSERT INTO datatabel (Type_TinyText) VALUES('".$_SESSION['bestand' . $i]."')"; 
			    			else $query = "";

								//uitvoeren van de datatabel record
								if (($_POST['t' . $i] != '4a') && mysql_query($query)) {
					 				$error_extra = 1;

			    				$Veld_ID = mysql_insert_id();
			    				
			    				if ($_POST['t'.$i] == '4b') $datatype = 4;
			    				else $datatype = $_POST['t'.$i];
			    				
			    				$query = "INSERT INTO extra_velden (Data_Kolom_ID, Veld_Naam, Aangemaakt_Door, DataType, Type_Beschrijving, Tabel_Type, Is_Verplicht)";
			    				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['n'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['i'.$i] ."', '3', '".$_POST['v'.$i]."')";

				    			if (mysql_query($query)) {
						 				$error_extra = 2;

				    				$Veld_ID = mysql_insert_id();	  				
		  	  					$query = "INSERT INTO comp_koppel_extra (Kolom_ID, Comp_Lijst_ID) VALUES('".$Veld_ID. "', '". $Comp_ID ."')";

					    			if (mysql_query($query)) {
							 				$error_extra = 3;
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
		  	  					$query = "INSERT INTO melding_koppel_extra (Kolom_ID, Meld_Lijst_ID) VALUES('".$Veld_ID. "', '". $Meld_ID ."')";

					    			if (mysql_query($query)) {
							 				$error_extra = 3;
			  		  			}
				    			}
								}
							}
						}
					}
				}

				//de foutmeldingen
				if ($errorLevel == 3) {
					if($error_extra == 3) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is (inclusief extra velden) aan het systeem toegevoegd!<br>");
					else if ($error_extra == 1) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd!<br>Alleen het toevoegen van extra velden (in de extra_velden tabel) is mislukt!<br>");
					else if ($error_extra == 2) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd!<br>Alleen het leggen van de koppeling tussen de extra velden en het component is mislukt!<br>");
					else if ($error_extra == 0) {
						if($_POST['aantal'] == 0)
							echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is succesvol aan het systeem toegevoegd!<br>");
						else echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd!<br>Alleen het toevoegen van extra velden (in de datatabel) is mislukt!<br>");
					}
				}
				else if ($errorLevel == 0) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
				else if ($errorLevel == 1) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd.<br>Alleen er is iets foutgegaan met het toevoegen van de melding! De melding is dus niet aan het systeem toegevoegd!");
				else if ($errorLevel == 2) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" en bijbehorende melding is aan het systeem toegevoegd.<br>Alleen er is iets foutgegaan met het verwijzen van de melding naar het component!");
				echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een component toe te voegen.</a>');
			}
			//er mag niet opgeslagen worden, dus toon het formulier met invoervelden
			else {
				//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
				date_default_timezone_set ("Europe/Amsterdam");

		?>
	
		<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>">
  		<table>
  			<tr>
  				<td>Type om toe te voegen:</td>
  				<td><select name="comp_type" id="comp_type" onchange="PostDocument('<?php echo($_SESSION['huidige_pagina']); ?>');">
		    	  <?php
		    			if (isset($_POST['comp_type']))
		    				$selected = $_POST['comp_type'];
		    			//het selecteren van de waarde die in de treeview is aangeklikt
		    			//in dit geval is er een type aangeklikt
		    			else if (isset($_GET['c']) && $_SESSION['type_overzicht'] == 2) {
		    				$selected = $_GET['c'];
		    			}
		    			//nu is er een component aangeklikt, dus hier het type van bepalen
		    			else if (isset($_GET['c'])){
		    				$query = "SELECT Comp_Type_ID FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
			    			$resultaat = mysql_query($query);
			    			$data = mysql_fetch_array($resultaat);
								$selected = $data['Comp_Type_ID'];
		    			}
		    			else $selected = 'SELECTED';

	    				//Type ophalen uit gebruikersgroeprechten
	    				$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '".$_SESSION['groep_id']."'";
	    			  $resultaat = mysql_query($query);
							$data = mysql_fetch_array($resultaat);

							Vul_Component_Types_Select_Box($data[0], $selected, false);
		    		?></select>
		    	</td>
  			</tr>
  			<tr>
  				<td>Naam component:</td>
  				<td><iframe id="frame_naam" name="frame_naam" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_naam.php?c=<?php echo($selected); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="150" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
  					<?php if (isset($_POST['opslaan']) && ($_POST['opslaan'] == 1) && isset($_POST['hidden_naam']) && $_POST['hidden_naam'] == '') echo("<b>* Er is geen naam voor deze instantie ingevuld!</b>");?></td>
  			</tr>
  			<tr>
  				<td>Parent component:</td>
					<?php 
						if(isset($_POST['hidden_parent']))// && isset($_POST['opslaan']) && $_POST['opslaan'] == 1) 
  						$selectie = "&n=" . $_POST['hidden_parent']; 
  					else $selectie = "&n=-1";
 						//bepalen hoogte van het scherm
						if(isset($_SESSION['admin_deel']) && 	$_SESSION['admin_deel'] > 0)
							$hoogte = 42;
						else $hoogte = 22;
 					?>
  				<td><iframe id="frame_parent" name="frame_parent" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_parent.php?c=<?php echo($selected . $selectie); ?>" width="550" height="<?php echo($hoogte); ?>" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
  			</tr>
  			<tr>
  				<td>Type melding:</td> 
  				<td><select name="type_melding" onchange="PostDocument('<?php echo($_SESSION['huidige_pagina']); ?>');">
  					<?php
  					
  							//onchange="switchMelding();"
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
  					?>
  					</select>&nbspAangemaakt:&nbsp<input name="statusdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['statusdatum'])) echo($_POST['statusdatum']); else echo(date('d-m-Y'));?>">
  					  <input name="statustijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['statustijd'])) echo($_POST['statustijd']); else echo(date('H:i'));?>">
  					  <?php if(isset($_POST['statusdatum']) && (!Valideer_Datum($_POST['statusdatum']) || !Valideer_Tijd($_POST['statustijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?></td>
					</td>
  			</tr>
				<tr>
					<td>Melding beschrijving:</td>					
					<td><iframe id="frame_melding" name="frame_melding" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_melding.php?c=<?php echo($meld_selectie); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="580" height="72" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
					</td>
				</tr>
  			<tr>
  				<td>Locatie:</td>
  				<td><select name="comp_locatie">
  				<?php
  					if(isset($_POST['comp_locatie']) && isset($_POST['opslaan']) && $_POST['opslaan'] == 1) 
  						$Selectie = $_POST['comp_locatie'];
  					else $Selectie = 'SELECTED';
  					
  					$query = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
	    			$resultaat = mysql_query($query);
				  	while ($data = mysql_fetch_array($resultaat)) {
	  	  			echo('<option value="'.$data['Locatie_ID'].'"');
	  	  			if ($Selectie == $data['Locatie_ID'] || $Selectie == 'SELECTED') {
	  	  				echo(' SELECTED');
	  	  				$Selectie = -1;
	  	  			}
	  	  			echo('>'. $data['Loc_Naam'] .'</option>');
				  	}
  				?>	
 					</select> Verantwoordelijke:
						<?php
		
							if (isset($_POST['hidden_verantwoordelijke'])) 
								$verantwoordelijke = $_POST['hidden_verantwoordelijke'];
							else {
		  					$query = "SELECT Type_Verantwoordelijke FROM comp_type WHERE Comp_Type = '". $selected ."'";
			    			$resultaat = mysql_query($query);
						  	$data = mysql_fetch_array($resultaat);
								$verantwoordelijke = $data[0];
							}
						?>
						<iframe id="frame_contact" name="frame_contact" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/type_verantwoordelijke.php?c=<?php echo($selected . "&s=" . $verantwoordelijke);?>" width="300" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
 				 </td>
  			</tr>
  			<tr>
  				<td>Fabricant contact:</td>
  				<?php
  					//instellen van de variabelen die meegegeven gaan worden
  					//c = het geselecteerde type om het standaard contact dat hierbij hoort te zoeken = $selected
  					//d = de datum = $datum
  					//t = de tijd = $tijd
  					//n = het geselecteerde type

  					if(isset($_POST['hidden_fabricant']) && isset($_POST['opslaan']) && $_POST['opslaan'] == 1) 
  						$selectie = "&n=" . $_POST['hidden_fabricant']; 
  					else $selectie = "&n=-1";

  					if(isset($_POST['hidden_fabricagedatum'])) 
  						$leverdatum = $_POST['hidden_fabricagedatum']; 
  					else $leverdatum = date('d-m-Y');
  					
						if(isset($_POST['hidden_fabricagetijd'])) 
							$levertijd = $_POST['hidden_fabricagetijd']; 
						else $levertijd = date('H:i');
  				?>
  				<td><iframe id="frame_fabricant" name="frame_fabricant" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_fabricant.php?c=<?php echo($selected ."&d=".$leverdatum . "&t=". $levertijd . $selectie); ?>" width="375" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
  					  <?php if(isset($_POST['hidden_fabricagedatum']) && (!Valideer_Datum($_POST['hidden_fabricagedatum']) || !Valideer_Tijd($_POST['hidden_fabricagetijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?></td>
  				</td>
  			</tr>
  			<tr>
  				<td>Leverancier contact:</td>
  				<?php
  					//instellen van de variabelen die meegegeven gaan worden
  					//c = het geselecteerde contact = $selected
  					//d = de datum = $datum
  					//t = de tijd = $tijd
  					//n = het geselecteerde type

  					if(isset($_POST['hidden_leverancier']) && isset($_POST['opslaan']) && $_POST['opslaan'] == 1) 
  						$selectie = "&n=" . $_POST['hidden_leverancier']; 
  					else $selectie = "&n=-1";

  					if(isset($_POST['hidden_leverdatum'])) 
  						$fabricagedatum = $_POST['hidden_leverdatum']; 
  					else $fabricagedatum = date('d-m-Y');
  					
						if(isset($_POST['hidden_levertijd'])) 
							$fabricagetijd = $_POST['hidden_levertijd']; 
						else $fabricagetijd = date('H:i');
  				?>
  				<td><iframe id="frame_leverancier" name="frame_leverancier" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/comp_toevoegen_leverancier.php?c=<?php echo($selected ."&d=".$fabricagedatum . "&t=". $fabricagetijd . $selectie);  ?>" width="375" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
 					  <?php if(isset($_POST['hidden_leverdatum']) && (!Valideer_Datum($_POST['hidden_leverdatum']) || !Valideer_Tijd($_POST['hidden_levertijd']))) echo('<b>* Onjuiste datum/tijd samenstelling!</b>'); ?></td>
  				</td>
  			</tr>
  			<tr>
  				<td>Extra velden:<br>(* = verplicht)</td>
  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/component_extra_velden.php?c=<?php echo($selected); ?>" width="400" height="100" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
  					<?php
  					  if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && !Extra_Velden_Controle()) echo("<b>* Foutieve waardes!</b>");
  					?>
  				</td>
  			</tr>
				<tr>
					<td>
  					<?php

		 					//eerst bepalen hoeveel velden er zijn, ivm met het datumveld dat uit 2 input boxen bestaat
							$query = "SELECT Kolom_ID FROM type_comp_koppel_extra WHERE Comp_Type_ID = '". $selected ."'";
							$resultaat = mysql_query($query);
							$aantal_velden = 0;
							$bestands_velden = 0;
							while ($data = mysql_fetch_row($resultaat)) {
								$query = "SELECT * FROM extra_velden WHERE Kolom_ID = '". $data[0]  ."'";
								$result = mysql_query($query);
								$velden = mysql_fetch_array($result);
								//4 = datumtijd
								if ($velden['DataType'] == 4)
									$aantal_velden++;
								if ($velden['DataType'] == 5)
									$bestands_velden++;
								
								$aantal_velden++;
							}
 							//het aantal onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"aantal\" name=\"aantal\" type=\"hidden\" value=\"".$aantal_velden."\">\n");
 							//het aantal bestandsvelden onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
 							echo("<input id=\"bestands_velden\" name=\"bestands_velden\" type=\"hidden\" value=\"".$bestands_velden."\">\n");
							
  						//5 hidden velden aanmaken voor elk extra veld: 1 voor de waarde, 1 voor het type en 1 voor de verplichtheid en 1 voor de ID van de parent record en 1 voor de veldnaam
							for($i = 0; $i < $aantal_velden; $i++){
	  						echo("<input id=\"".$i."\" name=\"".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST[$i])) echo($_POST[$i]);
	  						echo("\">");
  							echo("<input id=\"t".$i."\" name=\"t".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['t'.$i])) echo($_POST['t'.$i]);
	  						echo("\">");
  							echo("<input id=\"v".$i."\" name=\"v".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['v'.$i])) echo($_POST['v'.$i]);
	  						echo("\">");
  							echo("<input id=\"i".$i."\" name=\"i".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['i'.$i])) echo($_POST['i'.$i]);
	  						echo("\">");
  							echo("<input id=\"n".$i."\" name=\"n".$i."\" type=\"hidden\" value=\"");
	  						if (isset($_POST['n'.$i])) echo($_POST['n'.$i]);
	  						echo("\">\n");
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
  					<input id="opslaan" name="opslaan" type="hidden" value="1">
  					<input id="hidden_fabricagedatum" name="hidden_fabricagedatum" type="hidden" value="<?php echo($fabricagedatum); ?>">
  					<input id="hidden_fabricagetijd" name="hidden_fabricagetijd" type="hidden" value="<?php echo($fabricagetijd); ?>">
						<input id="hidden_leverdatum" name="hidden_leverdatum" type="hidden" value="<?php echo($leverdatum); ?>">
						<input id="hidden_levertijd" name="hidden_levertijd" type="hidden" value="<?php echo($levertijd); ?>">
						<input id="hidden_parent" name="hidden_parent" type="hidden" value="">
						<input id="hidden_schaduw" name="hidden_schaduw" type="hidden" value="">
						<input id="hidden_naam" name="hidden_naam" type="hidden" value="">
						<input id="hidden_fabricant" name="hidden_fabricant" type="hidden" value="">
						<input id="hidden_leverancier" name="hidden_leverancier" type="hidden" value="">
  					<input id="hidden_melding" name="hidden_melding" type="hidden" value="">
  					<input id="hidden_status" name="hidden_status" type="hidden" value=""></td>
					<td><input name="hidden_verantwoordelijke" id="hidden_verantwoordelijke" type="hidden" value="-1"><a href="javascript:submitComponentToevoegen();">Toevoegen</a></td>
  			</tr>
  		</table>
  	</form>
	
	<?php

		}
?>