<?php
	
	$_SESSION['admin_deel'] = 2;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
  
  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
	
  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {
  	
  	?>
  	<div id="linkerdeel">
  		<?php 
  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_componenten/comp_functies.php\"></script>");
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
    	
    	<h2>Component toevoegen</h2>
    		<?php
    			
    			
    			//het valideren van de invoer, dus controleren of de ingevoerde gegevens opgeslagen mogen worden
    			function Valideer_Invoer() {
						
						//controleren of het component niet het maximum aantal instanties van dit component overtreedt
						if (isset($_POST['hidden_aantal']) && isset($_POST['hidden_maximum']) && $_POST['hidden_aantal'] > $_POST['hidden_maximum'])
							return false;
							
						//controleren of er wel een naam voor dit component ingevoerd is
						if (isset($_POST['hidden_naam'])) {
							if ($_POST['hidden_naam'] == '')
								return false;
						} else return false;

    				//controleren of er wel een type voor dit component geselecteerd is
    				if (isset($_POST['hidden_type']) && $_POST['hidden_type'] =='') 
							return false;

    				//de statusdatum controleren
    				if (isset($_POST['statusdatum'])) {
    					//wanneer de statusdatum gevuld is, dan...
     					if($_POST['statusdatum'] !='') {
     						
     						//controleren op de juiste samenstelling van de statusdatum
	   						if (Valideer_Datum($_POST['statusdatum']) == false)
	   						return false;
    					
	    					//controleren of de tijd correct ingevoerd is
	    					if(isset($_POST['statustijd'])) {
	    					  if (Valideer_Tijd($_POST['statustijd']) == false)
	    					  	return false;
	    					}
	    				}
     				} 
    				
    				//de leverdatum controleren
    				if (isset($_POST['leverdatum'])) {
    					//wanneer de leverdatum gevuld is, dan...
     					if($_POST['leverdatum'] !='') {
     						
     						//controleren op de juiste samenstelling van de leverdatum
	   						if (Valideer_Datum($_POST['leverdatum']) == false)
	   						return false;
    					
	    					//controleren of de tijd correct ingevoerd is
	    					if(isset($_POST['levertijd'])) {
	    					  if (Valideer_Tijd($_POST['levertijd']) == false)
	    					  	return false;
	    					}
	    				}
     				} 

    				//de fabricagedatum controleren
    				if (isset($_POST['fabricagedatum'])) {
    					//wanneer de fabricagedatum ingevuld is, dan... 
    					if($_POST['fabricagedatum'] !='') {

     						//controleren op de juiste samenstelling van de fabricagedatum
		   					if (Valideer_Datum($_POST['fabricagedatum']) == false)
		   						return false;
    						//controleren of de tijd correct ingevoerd is
	    					if(isset($_POST['fabricagetijd'])) {
	    						if (Valideer_Tijd($_POST['fabricagetijd']) == false)
	    					 		return false;
    						}
     					}
     				}
    				//alle controles zijn goed doorlopen
    				return true;
    			}
    			
    			
    			//controleren of er opgeslagen kan worden, of dat de invoervelden getoond moeten worden
    			if (Valideer_Invoer()) {
    				//het eerste gedeelte van de query
    				$query = "INSERT INTO comp_lijst (Comp_Naam, Comp_Type_ID, Comp_Parent, Comp_Locatie, Comp_Verantwoordelijke, Contact_Fabricant, Contact_Leverancier";
    				//als er een leverdatum ingevoerd is, dan dit veld ook toevoegen aan de query
    				if (isset($_POST['leverdatum']) && $_POST['leverdatum'] != '')
    					$query = $query . ", Lever_datum";
    				//als er een fabricagedatum ingevoerd is, dan dit veld ook toevoegen aan de query
    				if (isset($_POST['fabricagedatum']) && $_POST['fabricagedatum'] != '')
    					$query = $query . ", Fabricatie_Datum";
    				
    				//de waardes, welke opgeslagen moeten worden in de database
    				$query = $query . ") VALUES ('". $_POST['hidden_naam'] ."', '". $_POST['comp_type'] ."', '". $_POST['hidden_type'] ."', '";
    				$query = $query .  $_POST['comp_locatie'] ."', '".  $_POST['comp_verantwoordelijke']."', '".$_POST['hidden_fabricant']."', '".$_POST['hidden_leverancier']."'";
    				    				
    				//de waarde voor de leverdatum aan de query toevoegen
    				if (isset($_POST['leverdatum']) && $_POST['leverdatum'] != '') {
	    				$datum=split("-",$_POST['leverdatum']);
  	  				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['levertijd'] .":00'";
  	  			}
    				//de waarde voor de fabricagedatum aan de query toevoegen
    				if (isset($_POST['fabricagedatum']) && $_POST['fabricagedatum'] != '') {
	    				$datum=split("-",$_POST['fabricagedatum']);
	    				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['fabricagetijd'] .":00'";
    				}
    				
    				//de query afsluiten met een haakje
    				$query = $query . ')';
						
						//variabele om bij te houden hoeveel querys goed uitgevoerd zijn
						//dit is bedoeld voor het geven van de juiste foutmelding
						$errorLevel = 0;
						
						//wanneer het toevoegen van het component goed gegaan is, dan een melding toevoegen
						if (mysql_query($query)) {
							$errorLevel = 1;

							//de toegevoegde component_ID opslaan voor later gebruik
							$Comp_ID = mysql_insert_id();

							//De eerste melding van het component toevoegen, dit is zeer waarschijnlijk het plaatsen van het component oid
							$query = "INSERT INTO melding_lijst (Meld_Type_ID, Comp_Lijst_ID, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Behandeld_Door, Gemeld_Door)";
							$query = $query . "VALUES ('". $_POST['type_melding'] ."', '". $Comp_ID ."'";
		
		  				//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
		  				if (isset($_POST['statusdatum']) && $_POST['statusdatum'] != '') {
		    				$datum=split("-",$_POST['statusdatum']);
			  				$query = $query . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['statustijd'] .":00'";							
							}
							else $query = $query . ", NOW()";
							$query = $query . ", '". $_POST['hidden_status'] ."', '1', '". $_POST['hidden_melding'] ."', '". $_SESSION['gebr_id'] ."', '". $_SESSION['gebr_id'] ."') ";
							
							//de melding is goed toegevoegd, dus nu een verwijzing naar de laatste melding bij het component voegen
							if (mysql_query($query)) {
								$errorLevel = 2;
								//de ID van de toegevoegde melding ophalen
								$Meld_ID = mysql_insert_id();
								
								$query = "UPDATE comp_lijst SET Laatste_Melding = '". $Meld_ID ."' WHERE Comp_Lijst_ID='". $Comp_ID ."'";
								if (mysql_query($query)) {$errorLevel = 3;}
							}
						}
												
						if ($errorLevel == 3) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd!<br>");
						else if ($errorLevel == 0) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
						else if ($errorLevel == 1) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" is aan het systeem toegevoegd.<br>Alleen er is iets foutgegaan met het toevoegen van de melding! De melding is dus niet aan het systeem toegevoegd!");
						else if ($errorLevel == 2) echo("Het nieuwe component \"". $_POST['hidden_naam'] ."\" en bijbehorende melding is aan het systeem toegevoegd.<br>Alleen er is iets foutgegaan met het verwijzen van de melding naar het component!");
    				echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een component toe te voegen.</a>');
    			}
    			//er mag niet opgeslagen worden, dus toon het formulier met invoervelden
    			else {
						//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
						date_default_timezone_set ("Europe/Amsterdam");
    		?>
    	
    		<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>">
	    		<table>
	    			<tr>
	    				<td>Selecteer type om toe te voegen:</td>
	    				<td><select name="comp_type" id="comp_type" onchange="switchDocument(<?php if(isset($_POST['comp_naam'])){ echo("&n=". $_POST['hidden_naam']); } ?> );">
				    	  <?php
				    			$query = "SELECT Comp_Type, Type_Naam FROM comp_type WHERE Type_Parent IN (SELECT Comp_Type_ID FROM comp_lijst)";
				    			$resultaat = mysql_query($query);
				    			$selected = 'SELECTED';
							  	while ($data = mysql_fetch_array($resultaat)) {
			  	  				echo('<option value="'.$data['Comp_Type'].'"');
				  	  			
				  	  			//wanneer er data gepost is, dan....
				  	  			if (isset($_POST['comp_type'])) {
				  	  				//kijken of het huidige record hetzelfde is als de geposte record,
				  	  				//is dit het geval, dan dit record als de huidige selectie instellen
				  	  				if ($_POST['comp_type'] == $data['Comp_Type'])  {
				  	  					echo(" SELECTED"); 
				  	  					$selected = $data['Comp_Type'];
				  	  				}
				  	  			}
				  	  			//geen data gepost
				  	  			else {
				  	  				if ($selected == 'SELECTED') {
				  	  					echo($selected);
				  	  					$selected = $data['Comp_Type'];
				  	  				}
				  	  			}
				  	  			echo('>'. $data['Type_Naam'] .'</option>');
							  	}
				    		?></select>
				    	</td>
	    			</tr>
	    			<tr>
	    			</tr>
	    			<tr>
	    				<td>Naam van het component:</td>
	    				<td><iframe id="frame_naam" name="frame_naam" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_naam.php?c=<?php echo($selected); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="450" height="44" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
	    					<?php if (isset($_POST['hidden_naam']) && $_POST['hidden_naam'] == '') echo("<b>* Er is geen naam voor deze instantie ingevuld!</b>");?></td>
	    			</tr>
	    			<tr>
	    				<td>Selecteer de parent van het component:</td>
	    				<td><iframe id="frame_parent" name="frame_parent" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_parent.php?c=<?php echo($selected); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="450" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
	    			</tr>
	    			<tr>
	    				<td>Type melding:</td>
	    				<td><select name="type_melding" onchange="switchMelding();">
	    					<?php
	    						$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
			    			  $resultaat = mysql_query($query);

						  		if (isset($_POST['type_melding'])) $selectie = $_POST['type_melding'];
						  		else $selectie = 'SELECTED';

							  	while ($data = mysql_fetch_array($resultaat)) {
			  	  				echo('<option value="'.$data['Meld_Type_ID'].'"');
				  	  			if ($data['Meld_Type_ID'] == $selectie || $selectie == 'SELECTED') {
				  	  				echo('SELECTED');
				  	  				$selectie = $data['Meld_Type_ID'];
				  	  			}
				  	  			echo('>'. $data['Melding_Type_Naam'] .'</option>');
									}
	    					?>
	    					</select></td>
	    			</tr>
						<tr>
							<td>Melding beschrijving:</td>
							<td><iframe id="frame_melding" name="frame_melding" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_melding.php?c=<?php echo($selectie); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="450" height="72" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
							</td>
						</tr>
	    			<tr>
	    				<td>statusdatum:</td>
	    				<td><input name="statusdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['statusdatum'])) echo($_POST['statusdatum']); else echo(date('d-m-Y'));?>">
	    					  <input name="statustijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['statustijd'])) echo($_POST['statustijd']); else echo(date('H:i'));?>">
	    					  <?php if(isset($_POST['statusdatum']) && (!Valideer_Datum($_POST['statusdatum']) || !Valideer_Tijd($_POST['statustijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
	    			</tr>
	    			<tr>
	    				<td>Locatie:</td>
	    				<td><select name="comp_locatie">
	    				<?php
	    					$query = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
			    			$resultaat = mysql_query($query);
						  	while ($data = mysql_fetch_array($resultaat)) {
			  	  			echo('<option value="'.$data['Locatie_ID'].'">'. $data['Loc_Naam'] .'</option>');
						  	}
	    				?>	
	   					</select></td>
	    			</tr>
	    			<tr>
	    				<td>Verantwoordelijke:</td>
	    				<td><select name="comp_verantwoordelijke">
	    				<?php
	    					$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
			    			$resultaat = mysql_query($query);
						  	while ($data = mysql_fetch_array($resultaat)) {
			  	  			echo('<option value="'.$data['Werknem_ID'].'"');
			  	  			if ($_SESSION['gebr_id'] == $data['Werknem_ID']) echo(' SELECTED');
			  	  			echo('>'. $data['inlognaam'] .'</option>');
						  	}
	    				?>    					
	    				</select></td>
	    			</tr>
	    			<tr>
	    				<td>Fabricant contact:</td>
	    				<td><iframe id="frame_fabricant" name="frame_fabricant" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_fabricant.php?c=<?php echo($selected); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="450" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
	    				</td>
	    			</tr>
	    			<tr>
	    				<td>Leverancier contact:</td>
	    				<td><iframe id="frame_leverancier" name="frame_leverancier" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_leverancier.php?c=<?php echo($selected); if(isset($_POST['hidden_naam'])){ echo("&n=".$_POST['hidden_naam']); } ?>" width="450" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
	    				</td>
	    			</tr>
	    			<tr>
	    				<td>Leverdatum:</td>
	    				<td><input name="leverdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['leverdatum'])) echo($_POST['leverdatum']); else echo(date('d-m-Y'));?>">
	    					  <input name="levertijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['levertijd'])) echo($_POST['levertijd']); else echo(date('H:i'));?>">
	    					  <?php if(isset($_POST['leverdatum']) && (!Valideer_Datum($_POST['leverdatum']) || !Valideer_Tijd($_POST['levertijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
	    			</tr>
	    			<tr>
	    				<td>Fabricatiedatum:</td>
	    				<td><input name="fabricagedatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['fabricagedatum'])) echo($_POST['fabricagedatum']); else echo(date('d-m-Y'));?>">
	    					  <input name="fabricagetijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['fabricagetijd'])) echo($_POST['fabricagetijd']); else echo(date('H:i'));?>">
	    					  <?php if(isset($_POST['fabricagedatum']) && (!Valideer_Datum($_POST['fabricagedatum']) || !Valideer_Tijd($_POST['fabricagetijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
	    			</tr>
	    			<tr>
	    				<td><input id="hidden_type" name="hidden_type" type="hidden" value=""><input id="hidden_naam" name="hidden_naam" type="hidden" value="">
	    						<input id="hidden_aantal" name="hidden_aantal" type="hidden" value=""><input id="hidden_maximum" name="hidden_maximum" type="hidden" value="">
	    						<input id="hidden_fabricant" name="hidden_fabricant" type="hidden" value=""><input id="hidden_leverancier" name="hidden_leverancier" type="hidden" value="">
		    					<input id="hidden_melding" name="hidden_melding" type="hidden" value=""><input id="hidden_status" name="hidden_status" type="hidden" value=""></td>
	    				<td><a href="javascript:submitComponentToevoegen();">Toevoegen</a></td>
	    			</tr>
	    		</table>
	    	</form>
    	
    	<?php
    		}
    	?>
    	
    </div>    	
    	
<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>