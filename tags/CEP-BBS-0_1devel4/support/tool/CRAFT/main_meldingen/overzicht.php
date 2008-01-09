<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 3;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=1';
	  
		if (isset($_GET['b'])) {
			$_SESSION['type_overzicht'] = 2;
		}
		else if(isset($_GET['c'])) {
			$_SESSION['type_overzicht'] = 1;
		}
		else if(isset($_GET['o'])) {
			$_SESSION['type_overzicht'] = $_GET['o'];
		} else if (!isset($_SESSION['type_overzicht'])) $_SESSION['type_overzicht'] = 2;
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<div id="boom_knoppen_container">
		  		<div id="boom_schakel_knop">
		  			<?php
		  				if ($_SESSION['type_overzicht'] == '1')
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=2\">Geef type meldingen weer</a>");
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

	    	<h2>Meldingen overzicht</h2>
				<?php
					
					//er is iets gekozen, dus gegevens weergeven
					if (isset($_GET['c']) || isset($_GET['b'])) {
						//bepalen wat er precies weergegeven moet worden
						//meldingen per component
						if (isset($_GET['c'])) {  //($_SESSION['type_overzicht'] == 1) {
							
							$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
						  $res = mysql_query($query);
							$data = mysql_fetch_array($res);
							echo("<h3>". $data['Comp_Naam']."</h3>");

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
							echo($row['Type_Naam'] ."</td><td><a href=\"".$_SESSION['pagina']."algemene_functionaliteit/comp_type.php?c=". $data['Comp_Type_ID']."\" target=\"_blank\">Meer info</a></td></tr>");
							
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
			   				echo("<br>Meldingen behorende bij dit component:<br>");
			   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."algemene_functionaliteit/melding_historie.php?c=".$_GET['c']."\" width=\"450\" height=\"175\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
			   			}
							else 
		   					echo("<br>Er zijn bij dit component geen meldingen gevonden.<br>");
						}
						//melding per type melding
						else if (isset($_GET['b'])) {//($_SESSION['type_overzicht'] == 2) {
							$query = "SELECT * FROM melding_type WHERE Meld_Type_ID = '".$_GET['b']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							
							echo("<h3>". $row['Melding_Type_Naam']."</h3>");

							echo("<table>");
							echo("<tr><td>Standaard beschrijving:</td><td>".$row['Stand_Beschrijving']."</td></tr>");
							echo("<tr><td>Standaard oplossing:</td><td>".$row['Stand_Oplossing']."</td></tr>");
							echo("<tr><td>Toekennende status:</td><td>");

							$query = "SELECT Status FROM status WHERE Status_ID = '".$row['Huidige_Status']."'";
						  $rest = mysql_query($query);
							$data = mysql_fetch_array($rest);
							
							echo($data['Status']."</td></tr>");
							echo("<tr><td>Algemene melding:</td><td>");
							if ($row['Algemene_Melding'] == 1) echo("Ja");
							else echo("Nee");
							echo("</td></tr>");
							echo("</table>");
							
							$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Meld_Type_ID = '".$_GET['b']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);
							if ($row[0] != 0){
			   				echo("<br>Meldingen van dit type:<br>");
			   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_meldingen/meldingen_per_type.php?c=".$_GET['b']."\" width=\"650\" height=\"250\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
			   			}
							else 
		   					echo("<br>Er zijn geen meldingen van dit type gevonden.<br>");
						}
					}
					//nieuwste meldingen per laatste inlog
					else {
//						$_SESSION['laatste_inlog'] = '2007-01-12 09:00:00'; //<----- CHEATZ!!!!!!

						$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Meld_Datum > '".$_SESSION['laatste_inlog']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						if ($row[0] != 0) {
							echo("Sinds uw laatste inlog zijn onderstaande meldingen aan het systeem toegevoegd:<br>");
		   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_meldingen/Meldingen_Overzicht.php\" width=\"650\" height=\"250\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe>");
		   			}
		   			else
	   					echo("Er zijn geen nieuwe meldingen aan het systeem toegevoegd sinds uw laatste inlog.<br>");
					}
				?>
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>