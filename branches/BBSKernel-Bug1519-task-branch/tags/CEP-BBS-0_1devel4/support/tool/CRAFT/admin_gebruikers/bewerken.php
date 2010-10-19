<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 7;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');  
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
		
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
	    	
	    	<h2>Gebruikers bewerken</h2>
				
				<?php
				
	 	    		function Valideer_Invoer() {
							//Gebr_Naam
							if (isset($_POST['Gebr_Naam'])) {
								if ($_POST['Gebr_Naam'] == '')
									return false;
							} else return false;
	 	    				
	 	    			//Gebr_Email
							if (isset($_POST['Gebr_Email'])) {
								if ($_POST['Gebr_Email'] != '' && !mail_check($_POST['Gebr_Email']))
									return false;
							} else return false;
	
							//Laatst_Ingelogd
		  				if (isset($_POST['Inlog_Datum'])) {
		  					//wanneer de Inlog_Datum ingevuld is, dan... 
		  					if($_POST['Inlog_Datum'] !='') {
		
		   						//controleren op de juiste samenstelling van de Inlog_Datum
			   					if (Valideer_Datum($_POST['Inlog_Datum']) == false)
			   						return false;
		  						//controleren of de tijd correct ingevoerd is
		    					if(isset($_POST['Inlog_Tijd'])) {
		    						if (Valideer_Tijd($_POST['Inlog_Tijd']) == false)
		    					 		return false;
		  						} else return false;
		   					} else return false;
		     			} else return false;
	
	 	    			return true;
	 	    		}
				
					if(Valideer_Invoer()) {
						$query = "UPDATE gebruiker SET inlognaam = '". $_POST['Gebr_Naam'] ."', ";
						if ($_POST['Wachtwoord'] != '')
							$query = $query . "Wachtwoord = '". md5(strtolower($_POST['Wachtwoord'])) ."', ";
						
						$query = $query . "Start_Alg='". $_POST['Alg_Start'] ."'";
						$query = $query . ", Start_Comp='". $_POST['Comp_Start'] ."', Start_Melding = '". $_POST['Melding_Start'] ."', Start_Stats='". $_POST['Stats_Start'] ."'";
						$query = $query . ", Groep_ID = '". $_POST['Gebr_Groep'] ."', Gebruiker_Taal='". $_POST['Gebr_Taal'] ."', Emailadres = '". $_POST['Gebr_Email'] ."'";

	  				//de waarde voor de inlogdatum aan de query toevoegen
	  				if (isset($_POST['Inlog_Datum']) && $_POST['Inlog_Datum'] != '') {
	    				$datum = split("-",$_POST['Inlog_Datum']);
	    				$query = $query . ", Laatst_Ingelogd = '". $datum[2]."-".$datum[1]."-".$datum[0] ." ". $_POST['Inlog_Tijd'] .":00'";
	  				}
						$query = $query . " WHERE Werknem_ID = '" . $_GET['c'] . "'";
						
						if (mysql_query($query)) echo("De gewijzigde gebruiker \"". $_POST['Gebr_Naam'] ."\" is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van de gebruiker \"". $_POST['Gebr_Naam'] ."\"!! De gebruiker is niet bijgewerkt!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar de vorige gebruiker of selecteer links een gebruiker uit de treeview.</a>');
	
					}
					else {
						if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = 'SELECT * FROM gebruiker WHERE Werknem_ID = '. $_GET['c'];
					  	$resultaat = mysql_query($query);  	
					  	$row = mysql_fetch_array($resultaat);
				?>
					<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>&c=<?php echo($_GET['c']); ?>">
	    			<table>
	    				<tr>
	    					<td>Gebruikersnaam:</td>
	    					<td><input name="Gebr_Naam" type="text"  value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Gebr_Naam'], ENT_QUOTES)); else echo($row['inlognaam']); ?>">
		    				  <?php if(isset($_POST['Gebr_Naam']) && $_POST['Gebr_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?>
		    				</td>
	    				</tr>
	    				<tr>
	    					<td>Groep:</td>
	    					<td><select name="Gebr_Groep">
	    						<?php 
	    							$query = 'SELECT Groep_ID, Groeps_Naam FROM gebruikers_groepen WHERE Groep_ID >1';
										$resultaat = mysql_query($query);
							  		if (isset($_POST['Gebr_Groep'])) $selectie = $_POST['Gebr_Groep'];
							  		else $selectie = $row['Groep_ID'];
	
	    							while ($data = mysql_fetch_array($resultaat)) {
											echo("<option value=\"". $data['Groep_ID'] ."\" ");
											if ($data['Groep_ID'] == $selectie)
												echo ('SELECTED');
											echo(">". $data['Groeps_Naam'] ."</option>\r\n");
										}
	    						 ?></select></td>
	    				</tr>
	    				<tr>
	    					<td>Wachtwoord:</td>
	    					<td><input name="Wachtwoord" type="text"></td>
	    				</tr>
	    				<tr>
	    					<td>E-mailadres:</td>
	    					<td><input name="Gebr_Email" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Gebr_Email'], ENT_QUOTES)); else echo($row['Emailadres']); ?>">
		    				  <?php
		    				  	if (isset($_POST['Gebr_Email']))
		    				  		$mail = $_POST['Gebr_Email'];
		    				  	else $mail = '';
	
		    				  	if(isset($_POST['opslaan'])) {
											if ($mail == '' || !mail_check($mail))
		   				  				echo('<b>* Er is geen (geldig) e-mailadres ingevoerd!</b>');
		    				  	}
		    				  ?>    				  
	    					</td>
	    				</tr>
	    				<tr>
	    					<td>Gebruikerstaal:</td>
	    					<td><select name="Gebr_Taal">
	  							<?php 
	  								if (isset($_POST['Gebr_Taal']))
	  									$selectie = $_POST['Gebr_Taal'];
	  								else $selectie = $row['Gebruiker_Taal'];
	  							?>
	  							
	  							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Nederlands</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Engels</option>
	  						</select></td>
	    				</tr>
	    				<tr>
	    					<td>Algemene startpagina:</td>
	    					<td><select name="Alg_Start">
	  							<?php 
	  								if (isset($_POST['Alg_Start']))
	  									$selectie = $_POST['Alg_Start'];
	  								else $selectie = $row['Start_Alg'];
	  							?>
	
	   							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Intro scherm</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Componenten</option>
	   							<option value="3" <?php if($selectie == 3) echo('SELECTED'); ?>>Meldingen</option>
	   							<option value="4" <?php if($selectie == 4) echo('SELECTED'); ?>>Statistieken</option>
	   							<option value="5" <?php if($selectie == 5) echo('SELECTED'); ?>>Instellingen</option>     							
	  						</select></td>
	    				</tr>
	    				<tr>
	    					<td>Componenten startpagina:</td>
	    					<td><select name="Comp_Start">
	  							<?php 
	  								if (isset($_POST['Comp_Start']))
	  									$selectie = $_POST['Comp_Start'];
	  								else $selectie = $row['Start_Comp'];
	  							?>
	
	   							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Componenten overzicht</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Componenten toevoegen</option>
	   							<option value="3" <?php if($selectie == 3) echo('SELECTED'); ?>>Componenten bewerken</option>
	   							<option value="4" <?php if($selectie == 4) echo('SELECTED'); ?>>Componenten verwijderen</option>
	  						</select></td>
	    				</tr>
	    				<tr>
	    					<td>Meldingen startpagina:</td>
	    					<td><select name="Melding_Start">
	  							<?php 
	  								if (isset($_POST['Melding_Start']))
	  									$selectie = $_POST['Melding_Start'];
	  								else $selectie = $row['Start_Melding'];
	  							?>
	
	   							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Meldingen overzicht</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Meldingen toevoegen</option>
	   							<option value="3" <?php if($selectie == 3) echo('SELECTED'); ?>>Meldingen bewerken</option>
	   							<option value="4" <?php if($selectie == 4) echo('SELECTED'); ?>>Meldingen verwijderen</option>
	  						</select></td>
	    				</tr>
	    				<tr>
	    					<td>Statistieken startpagina:</td>
	    					<td><select name="Stats_Start">
	  							<?php 
	  								if (isset($_POST['Stats_Start']))
	  									$selectie = $_POST['Stats_Start'];
	  								else $selectie = $row['Start_Stats'];
	  							?>
	
	   							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Statistieken overzicht</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Statistieken toevoegen</option>
	   							<option value="3" <?php if($selectie == 3) echo('SELECTED'); ?>>Statistieken bewerken</option>
	   							<option value="4" <?php if($selectie == 4) echo('SELECTED'); ?>>Statistieken verwijderen</option>
	   						</select></td>
	    				</tr>
	    				<tr>
	    					<td>Laatste inlog:</td>
	    					<td>
									<?php 
										//splitten op de spatie (formaat is als volgt: 2007-08-26 12:01:56)
			    					$gedeeldveld=split(" ",$row['Laatst_Ingelogd']);
										//datum veld opdelen zodat de jaar, maand en dagvelden makkelijk te benaderen zijn
										$datum = split("-",$gedeeldveld[0]);
										//tijd veld opdelen zodat de uren, minuten en secondevelden makkelijk te benaderen zijn
										$tijd = split(":",$gedeeldveld[1]);
									 ?>
									<input name="Inlog_Datum" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Inlog_Datum'])) echo(htmlentities($_POST['Inlog_Datum'], ENT_QUOTES)); else echo($datum[2] ."-". $datum[1] ."-". $datum[0]); ?>">
									<input name="Inlog_Tijd" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Inlog_Tijd'])) echo(htmlentities($_POST['Inlog_Tijd'], ENT_QUOTES)); else echo($tijd[0] .":". $tijd[1]); ?>">	    						
	    					  <?php if(isset($_POST['Inlog_Datum']) && (!Valideer_Datum($_POST['Inlog_Datum']) || !Valideer_Tijd($_POST['Inlog_Tijd']))) echo('<b>* De ingevoerde datum/tijd is onjuist samengesteld!</b>'); ?></td>
	    					</td>
	    				</tr>
	    				<tr>
			    			<td><input name="opslaan" type="hidden" value="1"></td>
	    					<td><a href="javascript:document.theForm.submit();">Opslaan</a></td>
	    				</tr>
	    			</table>
	    		</form>
				
				<?php 
						}
						else echo('Er is geen gebruiker geselecteerd om te wijzigen.<br>Selecteer hiernaast een contact.');
					}
				?>
	
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	