<?php
	if (isset($_SESSION['admin_deel']))	{
		$_SESSION['admin_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=3';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		
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
	    	<h2>Meldingen verwijderen</h2>
	    	<?php
	    	
	    		
	    		//verwijder controle
	    		if(isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
						$query1 = "UPDATE comp_lijst SET Laatste_Melding='". $_POST['laatste'] ."' WHERE Comp_Lijst_ID = '".$_GET['c']."'";
	
						//extra velden verwijzingen verwijderen!!!
						//eerst kijken of er extra velden verwijzingen zijn.
						$query = "SELECT * FROM melding_koppel_extra WHERE Meld_Lijst_ID = '".$_POST['melding']."'";
						$num_rows = mysql_num_rows(mysql_query($query));
						$extra_velden = array();
						if($num_rows > 0) {
							//door de extra velden itereren en de ID's van de extra velden opslaan
							$resultaat = mysql_query($query);
					  	while ($data = mysql_fetch_array($resultaat)) {
		  		 	  	array_push($extra_velden, $data['Kolom_ID']);

					  	}
					  	//De koppeling tussen de melding en het extra veld verwijderen
					  	$query = "DELETE FROM melding_koppel_extra WHERE Meld_Lijst_ID = '".$_POST['melding']."'";
							if (mysql_query($query)) {
								$datatabel = array();
								//door de extra velden itereren om de datatabel verwijzing op te slaan en het extra veld  te verwijderen
								for($i = 0; $i < Count($extra_velden); $i++) {
									$query = "SELECT Data_Kolom_ID FROM extra_velden WHERE Kolom_ID = '".$extra_velden[$i]."'";
									$resultaat = mysql_query($query);
							  	$data = mysql_fetch_array($resultaat);
			  		 	  	array_push($datatabel, $data['Data_Kolom_ID']);
			  		 	  	
			  		 	  	$query = "DELETE FROM extra_velden WHERE Kolom_ID = '".$extra_velden[$i]."'";
									mysql_query($query);
								}
								
								//de entries in de datatabel verwijderen
								for($i = 0; $i < Count($datatabel); $i++) {			  		 	  	
			  		 	  	$query = "DELETE FROM datatabel WHERE Data_Kolom_ID = '".$datatabel[$i]."'";
									mysql_query($query);
								}								
							}
						}
	
						$query2 = "DELETE FROM melding_lijst WHERE Meld_Lijst_ID = " . $_POST['melding'];
	
						if (mysql_query($query1) && mysql_query($query2)) echo("De door u geselecteerde melding is uit het systeem verwijderd.<br>");
						else("Er is iets mis gegaan met het verwijderen van de geselecteerde melding!! De melding is niet verwijderd!");
						echo('<br><a href="'.$_SESSION['huidige_pagina']. '&c='.$_GET['c']. '">Klik hier om het meldingenoverzicht van dit component te zien.</a>');
						echo('<br><a href="'.$_SESSION['huidige_pagina']. '">Klik hier om terug te keren naar het begin van het verwijderenscherm.</a>');
						echo('<br>of selecteer links een component uit de treeview.');    		
	    		}
	    		else {
	    			//een component om de meldingen van te tonen is geselecteerd
	    			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
				    	
				    	?>
				    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
							<?php
							
							//gegevens over het geselecteerde component ophalen, zoals de naam van het component en de laatste opgeslagen melding (einde van de keten)
							$query = "SELECT Laatste_Melding, Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
							$resultaat = mysql_query($query);
							$data = mysql_fetch_array($resultaat);
							
							//melding richting de gebruiker
							echo("U heeft \"". $data['Comp_Naam'] ."\" geselecteerd.<br>");
							
							if ($data['Laatste_Melding'] > '1') {
								echo("Voor dit component staan de volgende meldingen in het systeem:<br><br>");
												
								//het einde van de keten selecteren, hierna terug werken naar het begin
								$query = "SELECT Meld_Lijst_ID, Meld_Datum, Prob_Beschrijving, Voorgaande_Melding FROM melding_lijst WHERE Meld_Lijst_ID ='". $data['Laatste_Melding'] ."'";
								$resultaat = mysql_query($query);
								$data = mysql_fetch_array($resultaat);
								
								//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
								$gedeeldveld=split(" ",$data['Meld_Datum']);
								//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
								$datum = split("-",$gedeeldveld[0]);
		
								//tabel aanmaken om de gegevens in te tonen
								echo("<table border=\"1\">\n");
								echo("<tr><td><input type=\"checkbox\" name=\"cVerwijderen\" CHECKED></td><td>" . substr($data['Prob_Beschrijving'], 0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['pagina']."algemene_functionaliteit/melding_info.php?c=".$data['Meld_Lijst_ID']."\" target=\"_blank\">Bekijken</a></td></tr>\n");
								$melding = $data['Meld_Lijst_ID'];
								$laatste = $data['Voorgaande_Melding'];
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
									echo("<tr><td>&nbsp</td><td>" .substr($data['Prob_Beschrijving'],0, 40) . "...</td><td>". $datum[2] ."-". $datum[1] ."-". $datum[0]. "</td><td><a href=\"".$_SESSION['pagina']."algemene_functionaliteit/melding_info.php?c=".$data['Meld_Lijst_ID']."\" target=\"_blank\">Bekijken</a></td></tr>\n");
								}
								//afsluiten tabel
								echo("</table>");
							
	
		    				//confirmatie gedoe
								?>    				
					    		<table>
					    			<tr><td><input type="hidden" name="laatste" value="<?php echo($laatste); ?>"><input type="hidden" name="melding" value="<?php echo($melding);?>">Weet u zeker dat u de geselecteerde melding verwijderen wilt?</td></tr>
					    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil de geselecteerde melding verwijderen</td></tr>
					    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
					    		</table>
					    	</form>
		    				<?php
		    			}
		    			else echo("Voor dit component staan geen meldingen in het systeem.<br>");


		    		}
	    			//er is geen component geselecteerd om de meldingen van te tonen
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