<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 1;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=1';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	    <div id="rechterdeel">
	    	<?php
		    	$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID = '". $_SESSION['gebr_id'] ."'";
		    	$result = mysql_query($query);
					$row = mysql_fetch_array($result);
		    	echo("<h2>Welkom ". $row['inlognaam'] ."</h2>");
					
					//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
					$gedeeldveld=split(" ",$_SESSION['laatste_inlog']);
					//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
					$datum = split("-",$gedeeldveld[0]);
		    	echo("Sinds uw laatste keer inloggen (".$datum[2] ."-". $datum[1] ."-". $datum[0]." om ". $gedeeldveld[1] .") zijn de volgende wijzigingen in het systeem opgetreden.<br>");

$_SESSION['laatste_inlog'] = '2007-01-12 09:00:00'; //<----- CHEATZ!!!!!!

					//het reservegedeelte!!
					echo("<b id=\"zwart\">Componenten reserve status:</b><br>");
   				echo("<iframe id=\"frame_reserve\" name=\"frame_reserve\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main/reserve.php\" width=\"700\" height=\"80\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe><br>");

					$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Meld_Datum > '".$_SESSION['laatste_inlog']."'";
				  $res = mysql_query($query);
					$row = mysql_fetch_array($res);
					if ($row[0] != 0) {
						echo("<br><b id=\"zwart\">Toegevoegde meldingen:</b><br>");
	   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_meldingen/Meldingen_Overzicht.php\" width=\"700\" height=\"75\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe><br>");
	   			}
	   			else echo("<b id=\"zwart\">Er zijn sinds uw laatste inlog geen meldingen toegevoegd.</b><br>");

					$query = "SELECT Count(Comp_Lijst_ID) FROM comp_lijst WHERE Laatste_Melding in";
					$query = $query . "(SELECT Meld_Lijst_ID FROM melding_lijst WHERE Meld_Datum > ";
					$query = $query . "'".$_SESSION['laatste_inlog']."' AND Voorgaande_Melding = 1)";
				  $res = mysql_query($query);
					$row = mysql_fetch_array($res);

					if ($row[0] != 0) {
						echo("<br><b id=\"zwart\">Toegevoegde componenten:</b><br>");
   					echo("<iframe id=\"frame_comp\" name=\"frame_comp\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_componenten/Comp_Overzicht.php\" width=\"700\" height=\"75\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe><br>");
   				} 
	   			else echo("<b id=\"zwart\">Er zijn sinds uw laatste inlog geen componenten toegevoegd.</b><br>");   				

					$query = "SELECT Count(Comp_Type) FROM comp_type WHERE Aanmaak_Datum > '".$_SESSION['laatste_inlog']."'";
				  $res = mysql_query($query);
					$row = mysql_fetch_array($res);
					if ($row[0] != 0) {
						echo("<br><b id=\"zwart\">Toegevoegde componenttypes:</b><br>");
	   				echo("<iframe id=\"frame_type\" name=\"frame_type\" align=\"middle\" marginwidth=\"0\" marginheight=\"0\" src=\"". $_SESSION['pagina'] ."main_componenten/Type_Overzicht.php\" width=\"700\" height=\"75\" ALLOWTRANSPARENCY frameborder=\"0\" scrolling=\"auto\"></iframe><br>");
	   			}
	   			else echo("<b id=\"zwart\">Er zijn sinds uw laatste inlog geen componenttypes toegevoegd.</b><br>");
    	
	    	?>
	    
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
