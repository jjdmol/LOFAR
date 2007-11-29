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

	
	//controle functie om te bekijken of er opgeslagen mag worden
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
		
		//meldingdatum checken
		if (Check_Melding_Datum() == false) {
			return false;
		}
		
		//beschrijving
		if (isset($_POST['Prob_Beschrijving'])) {
			if ($_POST['Prob_Beschrijving'] == '')
				return false;
		} else return false;

		return Extra_Velden_Controle();
		
		return true;
	}
	
	//functie welke controleert of de ingevoerde datum wel correct is
	//dwz: de ingevoerde datum moet hoger dan zijn voorligger zijn en lager dan de datum na hem
	function Check_Melding_Datum() {
		if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
		
			//De melding is onderdeel van een keten, als de melding datum daardoor lager is dan de datum van de voorgaande melding
			$query = "SELECT Meld_Datum FROM melding_lijst WHERE Meld_Lijst_ID IN (SELECT Voorgaande_Melding FROM melding_lijst m WHERE Meld_Lijst_ID = '".$_GET['m']."')";
			$result = mysql_query($query);
			$data = mysql_fetch_array($result);
			if ($data['Meld_Datum'] > Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum'], $_POST['Meld_Tijd']))
				return false;
	
			//of hoger is dan de melding na deze melding, dan is de keten corrupt.		
			$query = "SELECT Meld_Datum FROM melding_lijst WHERE Voorgaande_Melding = '".$_GET['m']."'";
			$result = mysql_query($query);
			$data = mysql_fetch_array($result);
			if (isset($data['Meld_Datum']) && ($data['Meld_Datum'] < Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum'], $_POST['Meld_Tijd'])))
				return false;
		}
		return true;
	}
	
	
	
	//controleren of er opgeslagen moet worden, of dat er een ander scherm getoond moet worden
	if(Valideer_Invoer()) {
		$query = "SELECT Meld_Type_ID FROM melding_lijst WHERE Meld_Lijst_ID = '".$_GET['m']."'";
		$result = mysql_query($query);
		$data = mysql_fetch_array($result);
		
		if (isset($_POST['Afgehandeld']) && ($_POST['Afgehandeld'] == 1 || $_POST['Afgehandeld'] == 'on'))
			$afgehandeld = 1;
		else $afgehandeld = 0;
		
		//query samenstellen
		$query = "UPDATE melding_lijst SET Meld_Type_ID='". $_POST['Type_Melding'] ."', Melding_Locatie='".$_POST['Melding_Locatie']."', Gemeld_Door='". $_POST['Gemeld_Door'] ."', ";
		$query = $query . "Prob_Beschrijving='". htmlentities($_POST['Prob_Beschrijving'], ENT_QUOTES)."', Prob_Oplossing='". htmlentities($_POST['Prob_Oplossing'], ENT_QUOTES) ."', Behandeld_Door='".$_POST['Behandeld_Door']."', ";
		$query = $query . "Afgehandeld='". $afgehandeld ."', Meld_Datum=";

		//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
		if (isset($_POST['Meld_Datum']) && $_POST['Meld_Datum'] != '') {
			$query = $query . "'".Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum'], $_POST['Meld_Tijd'])."'";
		}
		else $query = $query . "NOW()";
		
		$query = $query . " WHERE Meld_Lijst_ID='".$_GET['m']."'";

		$errorlevel = 0;
		if (mysql_query($query)) {
			//extra velden gedeelte!!!!!!!!!!!!
			$errorlevel = 1;
			
			
			//type controle: als het type melding verandert is, is de extra velden samensteling anders,
			//dus moet alles opnieuw aangemaakt worden..
			if ($data['Meld_Type_ID'] != $_POST['Type_Melding']) {

				//kijken of er wel entries in de datatabel voor dit component zijn
				$query = "SELECT COUNT(Kolom_ID) FROM melding_koppel_extra WHERE Meld_Lijst_ID = '". $_GET['m'] ."'";
				$resultaat2 = mysql_query($query);
				$tijdelijk = mysql_fetch_array($resultaat2);
				//wanneer de count waarde hoger is dan 0, dan zijn er entries en moeten deze verwijderd worden
				if ($tijdelijk[0] > 0) {
					
					//bij verandering van type is alles nieuw, dus alle oude dingen weggooien
					$opslag_Kolom_ID = "";
					$query = "SELECT Kolom_ID FROM melding_koppel_extra WHERE Meld_Lijst_ID = '".$_GET['m']."'";
					$resultaat2 = mysql_query($query);
					while ($tijdelijk = mysql_fetch_array($resultaat2)) {
			 			if ($opslag_Kolom_ID != "") $opslag_Kolom_ID = $opslag_Kolom_ID . ',';
			 			$opslag_Kolom_ID = $opslag_Kolom_ID . $tijdelijk['Kolom_ID'];
					}
					$query = "DELETE FROM melding_koppel_extra WHERE Meld_Lijst_ID = '".$_GET['m']."'";
					mysql_query($query);
					
					$query = "SELECT Data_Kolom_ID, DataType FROM extra_velden WHERE Kolom_ID IN(".$opslag_Kolom_ID.")";
					$opslag_Data_Kolom_ID = "";
					$bestanden_Data_Kolom_ID = "";
					$resultaat2 = mysql_query($query);
					while ($tijdelijk = mysql_fetch_array($resultaat2)) {
			 			//de id's van de bestanden opslaan!!!!
			 			if ($tijdelijk['DataType'] == 5) {
				 			if ($bestanden_Data_Kolom_ID != "") $bestanden_Data_Kolom_ID = $bestanden_Data_Kolom_ID . ',';
				 			$bestanden_Data_Kolom_ID = $bestanden_Data_Kolom_ID . $tijdelijk['Data_Kolom_ID'];
			 			}
			 			if ($opslag_Data_Kolom_ID != "") $opslag_Data_Kolom_ID = $opslag_Data_Kolom_ID . ',';
			 			$opslag_Data_Kolom_ID = $opslag_Data_Kolom_ID . $tijdelijk['Data_Kolom_ID'];
					}
					
					//Het bestand waarnaar verwezen wordt van de server verwijderen!!!
					$query = "SELECT Type_TinyText FROM datatabel WHERE Data_Kolom_ID IN (".$bestanden_Data_Kolom_ID.")";
					$rest2 = mysql_query($query);
					while ($tijdelijk = mysql_fetch_array($rest2)) {
						unlink('algemene_functionaliteit/'. $tijdelijk[0]);
					}

					$query = "DELETE FROM extra_velden WHERE Kolom_ID IN(".$opslag_Kolom_ID.")";
					mysql_query($query);				
					
					$query = "DELETE FROM datatabel WHERE Data_Kolom_ID IN(".$opslag_Data_Kolom_ID.")";
					mysql_query($query);
				}
					
				//het oude is nu verwijderd, dus kan het nieuwe toegevoegd worden...
				if (isset($_POST['aantemaken']) && $_POST['aantemaken'] > 0) {
					$errorlevel = 2;
					for ($i = 0; $i < $_POST['aantemaken']; $i++){
	 				
		 				//DataTabel entry maken
		  			//integer
		  			if ($_POST['at' . $i] == '1') 		  $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST['a'.$i]."')";
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
	    				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['an'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['ai'.$i] ."', '4', '".$_POST['av'.$i]."')";
	
		    			if (mysql_query($query)) {
				 				$error_extra = 2;
	
		    				$Veld_ID = mysql_insert_id();	  				
		  					$query = "INSERT INTO melding_koppel_extra (Kolom_ID, Meld_Lijst_ID) VALUES('".$Veld_ID. "', '". $_GET['m'] ."')";
			    			
			    			if (mysql_query($query)) {
					 				$error_extra = 3;
	  		  			}
		    			}
						}
					}
				}				
			}
			//type melding is niet verandert, dus de datatabel updaten en eventuele nieuwe extra velden toevoegen
			else {
			
				//aantal velden controle.. meer dan 0 dan dit doen, anders overslaan
				if (isset($_POST['aantal']) && $_POST['aantal'] > 0) {
					//voor elk veld het record updaten
					for ($i = 0; $i < $_POST['aantal']; $i++){

	    			//oude waarde op null zetten en dan opslaan
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
	
//						$query = "UPDATE datatabel SET " . $type . " ='" . $waarde . "' WHERE Data_Kolom_ID = '". $veld['Data_Kolom_ID'] ."'";
						
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
	    				$query = $query . "VALUES ('".$Veld_ID."', '".$_POST['an'.$i]."', '".$_SESSION['gebr_id'] ."', '".$datatype."', '". $_POST['ai'.$i] ."', '4', '".$_POST['av'.$i]."')";
	
		    			if (mysql_query($query)) {
				 				$error_extra = 2;
	
		    				$Veld_ID = mysql_insert_id();	  				
		  					$query = "INSERT INTO melding_koppel_extra (Kolom_ID, Meld_Lijst_ID) VALUES('".$Veld_ID. "', '". $_GET['m'] ."')";
			    			
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
		if ($errorlevel == 2) {
			if($error_extra == 3) echo("De melding (". $_GET['m'] .") is (inclusief extra velden) in het systeem bijgewerkt!<br>");
			else if ($error_extra == 1) echo("De melding (". $_GET['m'] .") is succesvol in het systeem bijgewerkt!<br>Alleen het toevoegen van extra velden (in de extra_velden tabel) is mislukt!<br>");
			else if ($error_extra == 2) echo("De melding (". $_GET['m'] .") is succesvol in het systeem bijgewerkt!<br>Alleen het leggen van de koppeling tussen de extra velden en de melding is mislukt!<br>");
			else if ($error_extra == 0) {
				if($_POST['aantemaken'] == 0)
					echo("De melding (". $_GET['m'] .") is succesvol in het systeem bijgewerkt!<br>");
				else echo("De melding (". $_GET['m'] .") is succesvol in het systeem bijgewerkt!<br>Alleen het toevoegen van extra velden (in de datatabel) is mislukt!<br>");
			}
		}
		else if ($errorlevel == 0) echo("De bewerkte melding (". $_GET['m'] .") kon niet in het systeem opgeslagen worden!.");
		else if ($errorlevel == 1) {
			echo("De melding (". $_GET['m'] .") is in het systeem opgeslagen.");
			if (isset($_POST['aantal']) && $_POST['aantal'] > 0)
				echo("<br>Alleen is er iets foutgegaan met het updaten van de datatabel!");
		}

		echo('<br><a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '&m='.$_GET['m'].'">Klik hier om deze melding opnieuw te bewerken/bewerken.</a>');
		echo('<br><a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het meldingenoverzicht van dit component.</a>');
		echo('<br>of selecteer links een component uit de treeview.');
	}
	//niet opslaan
	else {
		//er is een component geselecteerd
		if (isset($_GET['c']) && $_GET['c'] != 0 || isset($_GET['b']) && $_GET['b'] != 0) {

			
			//er is een melding geselecteerd, dus de gegevens van die melding weergeven
			if (isset($_GET['m']) && $_GET['m'] != 0) {
				$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID='". $_GET['m'] ."'";
				$resultaat = mysql_query($query);
				$row = mysql_fetch_array($resultaat);
				
				if (isset($_GET['b'])) $geselecteerd_type = $_GET['b'];
				else $geselecteerd_type = $row['Meld_Type_ID'];
				
				?>
					<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']."&m=".$_GET['m']. "&b=". $geselecteerd_type); ?>">
						<table>
							<tr>
								<td>Type melding:</td>
								<td><select name="Type_Melding" onChange="PostDocument('<?php echo($_SESSION['huidige_pagina'] . "&c=" . $_GET['c'] . "&o=" . $row['Meld_Type_ID'] . "&m=". $_GET['m']); ?>');">
								<?php
	    						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
			    			  $resultaat = mysql_query($query);
	
						  		if (isset($_GET['q'])) $type = $_GET['q'];
						  		else if (isset($_POST['Type_Melding'])) $type = $_POST['Type_Melding'];
						  		else $type = $row['Meld_Type_ID'];
	
							  	while ($data = mysql_fetch_array($resultaat)) {
			  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
				  	  			if ($data['Meld_Type_ID'] == $type) 
				  	  				echo('SELECTED');
				  	  			echo('>'. $data['Melding_Type_Naam'] .'</option>');
									}
								?>	
								</select> Locatie melding: <select name="Melding_Locatie">
								<?php
									$query = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
			    			  $resultaat = mysql_query($query);
	
						  		if (isset($_POST['Melding_Locatie'])) $selectie = $_POST['Melding_Locatie'];
						  		else $selectie = $row['Melding_Locatie'];
	
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
						  		else $selectie = $row['Gemeld_Door'];
	
							  	while ($data = mysql_fetch_array($resultaat)) {
			  	  				echo('<option value="'.$data['Werknem_ID'].'"');
				  	  			if ($data['Werknem_ID'] == $selectie)
				  	  				echo('SELECTED');
				  	  			echo('>'. $data['inlognaam'] .'</option>');
									}
								?>
								</select> op <?php
										if (isset($_POST['datumopslag']))
											$hidden_datum = $_POST['datumopslag'];
										else $hidden_datum = $row['Meld_Datum'];
										
										//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			    					$gedeeldveld=split(" ",$hidden_datum);
			    					
										//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
										$datum = split("-",$gedeeldveld[0]);
										//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
										$tijd = split(":",$gedeeldveld[1]);
									?>
			    				<input name="Meld_Datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum'])) echo($_POST['Meld_Datum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
    					  	<input name="Meld_Tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd'])) echo($_POST['Meld_Tijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
	    					  <?php if(isset($_POST['Meld_Datum']) && (!Valideer_Datum($_POST['Meld_Datum']) || !Valideer_Tijd($_POST['Meld_Tijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); 
	    					  			if (!Check_Melding_Datum()) echo("<b>* De ingevoerde datum is te hoog of te laag!</b>");
	    					  ?>
								</td>
							</tr>
							<tr>
								<td>Behandeld door:</td>
								<td><select name="Behandeld_Door">
		    					<?php 
										$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
				    			  $resultaat = mysql_query($query);
		
							  		if (isset($_POST['Behandeld_Door'])) $selectie = $_POST['Behandeld_Door'];
							  		else $selectie = $row['Behandeld_Door'];
		
								  	while ($data = mysql_fetch_array($resultaat)) {
				  	  				echo('<option value="'.$data['Werknem_ID'].'"');
					  	  			if ($data['Werknem_ID'] == $selectie || $selectie == 'SELECTED')
					  	  				echo('SELECTED');
					  	  			echo('>'. $data['inlognaam'] .'</option>');
										}
		    					?>	
								</select> Afgehandeld: <input name="Afgehandeld" type="checkbox"
			    				<?php
				    				if(isset($_POST['opslaan'])) {
				    					if(isset($_POST['Afgehandeld']) && ($_POST['Afgehandeld'] == 1 || $_POST['Afgehandeld'] == 'on')) 
				    					echo('CHECKED');
				    				}
				    				else if ($row['Afgehandeld'] == 1) echo('CHECKED');
									?>>
								</td>
							</tr>							
							<tr>
								<td>Probleem beschrijving:</td>
								<td><textarea name="Prob_Beschrijving" rows="3" cols="35"><?php if(isset($_POST['opslaan'])) echo(htmlentities($_POST['Prob_Beschrijving'], ENT_QUOTES)); else echo($row['Prob_Beschrijving']); ?></textarea>
									<?php if(isset($_POST['Prob_Beschrijving']) && $_POST['Prob_Beschrijving'] == '') echo('<b>* Er is geen probleem beschrijving ingevoerd!</b>'); ?>
								</td>
							</tr>
							<tr>
								<td>Probleem oplossing:</td>
								<td><textarea name="Prob_Oplossing" rows="3" cols="35"><?php if(isset($_POST['opslaan'])) echo(htmlentities($_POST['Prob_Oplossing'], ENT_QUOTES)); else echo($row['Prob_Oplossing']); ?></textarea></td>
							</tr>
							<tr>
			  				<td>Extra velden:<br>(* = verplicht)</td>
			  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/Melding_Bewerken_Extra_Velden.php?c=<?php echo($_GET['m'] . '&o=' . $row['Meld_Type_ID'] . '&n=' .$type); ?>" width="400" height="110" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
			  					<?php
			  					  if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && !Extra_Velden_Controle()) echo("<b>* Foutieve waardes!</b>");
			  					?>
			  				</td>
							</tr>
							<tr>
								<td><a href="<?php echo($_SESSION['huidige_pagina']."&c=".$_GET['c']); ?>">Overzicht meldingen</a></td>
								<td><input type="hidden" name="opslaan" value="1"><a href="javascript:SubmitMeldingBewerken();">Opslaan</a>
		  					<?php
				 					//type is gelijk gebleven, dus kunnen extra velden en nog niet aangemaakte velden aanwezig zijn
				 					if ($row['Meld_Type_ID'] == $type){
					 					//eerst bepalen hoeveel velden er zijn, ivm met het datumveld dat uit 2 input boxen bestaat
										$query = "SELECT Kolom_ID FROM melding_koppel_extra WHERE Meld_Lijst_ID = '". $_GET['m'] ."'";
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
										//nog niet aangemaakte extra velden (die later toegevoegd zijn) toevoegen
										$query = "SELECT Kolom_ID FROM type_melding_koppel_extra WHERE Meld_Type_ID in (SELECT Meld_Type_ID FROM melding_lijst WHERE Meld_Lijst_ID = '". $_GET['m'] ."')";
										$rest = mysql_query($query);
										$bestand_aanmaken = 0;
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
				 					}
				 					//het type is veranderd, dus alleen a velden plaatsen
				 					else {
										$aantal_velden = 0;
			 							//het aantal onthouden zodat er nadat er gepost is, gemakkelijk door de velden geitereerd kunnen worden
			 							echo("<input id=\"aantal\" name=\"aantal\" type=\"hidden\" value=\"".$aantal_velden."\">\n");
										//de hidden velden voor de nog niet aangemaakte extra velden aanmaken
										$aan_te_maken = 0;
										$bestand_aanmaken = 0;
										//nog niet aangemaakte extra velden (die later toegevoegd zijn) toevoegen
										$query = "SELECT Kolom_ID FROM type_melding_koppel_extra WHERE Meld_Type_ID = '". $type ."'";
										$rest = mysql_query($query);
										while ($data = mysql_fetch_array($rest)) {
											$query = "SELECT DataType FROM extra_velden WHERE Kolom_ID = '". $data['Kolom_ID'] ."'";
											$resultt = mysql_query($query);
											$uitkomst = mysql_fetch_array($resultt);
		
											if ($uitkomst['DataType'] == 4)
												$aan_te_maken++;
											if ($uitkomst['DataType'] == 5)
												$bestand_aanmaken++;
			
											$aan_te_maken++;									
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
				 					}
									
							  ?>
								</td>
							</tr>
						</table>
					</form>
				
				<?php
				
			} // er is geen melding geselecteerd, 
			else {
				//dus alle meldingen van dat component tonen
				if(isset($_GET['c']) && $_GET['c'] != 0) {
					//meldingen laten zien
					//gegevens over het geselecteerde component ophalen, zoals de naam van het component en de laatste opgeslagen melding (einde van de keten)
					$query = "SELECT Laatste_Melding, Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
					
					//melding richting de gebruiker
					echo("U heeft \"". $data[1] ."\" geselecteerd.<br>Voor dit component staan de volgende meldingen in het systeem:<br><br>");
					
					//het einde van de keten selecteren, hierna terug werken naar het begin
					$query = "SELECT Meld_Lijst_ID, Meld_Datum, Prob_Beschrijving, Voorgaande_Melding FROM melding_lijst WHERE Meld_Lijst_ID ='". $data[0] ."'";
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
					
					//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
					$gedeeldveld=split(" ",$data['Meld_Datum']);
					//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
					$datum = split("-",$gedeeldveld[0]);

					//tabel aanmaken om de gegevens in te tonen
					echo("<table border =\"1\">");
					echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" . substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$_GET['c']. "&m=". $data['Meld_Lijst_ID']. "\">Bewerken</a></td></tr>");
			
					//terugwerken richting het begin van de meldingenketen
					while ($data['Voorgaande_Melding'] != 1) { 
						$query = "SELECT Meld_Lijst_ID, Meld_Datum, Prob_Beschrijving, Voorgaande_Melding, Meld_Type_ID FROM melding_lijst WHERE Meld_Lijst_ID ='". $data['Voorgaande_Melding'] ."'";
						$resultaat = mysql_query($query);
						$data = mysql_fetch_array($resultaat);
			
						//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
						$gedeeldveld=split(" ",$data['Meld_Datum']);
						//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
						$datum = split("-",$gedeeldveld[0]);
					
						//tonen gegevens
						echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" .substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$_GET['c']. "&m=". $data['Meld_Lijst_ID']. "&q=" . $data['Meld_Type_ID'] .  "\">Bewerken</a></td></tr>");
					}
					//afsluiten tabel
					echo("</table>");
				}
				//alle meldingen van dit type laten zien
				else if (isset($_GET['b']) && $_GET['b'] != 0) {
					//meldingen laten zien
					//gegevens over het geselecteerde type melding ophalen, zoals de naam van de melding 
					$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID = '". $_GET['b'] ."'";
					$resultaat = mysql_query($query);
					$data = mysql_fetch_array($resultaat);
					
					//melding richting de gebruiker
					echo("U heeft \"". $data[0] ."\" geselecteerd.<br>Voor dit type melding staan de volgende meldingen in het systeem:<br><br>");
					
					//het einde van de keten selecteren, hierna terug werken naar het begin
					$query = "SELECT Meld_Lijst_ID, Meld_Datum, Prob_Beschrijving, Comp_Lijst_ID FROM melding_lijst WHERE Meld_Type_ID ='". $_GET['b'] ."'";
					$resultaat = mysql_query($query);

					//tabel aanmaken om de gegevens in te tonen
					echo("<table border =\"1\">");
			
					//terugwerken richting het begin van de meldingenketen
					while ($data = mysql_fetch_array($resultaat)) { 
			
						//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
						$gedeeldveld=split(" ",$data['Meld_Datum']);
						//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
						$datum = split("-",$gedeeldveld[0]);
					
						//tonen gegevens
						echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" .substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$data['Comp_Lijst_ID']. "&m=". $data['Meld_Lijst_ID']. "&b=" . $_GET['b'] .  "\">Bewerken</a></td></tr>");
					}
					//afsluiten tabel
					echo("</table>");
				}
			}
		}
		//er is geen component geselecteerd
		else echo('Selecteer hiernaast een component om een melding van te bewerken.');
	}

?>