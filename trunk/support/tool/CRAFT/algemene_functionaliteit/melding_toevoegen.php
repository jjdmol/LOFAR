<?php

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

	//functie welke controleert of de ingevoerde datum wel correct is
	//dwz: de ingevoerde datum moet hoger dan zijn voorligger zijn
	function Check_Melding_Datum() {
		if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {

			if (isset($_SESSION['type_overzicht']) && $_SESSION['type_overzicht'] == '2') 
				$Comp_Selectie = $_POST['Comp_Selection'];
			else 
				$Comp_Selectie = $_GET['c'];

			//bij de laatste melding van dit component kijken of de ingevoerde datum hoger is dan die ingevoerde datum
			$query = "SELECT Meld_Datum FROM melding_lijst WHERE Meld_Lijst_ID IN (SELECT Laatste_Melding FROM comp_lijst WHERE Comp_Lijst_ID = '" . $Comp_Selectie . "')";
			$result = mysql_query($query);
			$data = mysql_fetch_array($result);
			if ($data['Meld_Datum'] > Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum'], $_POST['Meld_Tijd']))
				return false;
		}
		return true;
	}
	

	function Valideer_Invoer() {
		if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
			return false;
		
		//Meldingdatum controle
		if (isset($_POST['Meld_Datum'])) {
			//wanneer de statusdatum gevuld is, dan...
			if($_POST['Meld_Datum'] !='') {
				
				//controleren op de juiste samenstelling van de statusdatum
				if (Valideer_Datum($_POST['Meld_Datum']) == false)
				return false;
			
				//controleren of de tijd correct ingevoerd is
				if(isset($_POST['Meld_Tijd'])) {
				  if (Valideer_Tijd($_POST['Meld_Tijd']) == false)
				  	return false;
				}
			}
		} 
		
		if (isset($_POST['Comp_Selection']) && $_POST['Comp_Selection'] == -1)
			return false;
		
		
		//meldingdatum checken
		if (Check_Melding_Datum() == false) {
			return false;
		}
		
		//beschrijving
		if (isset($_POST['hidden_beschrijving'])) {
			if ($_POST['hidden_beschrijving'] == '')
				return false;
		} else return false;

		return Extra_Velden_Controle();

		return true;
	}


	if (Valideer_Invoer()) {
		if (isset($_SESSION['type_overzicht']) && $_SESSION['type_overzicht'] == '2') 
			$Comp_Selectie = $_POST['Comp_Selection'];
		else 
			$Comp_Selectie = $_GET['c'];

		
		//uit de componenten lijst halen welke melding hier als laatste bij opgeslagen is
		//deze waarde is nodig om een keten van meldingen te kunnen vormen
		$query = "SELECT Laatste_Melding, Comp_Parent FROM comp_lijst WHERE Comp_Lijst_ID = '". $Comp_Selectie ."'";
		$resultaat = mysql_query($query);
  	$row = mysql_fetch_array($resultaat);
		
		//de query om de melding toe te voegen, samenstellen
		$query = "INSERT INTO melding_lijst (Meld_Type_ID, Comp_Parent, Melding_Locatie, Comp_Lijst_ID, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Prob_Oplossing, Behandeld_Door, Gemeld_Door, Afgehandeld)";
		$query = $query . "VALUES ('". $_POST['Type_Melding'] ."', '". $row['Comp_Parent']. "', '". $_POST['Melding_Locatie'] ."', '" .$Comp_Selectie . "'";

		//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
		if (isset($_POST['Meld_Datum']) && $_POST['Meld_Datum'] != '') {
			$datum=split("-",$_POST['Meld_Datum']);
			$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['Meld_Tijd'] .":00'";							
		}
		else $query = $query . ", NOW()";
		$query = $query . ", '". $_POST['hidden_status'] ."', '". $row['Laatste_Melding'] ."', '". htmlentities($_POST['hidden_beschrijving'], ENT_QUOTES) ."', '". htmlentities($_POST['hidden_oplossing'], ENT_QUOTES);
		$query = $query . "', '". $_POST['Behandeld_Door'] ."', '". $_POST['Gemeld_Door'] ."', '";
		//de afgehandeld checkbox vertalen naar sql taal ;)
		if (isset($_POST['afgehandeld']) && ($_POST['afgehandeld'] == 'on' || $_POST['afgehandeld'] == '1'))
			$query = $query . "1') ";
		else $query = $query . "0') ";

		$errorlevel = 0;
		//uitvoeren van de insert query
		if (mysql_query($query)) {
			$errorlevel = 1;

			//de id van de zojuist toegevoegde melding halen
			$Laatste_Melding = mysql_insert_id();
			//het component waar deze melding bijhoort bijwerken, zodat deze weet dat er een nieuwe laatste_melding is (einde van de keten)
			$query = "UPDATE comp_lijst SET Laatste_Melding='". $Laatste_Melding ."' WHERE Comp_Lijst_ID='". $Comp_Selectie ."'";
			
			if (mysql_query($query)) {
				$errorlevel = 2;
					
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
	  				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['n'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['i'.$i] ."', '4', '".$_POST['v'.$i]."')";
	
	    			if (mysql_query($query)) {
			 				$error_extra = 2;
	
	    				$Veld_ID = mysql_insert_id();	  				
	  					$query = "INSERT INTO melding_koppel_extra (Kolom_ID, Meld_Lijst_ID) VALUES('".$Veld_ID. "', '". $Laatste_Melding ."')";
		    			
		    			if (mysql_query($query)) {
				 				$error_extra = 3;
			  			}
	    			}
					}
				}
			}
		}

		//de foutmeldingen
		if ($errorlevel == 2) {
			if($error_extra == 3) echo("De nieuwe melding (". $Laatste_Melding .") is (inclusief extra velden) aan het systeem toegevoegd!<br>");
			else if ($error_extra == 1) echo("De nieuwe melding (". $Laatste_Melding .") is succesvol aan het systeem toegevoegd!<br>Alleen het toevoegen van extra velden (in de extra_velden tabel) is mislukt!<br>");
			else if ($error_extra == 2) echo("De nieuwe melding (". $Laatste_Melding .") is succesvol aan het systeem toegevoegd!<br>Alleen het leggen van de koppeling tussen de extra velden en de melding is mislukt!<br>");
			else if ($error_extra == 0) {
				if($_POST['aantal'] == 0)
					echo("De nieuwe melding (". $Laatste_Melding .") is succesvol aan het systeem toegevoegd!<br>");
				else echo("De nieuwe melding (". $Laatste_Melding .") is succesvol aan het systeem toegevoegd!<br>Alleen het toevoegen van extra velden (in de datatabel) is mislukt!<br>");
			}
		}
		else if ($errorlevel == 0) echo("De nieuwe melding (". $Laatste_Melding .") kon niet aan het systeem toegevoegd worden!.");
		else if ($errorlevel == 1) echo("De nieuwe melding (". $Laatste_Melding .") is aan het systeem toegevoegd.<br>Alleen is er iets foutgegaan met het updaten van de componten tabel! De 'laatste meldin' verwijzing is niet geupdated!");
		$url = '';
		if(isset($_GET['c'])) $url = $url . ("&c=" . $Comp_Selectie);		
		if(isset($_GET['b'])) $url = $url . ("&b=" . $_GET['b']);
		
		echo('<a href="'.$_SESSION['huidige_pagina']. $url .'">Klik hier om nog een melding aan dit component toe te voegen of geselecteer een component uit de treeview.</a>');
	}
	else {
		if ((isset($_GET['c']) && $_GET['c'] != 0) || (isset($_GET['b']))) {
			//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
			//date_default_timezone_set("Europe/Amsterdam");
			putenv("TZ=Europe/Amsterdam");
	
?>

    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); if(isset($_GET['c'])) echo("&c=" . $_GET['c']); if(isset($_GET['b'])) echo("&b=".$_GET['b']); ?>">
    		<table>
  				<?php
  					if (isset($_SESSION['type_overzicht']) && $_SESSION['type_overzicht'] == '2') {
							if (isset($_POST['Comp_Selection']))
								$Comp_Selection = $_POST['Comp_Selection'];
							else $Comp_Selection = -1;

							echo("<tr><td>Component:</td><td><select name=\"Comp_Selection\" onChange=\"PostDocument('" . $_SESSION['huidige_pagina'] ."');\">");
							echo("<option value=\"-1\"");
							if ($Comp_Selection == -1) echo (" SELECTED");
							echo(">None selected</option>");
							Vul_Componenten_Select_Box(Bepaal_Types(), $Comp_Selection);
							echo("</select>");

							if(isset($_POST['Comp_Selection']) && ($_POST['Comp_Selection'] == -1)) echo('<b>* Er is geen component geselecteerd!</b>'); 
							echo("</td></tr>");
						}
						else {
							//het weergeven van de naam van het geselecteerde component
							$query = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
	    			  $resultaat = mysql_query($query);
					  	$data = mysql_fetch_array($resultaat);
					  	echo("<tr><td>Component:</td><td>".$data[0]."</td></tr>");
						}
  				?>    				
    			<tr>    				
    				<td>Type melding:</td>
	  				<?php
	  					if (isset($_SESSION['type_overzicht']) && $_SESSION['type_overzicht'] == '2') {
	  						$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID = '".$_GET['b']."'";
		    			  $resultaat = mysql_query($query);
						  	$data = mysql_fetch_array($resultaat);
						  	
						  	$type = $_GET['b'];
						  	echo("<td><input type=\"hidden\" name=\"Type_Melding\" id=\"Type_Melding\" value=\"".$type."\">");
	  						echo($data['Melding_Type_Naam'] . "&nbsp&nbsp&nbsp&nbsp");
	  					}
	  					else {
	  					
		  					if (isset($_GET['c'])) {
			    				echo("<td><select name=\"Type_Melding\" onChange=\"PostDocument('" . $_SESSION['huidige_pagina'] . "&c=" . $_GET['c'] ."');\">");	
								}
								else 
			    				echo("<td><select name=\"Type_Melding\" onChange=\"PostDocument('" . $_SESSION['huidige_pagina'] ."');\">");
	
	  						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
		    			  $resultaat = mysql_query($query);
	
					  		if (isset($_GET['b'])) $type = $_GET['b'];
					  		else if (isset($_POST['Type_Melding'])) $type = $_POST['Type_Melding'];
					  		else $type = 'SELECTED';
	
						  	while ($data = mysql_fetch_array($resultaat)) {
		  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
			  	  			if ($data['Meld_Type_ID'] == $type || $type == 'SELECTED') {
			  	  				echo('SELECTED');
			  	  				$type = $data['Meld_Type_ID'];
			  	  			}
			  	  			echo('>'. $data['Melding_Type_Naam'] .'</option>');
								}    					
								echo("</select> ");
							}
  					?>
    				 Locatie melding: <select name="Melding_Locatie">
						<?php
							$query = "SELECT Comp_Locatie FROM comp_lijst";
	    			  $resultaat = mysql_query($query);
					  	$row = mysql_fetch_array($resultaat);

							$query = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
	    			  $resultaat = mysql_query($query);

				  		if (isset($_POST['Melding_Locatie'])) $selectie = $_POST['Melding_Locatie'];
				  		else $selectie = $row[0];

					  	while ($data = mysql_fetch_array($resultaat)) {
	  	  				echo('<option value="'.$data['Locatie_ID'].'"');
		  	  			if ($data['Locatie_ID'] == $selectie)
		  	  				echo('SELECTED');
		  	  			echo('>'. $data['Loc_Naam'] .'</option>');
							}
						?>	
						</select>    				
    				</td>
    			</tr>
    			<tr>
    				<td>Gemeld door:</td>
    				<td><select name="Gemeld_Door">
    					<?php 
								$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
		    			  $resultaat = mysql_query($query);

					  		if (isset($_POST['Gemeld_Door'])) $selectie = $_POST['Gemeld_Door'];
					  		else $selectie = 'SELECTED';

						  	while ($data = mysql_fetch_array($resultaat)) {
		  	  				echo('<option value="'.$data['Werknem_ID'].'"');
			  	  			if ($data['Werknem_ID'] == $selectie || $selectie == 'SELECTED') {
			  	  				echo('SELECTED');
			  	  				$selectie = $data['Werknem_ID'];
			  	  			}
			  	  			echo('>'. $data['inlognaam'] .'</option>');
								}
    					?>
    				</select> op <input name="Meld_Datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum'])) echo($_POST['Meld_Datum']); else echo(date('d-m-Y'));?>">
    					  <input name="Meld_Tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd'])) echo($_POST['Meld_Tijd']); else echo(date('H:i'));?>">
    					  <?php if(isset($_POST['Meld_Datum']) && (!Valideer_Datum($_POST['Meld_Datum']) || !Valideer_Tijd($_POST['Meld_Tijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); 
	   					  			if (!Check_Melding_Datum()) echo("<b>* De ingevoerde datum is te laag!</b>");
    					  ?>
    					</td>
    				</td>
    			</tr>
    			<tr>
    				<td>Behandeld door:</td>
    				<td><select name="Behandeld_Door">
    					<?php 
								$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
		    			  $resultaat = mysql_query($query);

					  		if (isset($_POST['Behandeld_Door'])) $selectie = $_POST['Behandeld_Door'];
					  		else $selectie = 'SELECTED';

						  	while ($data = mysql_fetch_array($resultaat)) {
		  	  				echo('<option value="'.$data['Werknem_ID'].'"');
			  	  			if ($data['Werknem_ID'] == $selectie || $selectie == 'SELECTED') {
			  	  				echo('SELECTED');
			  	  				$selectie = $data['Werknem_ID'];
			  	  			}
			  	  			echo('>'. $data['inlognaam'] .'</option>');
								}
    					?>	
   					</select> Afgehandeld: <input name="afgehandeld" type="checkbox"
    				<?php
	    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
	    					if(isset($_POST['afgehandeld']) && ($_POST['afgehandeld'] == 1 || $_POST['afgehandeld'] == 'on')) 
	    					echo('CHECKED');
  						}
  						?>>
   					</td>
    			</tr>
    			<tr>
    				<td>Probleem beschrijving:</td>
    				<td><iframe id="frame_beschrijving" name="frame_beschrijving" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/melding_probleem_beschrijving.php<?php if(isset($type)) echo("?c=".$type); ?>" width="305" height="56" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
    					<?php if(isset($_POST['opslaan']) && ($_POST['opslaan'] == 1) && ($_POST['hidden_beschrijving'] == '')) echo('<b>* Er is geen beschrijving ingevoerd!</b>'); ?></td>
    			</tr>
    			<tr>
    				<td>Probleem oplossing:</td>
    				<td><iframe id="frame_oplossing" name="frame_oplossing" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/melding_probleem_oplossing.php<?php if(isset($type)) echo("?c=".$type); ?>" width="305" height="56" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
    			</tr>
    			<tr>
	  				<td>Extra velden:<br>(* = verplicht)</td>
	  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/Melding_Toevoegen_Extra_Velden.php?c=<?php echo($type); ?>" width="400" height="93" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
	  					<?php
	  					  if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && !Extra_Velden_Controle()) echo("<b>* Foutieve waardes!</b>");
	  					?>
	  				</td>
    			</tr>
    			<tr>
  					<?php
  						$dinges = "";
  						if (isset($_SESSION['type_overzicht']) && $_SESSION['type_overzicht'] == '2') {
  							if (isset($_POST['Comp_Selection']) && $_POST['Comp_Selection'] != -1)
  								$dinges = ("?c=".$_POST['Comp_Selection']);
  						}
  						else if(isset($_GET['c'])) $dinges = ("?c=".$_GET['c']);
  					?>

    				<td>Historie:</td>
    				<td><iframe id="frame_historie" name="frame_historie" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/melding_historie.php<?php echo($dinges); ?>" width="500" height="88" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
    			</tr>
    			<tr>
    				<td>
	  					<?php
			 				
			 					//eerst bepalen hoeveel velden er zijn, ivm met het datumveld dat uit 2 input boxen bestaat
								$query = "SELECT Kolom_ID FROM type_melding_koppel_extra WHERE Meld_Type_ID = '". $type ."'";
								$resultaat = mysql_query($query);
								$aantal_velden = 0;
								//een bestands_velden variabele, om bij te houden hoeveel bestanden hier maximaal geupload kunnen worden
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
 	 					  ?>
    					<input name="hidden_status" id="hidden_status" type="hidden" value="">
    					<input name="hidden_beschrijving" id="hidden_beschrijving" type="hidden" value="">
    					<input name="hidden_oplossing" id="hidden_oplossing" type="hidden" value="">
    					<input name="opslaan" type="hidden" value="1">
    				</td>
    				<td><a href="javascript:SubmitMeldingToevoegen();">Toevoegen</a></td>
    			</tr>
    		</table>
    	</form>
	<?php
			}
			else echo('Selecteer hiernaast een component om een melding aan toe te voegen.');
		}
	?>
