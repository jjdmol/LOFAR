<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 2;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=1';
		
		if (isset($_GET['o'])) {
			$_SESSION['type_overzicht'] = $_GET['o'];
		} else if (!isset($_SESSION['type_overzicht'])) $_SESSION['type_overzicht'] = 1;

	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<div id="boom_knoppen_container">
		  		<div id="boom_schakel_knop">
		  			<?php 
		  				if ($_SESSION['type_overzicht'] == '2')
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=1\">Geef comp. overzicht weer</a>");
							else
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=2\">Geef type overzicht weer</a>");
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
	    	
	    	<h2>Componenten overzicht</h2>
	    
				<?php

					//er is een component gekozen, dus gegevens weergeven
					if (isset($_GET['c'])) {
						//kijken of er een bypass actie uitgevoerd moet worden
						//dit vanwege het aanroepen van gegevens uit een popup oid
						//is dit niet t geval dan gewoon het type overzicht gebruiken
						if (isset($_GET['bypass'])) $actie = $_GET['bypass'];
						else $actie = $_SESSION['type_overzicht'];
						//is er een instantie van een type gekozen
						if ($actie == 1) {
							$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID='". $_GET['c'] ."'";
						  $resultaat = mysql_query($query);
							$data = mysql_fetch_array($resultaat);
							echo("<h3>".$data['Comp_Naam']."</h3>");
							echo("<table border=\"0\">");
							echo("<tr><td>Status component:</td><td>");
							$query = "SELECT Status FROM status WHERE Status_ID IN (SELECT Huidige_Status FROM melding_lijst WHERE Meld_Lijst_ID ='".$data['Laatste_Melding']."')";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row[0] . "</td></tr>");
							
							echo("<tr><td>Type component:</td><td>");
							$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type ='".$data['Comp_Type_ID']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Type_Naam'] ."</td><td><a href=\"".$_SESSION['huidige_pagina']."&o=2&c=". $data['Comp_Type_ID']."\">Meer info</a></td></tr>");
							
							echo("<tr><td>Locatie component:</td><td>");
							$query = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID ='".$data['Comp_Locatie']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Loc_Naam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/locatie.php?c=".$data['Comp_Locatie']."\" target=\"_blank\">Meer info</a></td></tr>");
							
							echo("<tr><td>Verantwoordelijke:</td><td>");
							$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Comp_Verantwoordelijke']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['inlognaam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/gebruiker.php?c=".$data['Comp_Verantwoordelijke']."\" target=\"_blank\">Meer info</a></td></tr>");

							echo("<tr><td>Fabricant:</td><td>");
							$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Contact_Fabricant']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Contact_Naam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Contact_Fabricant']."\" target=\"_blank\">Meer info</a></td></tr>");

							//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
							$gedeeldveld=split(" ",$data['Fabricatie_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);
							
							echo("<tr><td>Fabricatiedatum:</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
							echo("<tr><td>Leverancier:</td><td>");
							$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Contact_Leverancier']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Contact_Naam']  ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Contact_Leverancier']."\" target=\"_blank\">Meer info</a></td></tr>");

							//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
							$gedeeldveld=split(" ",$data['Lever_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);

							echo("<tr><td>Leverdatum:</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
							echo("</table>");

							$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							if ($row[0] != 0){
			   				echo("<br>Meldingen historie van dit component:<br>");
			   				echo("<iframe id=\"frame_overzicht\" name=\"frame_overzicht\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."algemene_functionaliteit/melding_historie.php?c=".$_GET['c']."\" width=\"450\" height=\"175\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");	
			   			}
							else 
		   					echo("<br>Er zijn bij dit component geen meldingen gevonden.<br>");
						
						}
						//of is er een type gekozen
						else if ($actie == 2) {
							$query = "SELECT * FROM comp_type WHERE Comp_Type='". $_GET['c'] ."'";
						  $resultaat = mysql_query($query);
							$data = mysql_fetch_array($resultaat);

							echo("<h3>".$data['Type_Naam']."</h3>");

							echo("<table border=\"0\">");
							echo("<tr><td>Aangemaakt door:</td><td>");
							$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Aangemaakt_Door']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['inlognaam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/gebruiker.php?c=".$data['Aangemaakt_Door']."\" target=\"_blank\">Meer info</a></td></tr>");

							//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
							$gedeeldveld=split(" ",$data['Aanmaak_Datum']);
							//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
							$datum = split("-",$gedeeldveld[0]);

							echo("<tr><td>Aangemaakt op:</td><td>".$datum[2] ."-". $datum[1] ."-". $datum[0]. " (".$gedeeldveld[1].")</td><td>&nbsp</td></tr>");
							echo("<tr><td>Structuur entry:</td><td>");
							if ($data['Structuur_Entry'] == 1) echo("Ja");
							else echo("Nee");
							echo("</td><td>&nbsp</td></tr>");

							echo("<tr><td>Fabricant:</td><td>");
							$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Gefabriceerd_Door']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Contact_Naam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Gefabriceerd_Door']."\" target=\"_blank\">Meer info</a></td></tr>");

							echo("<tr><td>Leverancier:</td><td>");
							$query = "SELECT Contact_Naam FROM contact WHERE Contact_ID ='".$data['Geleverd_Door']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['Contact_Naam']  ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/contact.php?c=".$data['Geleverd_Door']."\" target=\"_blank\">Meer info</a></td></tr>");

							echo("<tr><td>Minimum aantal:</td><td>".$data['Min_Aantal']."</td><td>&nbsp</td></tr>");
							echo("<tr><td>Momenteel aangemaakt:</td><td>");
							$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Type_ID = '". $data['Comp_Type'] ."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row[0] ."</td></tr>");
							echo("<tr><td>Maximum aantal:</td><td>".$data['Max_Aantal']."</td><td>&nbsp</td></tr>");
							echo("<tr><td>Reserve minimum:</td><td>".$data['Reserve_Minimum']."</td><td>&nbsp</td></tr>");
							echo("<tr><td>Type verantwoordelijke:</td><td>");
							$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID ='".$data['Type_Verantwoordelijke']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							echo($row['inlognaam'] ."</td><td><a href=\"".$_SESSION['pagina'] ."algemene_functionaliteit/gebruiker.php?c=".$data['Type_Verantwoordelijke']."\" target=\"_blank\">Meer info</a></td></tr>");
							echo("</table>");
							
							
							$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_Type_ID = '".$_GET['c']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							if ($row[0] != 0){
			   				echo("<br>Aangemaakte componenten van dit type:<br>");
			   				echo("<iframe id=\"frame_overzicht\" name=\"frame_overzicht\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."algemene_functionaliteit/componenten_per_type.php?c=".$_GET['c']."\" width=\"450\" height=\"130\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
			   			}
							else 
		   					echo("<br>Er bestaan geen instanties van dit type.<br>");
						}
					}
					else {					
//						$_SESSION['laatste_inlog'] = '2007-01-12 09:00:00'; //<----- CHEATZ!!!!!!
						
						$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Laatste_Melding in";
						$query = $query . "(SELECT Meld_Lijst_ID FROM melding_lijst WHERE Meld_Datum > ";
						$query = $query . "'".$_SESSION['laatste_inlog']."' AND Voorgaande_Melding = 1)";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);

						if ($row[0] != 0) {
							echo("Sinds uw laatste inlog zijn onderstaande componenten aan het systeem toegevoegd:<br>");
	   					echo("<iframe id=\"frame_comp\" name=\"frame_comp\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_componenten/Comp_Overzicht.php\" width=\"450\" height=\"126\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
	   				} 
	   				else
	   					echo("Er zijn geen nieuwe componenten aan het systeem toegevoegd sinds uw laatste inlog.<br>");

						$query = "SELECT Count(Comp_Type) FROM comp_type WHERE Aanmaak_Datum > '".$_SESSION['laatste_inlog']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						if ($row[0] != 0) {
							echo("<br><br>Sinds uw laatste inlog zijn onderstaande componenttypes aan het systeem toegevoegd:<br>");
		   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_componenten/Type_Overzicht.php\" width=\"450\" height=\"126\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
		   			}
		   			else
	   					echo("<br>Er zijn geen nieuwe type componenten aan het systeem toegevoegd sinds uw laatste inlog.<br>");
					}
				?>	    
	    </div>

	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 