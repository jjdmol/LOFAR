<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=1';
	    
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		$_SESSION['type_overzicht'] = 1;

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {

			function zet_datum_om($datum_tijd) {
				//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
				$gedeeldveld=split(" ",$datum_tijd);
				//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
				$datum = split("-",$gedeeldveld[0]);
				//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
				$tijd = split(":",$gedeeldveld[1]);
				$test = $datum[2] ."-". $datum[1] ."-". $datum[0] ." ". $tijd[0] .":". $tijd[1];
				return $test;
			}
	  	
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
	    	<h2>Type componenten</h2>
	    	<?php
	    		if (isset($_GET['c'])) {
						$query = "SELECT Type_Naam, Aanmaak_Datum FROM comp_type WHERE Comp_Type = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);
						echo("U heeft \"". $data[0]  ."\" geselecteerd. Dit compontent is aangemaakt op ". zet_datum_om($data[1]));

						$query = "SELECT Count(Comp_Type_ID) FROM comp_lijst";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);

						$query = "SELECT Count(Comp_Type_ID) FROM comp_lijst WHERE Comp_Type_ID = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						echo("<table>");
						echo("<tr><td>Aantal componenten van dit type:</td><td>". $row[0] . "</td><td>(Totale aantal componenten: ". $data[0] .")</td></tr>");
						
						$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst WHERE Comp_Lijst_ID IN (SELECT Comp_Lijst_ID FROM comp_lijst WHERE Comp_Type_ID IN (SELECT comp_type FROM comp_type c WHERE comp_type = '".$_GET['c']."'))";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);

						$query = "SELECT Count(Meld_Lijst_ID) FROM melding_lijst";
					  $rest = mysql_query($query);
						$data = mysql_fetch_array($rest);

						echo("<tr><td>Aantal meldingen bij dit type:</td><td>". $row[0] . "</td><td>(Totale aantal meldingen: ". $data[0] .")</td></tr>");
						
						
						echo("</table>");
	    		}
					else { echo("Er is geen type component geselecteerd!<br>Selecteer een type component!");}
	    	?>
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
