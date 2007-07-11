  <?php
	
	$_SESSION['admin_deel'] = 4;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
  
  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
	
  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {
  	
  	?>
  	<div id="linkerdeel">
  		<?php 
  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_meldingen/melding_functies.php\"></script>");
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
    	<h2>Meldingen toevoegen</h2>

			<?php

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
					if (isset($_POST['hidden_beschrijving'])) {
						if ($_POST['hidden_beschrijving'] == '')
							return false;
					} else return false;

					return true;
				}


				if (Valideer_Invoer()) {
					//uit de componenten lijst halen welke melding hier als laatste bij opgeslagen is
					//deze waarde is nodig om een keten van meldingen te kunnen vormen
					$query = "SELECT Laatste_Melding FROM comp_lijst WHERE Comp_Lijst_ID = '". $_GET['c'] ."'";
					$resultaat = mysql_query($query);
			  	$row = mysql_fetch_array($resultaat);
					
					//de query om de melding toe te voegen, samenstellen
					$query = "INSERT INTO melding_lijst (Meld_Type_ID, Comp_Lijst_ID, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Prob_Oplossing, Behandeld_Door, Gemeld_Door, Afgehandeld)";
					$query = $query . "VALUES ('". $_POST['Type_Melding'] ."', '". $_GET['c'] ."'";

  				//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
  				if (isset($_POST['Meld_Datum']) && $_POST['Meld_Datum'] != '') {
    				$datum=split("-",$_POST['Meld_Datum']);
	  				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['Meld_Tijd'] .":00'";							
					}
					else $query = $query . ", NOW()";
					$query = $query . ", '". $_POST['hidden_status'] ."', '". $row[0] ."', '". $_POST['hidden_beschrijving'] ."', '". $_POST['hidden_oplossing'];
					$query = $query . "', '". $_POST['Behandeld_Door'] ."', '". $_POST['Gemeld_Door'] ."', '";
					//de afgehandeld checkbox vertalen naar sql taal ;)
					if (isset($_POST['afgehandeld']) && ($_POST['afgehandeld'] == 'on' || $_POST['afgehandeld'] == '1'))
						$query = $query . "1') ";
					else $query = $query . "0') ";

					//uitvoeren van de insert query
					mysql_query($query);
					//de id van de zojuist toegevoegde melding halen
					$Laatste_Melding = mysql_insert_id();
					//het component waar deze melding bijhoort bijwerken, zodat deze weet dat er een nieuwe laatste_melding is (einde van de keten)
					$query = "UPDATE comp_lijst SET Laatste_Melding='". $Laatste_Melding ."' WHERE Comp_Lijst_ID='". $_GET['c'] ."'";
					mysql_query($query);
					
					//meldingen voor de gebruiker
					echo("De nieuwe melding (". $Laatste_Melding .") is aan het systeem toegevoegd!<br>");
   				echo('<a href="'.$_SESSION['huidige_pagina']. '&c=' . $_GET['c'] . '">Klik hier om nog een melding aan dit component toe te voegen of geselecteer een component uit de treeview.</a>');
				}
				else {
					if (isset($_GET['c']) && $_GET['c'] != 0 ) {
						//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
						date_default_timezone_set ("Europe/Amsterdam");
			?>

			    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
			    		<table>
			    			<tr>
			    				<td>Type melding:</td>
			    				<td><select name="Type_Melding" onChange="switchMelding();">
			    					<?php
			    						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
					    			  $resultaat = mysql_query($query);
			
								  		if (isset($_POST['Type_Melding'])) $type = $_POST['Type_Melding'];
								  		else $type = 'SELECTED';
			
									  	while ($data = mysql_fetch_array($resultaat)) {
					  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
						  	  			if ($data['Meld_Type_ID'] == $type || $type == 'SELECTED') {
						  	  				echo('SELECTED');
						  	  				$type = $data['Meld_Type_ID'];
						  	  			}
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
								  		else $selectie = 'SELECTED';
			
									  	while ($data = mysql_fetch_array($resultaat)) {
					  	  				echo('<option value="'.$data['Werknem_ID'].'"');
						  	  			if ($data['Werknem_ID'] == $selectie || $selectie == 'SELECTED') {
						  	  				echo('SELECTED');
						  	  				$selectie = $data['Werknem_ID'];
						  	  			}
						  	  			echo('>'. $data['inlognaam'] .'</option>');
											}
			    					?>
			    				</select></td>
			    			</tr>
			    			<tr>
			    				<td>Melddatum:</td>
			    				<td><input name="Meld_Datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum'])) echo($_POST['Meld_Datum']); else echo(date('d-m-Y'));?>">
			    					  <input name="Meld_Tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd'])) echo($_POST['Meld_Tijd']); else echo(date('H:i'));?>">
	  	    					  <?php if(isset($_POST['Meld_Datum']) && (!Valideer_Datum($_POST['Meld_Datum']) || !Valideer_Tijd($_POST['Meld_Tijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
			    				
			    				</td>
			    			</tr>
			    			<tr>
			    				<td>Probleem beschrijving:</td>
			    				<td><iframe id="frame_beschrijving" name="frame_beschrijving" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_meldingen/probleem_beschrijving.php<?php if(isset($type)) echo("?c=".$type); ?>" width="305" height="72" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
			    					<?php if(isset($_POST['opslaan']) && ($_POST['opslaan'] == 1) && ($_POST['hidden_beschrijving'] == '')) echo('<b>* Er is geen beschrijving ingevoerd!</b>'); ?></td>
			    			</tr>
			    			<tr>
			    				<td>Probleem oplossing:</td>
			    				<td><iframe id="frame_oplossing" name="frame_oplossing" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_meldingen/probleem_oplossing.php<?php if(isset($type)) echo("?c=".$type); ?>" width="305" height="72" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
			    			</tr>
			    			<tr>
			    				<td>Behandeld door:</td>
			    				<td><select name="Behandeld_Door">
			    					<?php 
											$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
					    			  $resultaat = mysql_query($query);
			
								  		if (isset($_POST['Behandeld_Door'])) $selectie = $_POST['Behandeld_Door'];
								  		else $selectie = 'SELECTED';
			
									  	while ($data = mysql_fetch_array($resultaat)) {
					  	  				echo('<option value="'.$data['Werknem_ID'].'"');
						  	  			if ($data['Werknem_ID'] == $selectie || $selectie == 'SELECTED') {
						  	  				echo('SELECTED');
						  	  				$selectie = $data['Werknem_ID'];
						  	  			}
						  	  			echo('>'. $data['inlognaam'] .'</option>');
											}
			    					?>	
			   					</select></td>
			    			</tr>
			    			<tr>
			    				<td>Afgehandeld:</td>
			    				<td><input name="afgehandeld" type="checkbox"
			    				<?php
				    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
				    					if(isset($_POST['afgehandeld']) && ($_POST['afgehandeld'] == 1 || $_POST['afgehandeld'] == 'on')) 
				    					echo('CHECKED');
		    						}
		    						?>>
		    					</td>
			    			</tr>
			    			<tr>
			    				<td>Historie:</td>
			    				<td><iframe id="frame_historie" name="frame_historie" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_meldingen/historie.php <?php if(isset($_GET['c'])) echo("?c=".$_GET['c']); ?>" width="425" height="105" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
			    			</tr>
			    			<tr>
			    				<td>
			    					<input name="hidden_status" id="hidden_status" type="hidden" value="">
			    					<input name="hidden_beschrijving" id="hidden_beschrijving" type="hidden" value="">
			    					<input name="hidden_oplossing" id="hidden_oplossing" type="hidden" value="">
			    					<input name="opslaan" type="hidden" value="1">
			    				</td>
			    				<td><a href="javascript:SubmitMeldingToevoegen();">Toevoegen</a></td>
			    			</tr>
			    		</table>
			    	</form>
				<?php
						}
						else echo('Selecteer hiernaast een component om een melding aan toe te voegen.');
					}
				?>

	  </div>

<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?> 