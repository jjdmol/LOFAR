  	<?php
	
	$_SESSION['admin_deel'] = 2;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
  
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
    	
    	<h2>Component bewerken</h2>
    	
    	<?php
    	
    		function Child_Controle() {
					$query = "SELECT Count(Comp_Type_ID) FROM comp_lijst WHERE Comp_Type_ID = '".$_GET['c']."' GROUP BY Comp_Type_ID";
					$resultaat = mysql_query($query);
					if ($resultaat != null) {
						$data = mysql_fetch_array($resultaat);
						//er hangen componenten onder, dus false retourneren om de bewerking te stoppen
						if (isset($data[0]))
							return false;
					}	
					else return true;
    		}
    		
    		function Type_Controle() {
					//wanneer het parent component veranderd wordt...
					if (isset($_POST['comp_huidige_type']) && isset($_POST['comp_nieuwe_type']) && $_POST['comp_huidige_type'] != $_POST['comp_nieuwe_type']){
						//parent!! kijken of er componenten onder hangen. is dit zo, dan niet verplaatsen
						if (!Child_Controle()) return false;
					}
					return true;
    		}

    		function Parent_Controle() {
					//wanneer het parent component veranderd wordt...
					if (isset($_POST['comp_huidige_parent']) && isset($_POST['comp_nieuwe_parent']) && $_POST['comp_huidige_parent'] != $_POST['comp_nieuwe_parent']){
						//parent!! kijken of er componenten onder hangen. is dit zo, dan niet verplaatsen
  					if (!Child_Controle()) return false;
					}					
					return true;
    		}
    	
				//validatie functie om te kijken of er opgeslagen mag worden.
				function Validatie_Opslaan(){
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
						return false;

					if (isset($_POST['comp_naam'])) {
						if ($_POST['comp_naam'] == '')
							return false;
					} else return false;

					if (!Type_Controle()) return false;					
					if (!Parent_Controle()) return false;

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
					return true;
				}

				//eerst een validatie doen om de ingevoerde gegevens te controleren en te kijken of er opgeslagen mag worden...
    		if(Validatie_Opslaan()) {

					//Een nieuwe melding van het component toevoegen, dit is zeer waarschijnlijk een bewerkenmelding oid
					$query_melding = "INSERT INTO melding_lijst (Meld_Type_ID, Comp_Lijst_ID, Meld_Datum, Huidige_Status, Voorgaande_Melding, Prob_Beschrijving, Behandeld_Door, Gemeld_Door)";
					$query_melding = $query_melding . "VALUES ('". $_POST['type_melding'] ."', '".$_GET['c']."'";

  				//het toevoegen van een statusdatum: eerst kijken of er 1 ingevuld is, anders de huidige datum gebruiken...
  				if (isset($_POST['statusdatum']) && $_POST['statusdatum'] != '') {
    				$datum=split("-",$_POST['statusdatum']);
	  				$query_melding = $query_melding . ", '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['statustijd'] .":00'";							
					}
					else $query_melding = $query_melding . ", NOW()";
					$query_melding = $query_melding . ", '". $_POST['hidden_status'] ."', '". $_POST['Voorgaande_Melding'] ."', '". $_POST['hidden_melding'] ."', '". $_SESSION['gebr_id'] ."', '". $_SESSION['gebr_id'] ."') ";

					mysql_query($query_melding);
					$melding_id = mysql_insert_id();

					//opslaan van het component
					$query = "UPDATE comp_lijst SET Comp_Naam = '". $_POST['comp_naam'] . "', Comp_Parent = '". $_POST['comp_nieuwe_parent'] . "', Laatste_Melding ='". $melding_id;
					$query = $query . "', Comp_Locatie = '". $_POST['comp_locatie'] ."', Comp_Verantwoordelijke = '". $_POST['comp_verantwoordelijke'] . "'";
					
  				//de waarde voor de leverdatum aan de query toevoegen
  				if (isset($_POST['leverdatum']) && $_POST['leverdatum'] != '') {
    				$datum = split("-",$_POST['leverdatum']);
	  				$query = $query . ", Lever_Datum = '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['levertijd'] .":00'";
	  			}

  				//de waarde voor de fabricagedatum aan de query toevoegen
  				if (isset($_POST['fabricagedatum']) && $_POST['fabricagedatum'] != '') {
    				$datum = split("-",$_POST['fabricagedatum']);
    				$query = $query . ", Fabricatie_Datum = '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['fabricagetijd'] .":00'";
  				}
				
					$query = $query . ", Contact_Leverancier='".$_POST['leverancier']."', Contact_Fabricant='".$_POST['fabricant']."' WHERE Comp_Lijst_ID = '" . $_GET['c'] . "'";

					if (mysql_query($query)) echo("Het gewijzigde component \"". $_POST['comp_naam'] ."\" is in het systeem bijgewerkt<br>");
					else("Er is iets mis gegaan met het opslaan van het component \"". $_POST['comp_naam'] ."\"!! Het component is niet bijgewerkt!");
					echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige component of selecteer links een component uit de treeview.</a>');
										
    		}
    		else {
					//de tijdzone waarin we leven instellen, wordt dit niet gedaan dan klaagt PHP
					date_default_timezone_set ("Europe/Amsterdam");
    	
	    		if (isset($_GET['c']) && $_GET['c'] != 0 ) {
						$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID ='". $_GET['c'] ."'";
						$resultaat = mysql_query($query);
				  	$row = mysql_fetch_array($resultaat);
				?>
		    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
						<table>
							<tr>
								<td>Component ID:</td>
								<td> <?php echo($row['Comp_Lijst_ID']); ?></td>
							</tr>
							<tr>
								<td>Naam component:</td>
								<td><input name="comp_naam" type="text" value="<?php if (isset($_POST['comp_naam'])) echo($_POST['comp_naam']); else echo($row['Comp_Naam']); ?>">
				    		<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['comp_naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>');?></td>
							</tr>
							<tr>
								<td>Type component:</td>
								<td>
									<?php 
										$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type = " . $row['Comp_Type_ID'];
										$result = mysql_query($query);
										$data = mysql_fetch_array($result);
										echo($data[0]);
  							 ?></td>
							</tr>
							<tr>
								<td>Parent component:</td>
								<td><input type="hidden" name="comp_huidige_parent" value="<?php echo($row['Comp_Parent']); ?>"><select name="comp_nieuwe_parent">
									<?php
									  $query2 = "SELECT Comp_Lijst_ID, Comp_Naam FROM comp_lijst WHERE Comp_Type_ID in (SELECT Type_Parent FROM comp_type WHERE Comp_Type = '".$row['Comp_Type_ID']."')";
										$resultaat2 = mysql_query($query2);
										while ($data = mysql_fetch_array($resultaat2)) {
											echo("<option value=\"". $data['Comp_Lijst_ID'] ."\"");
											if (isset($_POST['comp_nieuwe_parent']) && isset($_POST['comp_huidige_parent']) && $_POST['comp_nieuwe_parent'] != $_POST['comp_huidige_parent']) {
												if ($_POST['comp_nieuwe_parent'] == $data['Comp_Lijst_ID']) echo(" SELECTED"); }
											else {if ($data['Comp_Lijst_ID'] == $row['Comp_Parent']) echo(" SELECTED");}
											echo(">". $data['Comp_Naam'] ."</option>\r\n");
										}
									?></select>
				    		<?php if(!Parent_Controle()) echo('<b>* Parent kan niet veranderen vanwege onderliggende componenten!</b>');?>
								</td>
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
								<td>Status datum:</td>
								<td>
									<input name="statusdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['statusdatum'])) echo($_POST['statusdatum']); else echo(date('d-m-Y')); ?>">
									<input name="statustijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['statustijd'])) echo($_POST['statustijd']); else echo(date('H:i')); ?>">
	    					  <?php if(isset($_POST['statusdatum']) && (!Valideer_Datum($_POST['statusdatum']) || !Valideer_Tijd($_POST['statustijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
							</tr>
							<tr>
								<td>Locatie component:</td>
								<td><select name="comp_locatie">
									<?php
										$query2 = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
										$result = mysql_query($query2);
										while ($data = mysql_fetch_array($result)) {
											echo("<option value=\"". $data['Locatie_ID'] ."\"");
											if ($data['Locatie_ID'] == $row['Comp_Locatie']) echo(" SELECTED");
											echo(">". $data['Loc_Naam'] ."</option>\r\n");
										}
									?>
								</select></td>
							</tr>
							<tr>
								<td>Verantwoordelijke component:</td>
								<td><select name="comp_verantwoordelijke">
									<?php
										$query2 = "SELECT Werknem_ID, inlognaam FROM gebruiker";
										$result = mysql_query($query2);
										while ($data = mysql_fetch_array($result)) {
											echo("<option value=\"". $data['Werknem_ID'] ."\"");
											if ($data['Werknem_ID'] == $row['Comp_Verantwoordelijke']) echo(" SELECTED");
											echo(">". $data['inlognaam'] ."</option>\r\n");
										}
									?>
								</select></td>
							</tr>
							<tr>
								<td>Fabricant:</td>
								<td><select name="fabricant">				    				
									<?php
					    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
				    			  $resultaat = mysql_query($query);
			  						if (isset($_POST['fabricant'])) $selectie = $_POST['fabricant'];
			  						else $selectie = $row['Contact_Fabricant'];
										
								  	while ($data = mysql_fetch_array($resultaat)) {
								  		echo('<option value="'. $data['Contact_ID'] .'"');
								  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
								  			echo('SELECTED');
								  		echo('>'. $data['Contact_Naam'] .'</option>');
								  	}
							    ?></select>
								</td>
							</tr>
							<tr>
								<td>Leverancier:</td>
								<td><select name="leverancier">									
									<?php
					    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
				    			  $resultaat = mysql_query($query);
			  						if (isset($_POST['leverancier'])) $selectie = $_POST['leverancier'];
			  						else $selectie = $row['Contact_Leverancier'];
										
								  	while ($data = mysql_fetch_array($resultaat)) {
								  		echo('<option value="'. $data['Contact_ID'] .'"');
								  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
								  			echo('SELECTED');
								  		echo('>'. $data['Contact_Naam'] .'</option>');
								  	}
							    ?></select></td>
							</tr>
							<tr>
								<td>Leverdatum:</td>
								<td>
									<?php 
										//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			    					$gedeeldveld=split(" ",$row['Lever_Datum']);
										//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
										$datum = split("-",$gedeeldveld[0]);
										//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
										$tijd = split(":",$gedeeldveld[1]);
									 ?>
									<input name="leverdatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['leverdatum'])) echo($_POST['leverdatum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
									<input name="levertijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['levertijd'])) echo($_POST['levertijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
	    					  <?php if(isset($_POST['leverdatum']) && (!Valideer_Datum($_POST['leverdatum']) || !Valideer_Tijd($_POST['levertijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
							</tr>
							<tr>
								<td>Fabricagedatum:</td>
								<td>
									<?php 
										//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			    					$gedeeldveld=split(" ",$row['Fabricatie_Datum']);
										//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
										$datum = split("-",$gedeeldveld[0]);
										//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
										$tijd = split(":",$gedeeldveld[1]);
									 ?>
									<input name="fabricagedatum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['fabricagedatum'])) echo($_POST['fabricagedatum']); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
									<input name="fabricagetijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['fabricagetijd'])) echo($_POST['fabricagetijd']); else echo($tijd[0] .":". $tijd[1]); ?>">
	    					  <?php if(isset($_POST['fabricagedatum']) && (!Valideer_Datum($_POST['fabricagedatum']) || !Valideer_Tijd($_POST['fabricagetijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
							</tr>
			    		<tr>
								<td id="opslaan" align="right"><a href="javascript:SubmitComponentBewerken();">Opslaan</a></td>
			    			<td>
		    					<input id="hidden_melding" name="hidden_melding" type="hidden" value="">
		    					<input id="hidden_status" name="hidden_status" type="hidden" value="">
			    				<input id="opslaan" name="opslaan" type="hidden" value="1">
			    				<input id="Voorgaande_Melding" name="Voorgaande_Melding" type="hidden" value="<?php echo($row['Laatste_Melding']); ?>">
			    			</td>
			    		</tr>
						</table>
					</form>
				
				<?php
	    		}
					else echo('Er is geen component geselecteerd om te wijzigen.<br>Selecteer hiernaast een component.'); 
				}
    	?>
    </div>    	
<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>