<?php
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=5';

	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
	  
	  if (isset($_POST['datum']) && isset($_POST['tijd']))
	  	$_SESSION['huidige_pagina'] = $_SESSION['huidige_pagina'] . '&d='. Datum_Tijd_Naar_DB_Conversie($_POST['datum'],$_POST['tijd']);
	    

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {

			function controleer_datum($waarde) {
				if (substr_count($waarde, "-") == 2)
					return true;
				else return false;
			}
			
			
			function datum_invoer() {
	    	?>
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
	    		<table>
	    			<tr>
	    				<td>Datum:</td>
							<td>
								<input name="datum" id="datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['datum'])) echo($_POST['datum']); else echo(date('d-m-Y')); ?>">
								<input name="tijd" id="tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['tijd'])) echo($_POST['tijd']); else echo(date('H:i')); ?>">
							</td>
	    			</tr>
	    			<tr>
	    				<td><input name="opslaan" type="hidden" value="1"></td>
	    				<td><a href="javascript:document.theForm.submit();">Reconstrueer</a></td>
	    			</tr>
	    		</table>
	    	</form>
				<?php
			}
			

			
	  	?>
	  	<div id="linkerdeel">
	  		<?php	
					if(isset($_GET['d'])) {
						//datum en tijd
						$_SESSION['type_overzicht'] = $_GET['d'];
					}
					if(isset($_POST['datum'])) {
						//datum en tijd
						$_SESSION['type_overzicht'] = Datum_Tijd_Naar_DB_Conversie($_POST['datum'],$_POST['tijd']);
					}

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

	    	<h2>Historie</h2>
					<?php
						date_default_timezone_set ("Europe/Amsterdam");

						//wanneer er een datum ingevuld is, dan een situatie reconstrueren
						if(controleer_datum($_SESSION['type_overzicht'])) {
							//de samengestelde datum splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
    					$gedeeldveld=split(" ",$_SESSION['type_overzicht']);
    					
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
							$tijd = split(":",$gedeeldveld[1]);
							
							echo("Links ziet u de situatie van ".$datum[2] ."-". $datum[1] ."-". $datum[0]." om ". $tijd[0] .":". $tijd[1]);
							echo("<br>Selecteer links component om de status van dat component te bekijken.");
							echo("<br>of voer hieronder een nieuwe datum in:");
							datum_invoer();
							//wanneer een component geselecteerd is, in de reconstructie modus, dan de gegevens van dit component weergeven
							if (isset($_GET['c'])) {
								$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
		    			  $resultaat = mysql_query($query);
						  	$component = mysql_fetch_array($resultaat);
								echo("U heeft het component \"". $component['Comp_Naam'] . "\" (ID: ".$_GET['c']. ") geselecteerd");

								//laatste melding ophalen
								$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '". $component['Laatste_Melding'] ."'";
		    			  $resultaat = mysql_query($query);
								//datum kan gelijk aan of kleiner zijn dan de te zoeken datum.. 
								//is dit zo, dan heb je de huidige status gevonden 
						  	$melding = "";
						  	$komende_melding = "";
						  	$data = mysql_fetch_array($resultaat);
						  	if ($data['Meld_Datum'] <= $_SESSION['type_overzicht']) {
						  		$melding = $data['Meld_Lijst_ID'];
						  	}
								//zo niet, dan moet de ketting van meldingen doorlopen worden
								//om de huidige status te vinden
						  	else {
							  	$komende_melding = $data['Meld_Lijst_ID'];
						  		
						  		//haal een record op en herhaal dit zolang de datum groter is dan de gezochte datum
						  		while ($data['Meld_Datum'] > $_SESSION['type_overzicht'] && $data['Voorgaande_Melding'] != 1){
										$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '".$data['Voorgaande_Melding']."'";
		  	    			  $resultaat = mysql_query($query);
										$data = mysql_fetch_array($resultaat);
						  		}
						  		$melding = $data['Meld_Lijst_ID'];
						  		
						  	}
								$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '".$melding."'";
  	    			  $resultaat = mysql_query($query);
								$row = mysql_fetch_array($resultaat);

								$query = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID = '". $component['Comp_Locatie'] ."'";
  	    			  $resultaat = mysql_query($query);
								$tijdelijk = mysql_fetch_array($resultaat);
								echo("<br>Dit component stond op locatie: \"". $tijdelijk['Loc_Naam'] . "\"");

								$query = "SELECT Status FROM status WHERE Status_ID = '". $component['Comp_Locatie'] ."'";
  	    			  $resultaat = mysql_query($query);
								$tijdelijk = mysql_fetch_array($resultaat);
								echo("<br>De status van dit component was \"". $tijdelijk['Status']."\"");


								//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
	    					$gedeeldveld=split(" ",$row['Meld_Datum']);
								//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
								$datum = split("-",$gedeeldveld[0]);
								//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
								$tijd = split(":",$gedeeldveld[1]);
								echo("<br>Deze status was vanaf ". $datum[2] ."-". $datum[1] ."-". $datum[0] ." om " . $tijd[0] .":". $tijd[1] . " ingegaan.");
								
								
								//wanneer 1 melding(dus laatste is huidige) dan huidige datum weegeven
								if($component['Laatste_Melding'] == $melding) {
									echo("<br>Het component heeft deze status behouden tot dit moment.");
								}
								else {
									$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '" . $component['Laatste_Melding'] ."'";
  	    			  	$resultaat = mysql_query($query);
									$tijdelijk = mysql_fetch_array($resultaat);
									
									//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
		    					$gedeeldveld=split(" ",$tijdelijk['Meld_Datum']);
									//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
									$datum = split("-",$gedeeldveld[0]);
									//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
									$tijd = split(":",$gedeeldveld[1]);

									echo("<br>Het component heeft deze status behouden tot ". $datum[2] ."-". $datum[1] ."-". $datum[0] ." om " . $tijd[0] .":". $tijd[1] . ".");
									
								}
								//link
								echo("<br><br><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/comp_beschrijving.php?c=".$component['Comp_Lijst_ID']."\" target=\"_blank\">Klik hier voor uitgebreide informatie over dit station</a>");
								
/*						  	while ($data = mysql_fetch_array($resultaat)) {
									//
								}*/
							}
						}
						else {
				    	echo("Vul hieronder de datum en een tijd in van de te reconstrueren situatie:");
				    	datum_invoer();
						}					
					?>
					
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
