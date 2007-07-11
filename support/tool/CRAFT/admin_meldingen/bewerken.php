<?php
	if (isset($_SESSION['admin_deel']))	{
		$_SESSION['admin_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree.js\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_items.php\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_tpl.js\"></script>");
	  		?>
				<script language="JavaScript">
				<!--//
		 			new tree (TREE_ITEMS, TREE_TPL);
	   		//-->
				</script> 
			
			</div>
	    <div id="rechterdeel">
	    	<h2>Meldingen bewerken</h2>
		  
		  	<?php
		  		
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
						
						//beschrijving
						if (isset($_POST['Prob_Beschrijving'])) {
							if ($_POST['Prob_Beschrijving'] == '')
								return false;
						} else return false;
	
						return true;
		  		}
		  		
		  		//controleren of er opgeslagen moet worden, of dat er een ander scherm getoond moet worden
		  		if(Valideer_Invoer()) {
						if (isset($_POST['Afgehandeld']) && ($_POST['Afgehandeld'] == 1 || $_POST['Afgehandeld'] == 'on'))
							$afgehandeld = 1;
						else $afgehandeld = 0;
						
						//query samenstellen
						$query = "UPDATE melding_lijst SET Meld_Type_ID='". $_POST['Type_Melding'] ."', Gemeld_Door='". $_POST['Gemeld_Door'] ."', ";
						$query = $query . "Prob_Beschrijving='". htmlentities($_POST['Prob_Beschrijving'], ENT_QUOTES)."', Prob_Oplossing='". htmlentities($_POST['Prob_Oplossing'], ENT_QUOTES) ."', Behandeld_Door='".$_POST['Behandeld_Door']."', ";
						$query = $query . "Afgehandeld='". $afgehandeld ."', Meld_Datum=";
	
	  				//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
	  				if (isset($_POST['Meld_Datum']) && $_POST['Meld_Datum'] != '') {
	    				$datum=split("-",$_POST['Meld_Datum']);
		  				$query = $query . "'". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['Meld_Tijd'] .":00'";
						}
						else $query = $query . "NOW()";
						
						$query = $query . " WHERE Meld_Lijst_ID='".$_GET['m']."'";
						
						if (mysql_query($query)) echo("De gewijzigde melding (". $_GET['m'] .") is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van deze melding (". $_GET['m'] .")!! Deze melding is niet bijgewerkt!");
						echo('<br><a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '&m='.$_GET['m'].'">Klik hier om deze melding opnieuw te bewerken/bewerken.</a>');
						echo('<br><a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het meldingenoverzicht van dit component.</a>');
						echo('<br>of selecteer links een component uit de treeview.');
		  		}
		  		//niet opslaan
		  		else {
		  			//er is een component geselecteerd
		  			if (isset($_GET['c']) && $_GET['c'] != 0) {
			  			//er is een melding geselecteerd, dus de gegevens van die melding weergeven
			  			if (isset($_GET['m']) && $_GET['m'] != 0) {
								$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID='". $_GET['m'] ."'";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_array($resultaat);
								?>
									<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']."&m=".$_GET['m']); ?>">
										<table>
											<tr>
												<td>Type melding:</td>
												<td><select name="Type_Melding">
												<?php
					    						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
							    			  $resultaat = mysql_query($query);
					
										  		if (isset($_POST['Type_Melding'])) $type = $_POST['Type_Melding'];
										  		else $type = $row['Meld_Type_ID'];
					
											  	while ($data = mysql_fetch_array($resultaat)) {
							  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
								  	  			if ($data['Meld_Type_ID'] == $type) 
								  	  				echo('SELECTED');
								  	  			echo('>'. $data['Melding_Type_Naam'] .'</option>');
													}
												?>	
												</select></td>
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
												</select></td>
											</tr>
											<tr>
												<td>Meld datum:</td>
												<td>
													<?php
														//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
							    					$gedeeldveld=split(" ",$row['Meld_Datum']);
														//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
														$datum = split("-",$gedeeldveld[0]);
														//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
														$tijd = split(":",$gedeeldveld[1]);
													?>
							    				<input name="Meld_Datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum'])) echo($_POST['Meld_Datum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
				    					  	<input name="Meld_Tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd'])) echo($_POST['Meld_Tijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
			  	    					  <?php if(isset($_POST['Meld_Datum']) && (!Valideer_Datum($_POST['Meld_Datum']) || !Valideer_Tijd($_POST['Meld_Tijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?>
												</td>
											</tr>
											<tr>
												<td>Probleem beschrijving:</td>
												<td><textarea name="Prob_Beschrijving" rows="4" cols="35"><?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 ) echo(htmlentities($_POST['Prob_Beschrijving'], ENT_QUOTES)); else echo($row['Prob_Beschrijving']); ?></textarea>
													<?php if(isset($_POST['Prob_Beschrijving']) && $_POST['Prob_Beschrijving'] == '') echo('<b>* Er is geen probleem beschrijving ingevoerd!</b>'); ?>
												</td>
											</tr>
											<tr>
												<td>Probleem oplossing:</td>
												<td><textarea name="Prob_Oplossing" rows="4" cols="35"><?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 ) echo(htmlentities($_POST['Prob_Oplossing'], ENT_QUOTES)); else echo($row['Prob_Oplossing']); ?></textarea></td>
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
												</select></td>
											</tr>
											<tr>
												<td>Afgehandeld:</td>
												<td><input name="Afgehandeld" type="checkbox" 
							    				<?php
								    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
								    					if(isset($_POST['Afgehandeld']) && ($_POST['Afgehandeld'] == 1 || $_POST['Afgehandeld'] == 'on')) 
								    					echo('CHECKED');
								    				}
								    				else if ($row['Afgehandeld'] == 1) echo('CHECKED');
													?>>
												</td>
											</tr>
											<tr>
												<td><a href="<?php echo($_SESSION['huidige_pagina']."&c=".$_GET['c']); ?>">Overzicht meldingen</a></td>
												<td><input type="hidden" name="opslaan" value="1"><a href="javascript:document.theForm.submit();">Opslaan</a></td>
											</tr>
										</table>
									</form>
								
								<?php
								
			  			} // er is geen melding geselecteerd, dus alle meldingen van dat component tonen
			  			else {
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
									$query = "SELECT Meld_Lijst_ID, Meld_Datum, Prob_Beschrijving, Voorgaande_Melding FROM melding_lijst WHERE Meld_Lijst_ID ='". $data['Voorgaande_Melding'] ."'";
									$resultaat = mysql_query($query);
									$data = mysql_fetch_array($resultaat);
						
									//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
									$gedeeldveld=split(" ",$data['Meld_Datum']);
									//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
									$datum = split("-",$gedeeldveld[0]);
								
									//tonen gegevens
									echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" .substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$_GET['c']. "&m=". $data['Meld_Lijst_ID']. "\">Bewerken</a></td></tr>");
								}
								//afsluiten tabel
								echo("</table>");
			  			}
		  			}
		  			//er is geen component geselecteerd
		  			else echo('Selecteer hiernaast een component om een melding aan toe te voegen.');
		  		}
		  	
		  	?>
		  
		  </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");
	}
?> 