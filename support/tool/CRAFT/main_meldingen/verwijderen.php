<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 3;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=4';
	  
		if (isset($_GET['o'])) {
			$_SESSION['type_overzicht'] = $_GET['o'];
		} else if (!isset($_SESSION['type_overzicht'])) $_SESSION['type_overzicht'] = 2;
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<div id="boom_knoppen_container">
		  		<div id="boom_schakel_knop">
		  			<?php
		  				if ($_SESSION['type_overzicht'] == '1')
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=2\">Geef type overzicht weer</a>");
							else
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=1\">Geef comp. overzicht weer</a>");
		  			?>
					</div>
	  		</div>
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

	    	<h2>Meldingen verwijderen</h2>
	    	<?php
					//er is een component geselecteerd
					if (isset($_GET['c']) && $_GET['c'] != 0 || isset($_GET['b']) && $_GET['b'] != 0) {
			
						
						//er is een melding geselecteerd, dus de gegevens van die melding weergeven
						if (isset($_GET['m']) && $_GET['m'] != 0) {
		    			$query = "SELECT Comp_Lijst_ID, Meld_Type_ID, Prob_Beschrijving FROM melding_lijst WHERE Meld_Lijst_ID='". $_GET['m'] ."'";
					  	$resultaat = mysql_query($query);  	
					  	$data = mysql_fetch_array($resultaat);
							
		    			$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID = '". $data['Meld_Type_ID'] ."'";
					  	$resultaat = mysql_query($query);  	
					  	$row = mysql_fetch_array($resultaat);

					  	echo("U heeft de melding \"". $row['Melding_Type_Naam'] ."\" (".$_GET['m'].") geselecteerd<br>");
					  	echo("De ingekorte probleem beschrijving luidt als volgt \"". substr($data['Prob_Beschrijving'], 0, 40) ."\".<br>");

		    			$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '". $data['Comp_Lijst_ID'] ."'";
					  	$resultaat = mysql_query($query);  	
					  	$row = mysql_fetch_array($resultaat);
					  	
					  	echo("<br>Om deze melding te verwijderen dient u contact op te nemen met de administrator van uw groep<br>");
					  	echo("Ook kunt u contact opnemen met de algemene administrators<br>");
							echo("<br>De administrator(s), welke deze melding verwijderen kunnen, zijn:<br>");
							echo("<table>");
							//de groepen ophalen, welke toegang hebben tot dit type component
							$Collectie = Check_groepen($row['Comp_Type_ID']);
							for ($i = 0; $i < Count($Collectie); $i++) {
								$query = "SELECT Groep_ID, Admin_Rechten FROM gebruikers_groepen WHERE Groep_ID = '".$Collectie[$i]."'";
							  $resultaat = mysql_query($query);
						  	$row = mysql_fetch_array($resultaat);
								//kijken of de groep adminrechten heeft
								if ($row['Admin_Rechten'] == 1) {
									$query2 = "SELECT * FROM gebruiker WHERE Groep_ID = '".$row['Groep_ID']."'";
								  $rest = mysql_query($query2);
									//data weergeven
									while ($data = mysql_fetch_array($rest)) {
										echo("<tr><td>" . $data['Werknem_ID'] . "</td><td>". $data['inlognaam'] . "</td><td>" . $data['Emailadres'] . "</td></tr>");
									}
								}
							}
							echo("</table>");
							
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
								echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" . substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$_GET['c']. "&m=". $data['Meld_Lijst_ID']. "\">Verwijderen</a></td></tr>");
						
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
									echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" .substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$_GET['c']. "&m=". $data['Meld_Lijst_ID']. "&q=" . $data['Meld_Type_ID'] .  "\">Verwijderen</a></td></tr>");
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
									echo("<tr><td>". $data['Meld_Lijst_ID'] ."</td><td>" .substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['huidige_pagina']."&c=".$data['Comp_Lijst_ID']. "&m=". $data['Meld_Lijst_ID']. "&b=" . $_GET['b'] .  "\">Verwijderen</a></td></tr>");
								}
								//afsluiten tabel
								echo("</table>");
							}
						}
					}
					//er is geen component geselecteerd
					else echo('Selecteer hiernaast een component om een melding van te verwijderen.');
	    	?>

	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
