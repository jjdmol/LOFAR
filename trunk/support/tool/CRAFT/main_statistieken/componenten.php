<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=2';
	  $_SESSION['type_overzicht'] = 2;

	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	  include("status_object.php");
		date_default_timezone_set ("Europe/Amsterdam");

	  $Objecten_Array = array();

		function zet_timestamp_om($timestamp) {
			//365 (31536000)
			if ($timestamp > 31536000) 
				return (round($timestamp / 31536000 ,2) . " jaren");
			//24 (86400)
			else if ($timestamp > 86400)
				return (round($timestamp / 86400 ,2) . " dagen");
			//60 (3600)
			else if ($timestamp > 3600)
				return (round($timestamp / 3600 ,2) . " uren");
			//60
			else if ($timestamp > 60)
				return (round($timestamp / 60 ,2) . " minuten");		
			else if ($timestamp == 0)
				return ("0 minuten");
		}


		function zet_datum_om($datum_tijd) {
			//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			$gedeeldveld=split(" ",$datum_tijd);
			//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
			$datum = split("-",$gedeeldveld[0]);
			//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
			$tijd = split(":",$gedeeldveld[1]);
			$test = mktime($tijd[0], $tijd[1], $tijd[2], $datum[1], $datum[2], $datum[0], -1);
			return $test;
		}


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

	    	<h2>Componenten</h2>

				<?php
//				De totale tijd van een component in een bepaalde status<br>
//				meldingen van componenten onder dit component<br>

					if (isset($_GET['c'])) {
						$query = "SELECT Comp_Naam, Laatste_Melding FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$data = mysql_fetch_array($res);
						
						echo("U heeft \"". $data['Comp_Naam'] ."\" geselecteerd.");
						echo("<table>");
						echo("<tr>");
						echo("<td>Aantal meldingen bij dit component:</td><td>");

						$query = "SELECT Count(Meld_Type_ID) FROM melding_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						echo($row[0] ."</td><tr>");
						
						//array van statusobjecten worden: 0 waardes voor de tijd
						$query = "SELECT * FROM Status";
					  $res = mysql_query($query);
				  	while ($row = mysql_fetch_array($res)) {
				  		$Stat_Obj = new Status_Object();
				  		$Stat_Obj->Set_ID($row['Status_ID'],$row['Status'], 0, 0);
				 	  	array_push($Objecten_Array, $Stat_Obj);
  				  	$Stat_Obj = NULL;
				  	}
						//datum nu
						$eind = date('U');
						//daarna alle meldingen langs om tijden bij elkaar op te tellen. afhankelijk van de status ID
						$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '".$data['Laatste_Melding']."'";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						$begin = zet_datum_om($row['Meld_Datum']);
						$verschil = $eind - $begin;

						for ($i=0; $i < count($Objecten_Array); $i++) {
							if ($Objecten_Array[$i]->Get_ID() == $row['Huidige_Status']) {
								$Objecten_Array[$i]->Add_TotaalTijd($verschil);
								$Objecten_Array[$i]->Add_Aantal(1);
							}
						}
						$eind = zet_datum_om($row['Meld_Datum']);

				  	while ($row['Voorgaande_Melding'] != 1) {
				  		//status
							$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID = '".$row['Voorgaande_Melding']."'";
						  $res = mysql_query($query);
							$row = mysql_fetch_array($res);

							$begin = zet_datum_om($row['Meld_Datum']);
							$verschil = $eind - $begin;
							for ($i=0; $i < count($Objecten_Array); $i++) {
								if ($Objecten_Array[$i]->Get_ID() == $row['Huidige_Status']) {
									$Objecten_Array[$i]->Add_TotaalTijd($verschil);
 									$Objecten_Array[$i]->Add_Aantal(1);
								}
							}
							$eind = zet_datum_om($row['Meld_Datum']);
				  	}

						for ($i=0; $i < count($Objecten_Array); $i++) {
							echo("<tr><td>" . $Objecten_Array[$i]->Get_Naam() . "</td><td>". zet_timestamp_om($Objecten_Array[$i]->Get_TotaalTijd()) ."</td><td>(". $Objecten_Array[$i]->Get_Aantal() ." meldingen)</td></tr>");
						}
						
						echo("</table>");
						
					}
					else { echo("Er is geen component geselecteerd!<br>Selecteer een component om hier de gegevens van te zien.");}
				?>

	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
