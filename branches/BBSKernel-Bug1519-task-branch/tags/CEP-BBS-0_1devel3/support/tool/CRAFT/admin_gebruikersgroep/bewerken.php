<?php
	if (isset($_SESSION['admin_deel']))	{
		$_SESSION['admin_deel'] = 6;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
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
	    	
	    	<h2>Gebruikersgroepen bewerken</h2>
	
				<?php
					
					function Validatie_Opslaan() {
							if (isset($_POST['groepsnaam'])) {
								if($_POST['groepsnaam'] == '')
									return false;
							} else return false;
							
							return true;
					}
					
					function checkboxWaardes($waarde) {
						if (isset($_POST[$waarde]) && $_POST[$waarde] == 'on') 
							return "1";
						else return "0";
					}
					
					
					//het opslaan van de wijzigingen
					if(Validatie_Opslaan()) {
    				//Bekijken of de te bewerken groep een afgeleide van een admin-groep is
    				//dit kan door de waarde van het "admin_rechten" veld van de geselecteerde parent te bekijken
    				$query = "SELECT Admin_Rechten FROM gebruikers_groepen WHERE Groep_ID = '". $_POST['groepsparent'] ."'";
    				$resultaat = mysql_query($query);
						$row = mysql_fetch_array($resultaat);
						
						$query = "UPDATE gebruikers_groepen SET Groeps_Naam = '". htmlentities($_POST['groepsnaam'], ENT_QUOTES) ."', Groep_Parent = '". $_POST['groepsparent'] ."', ";
						$query = $query . "Intro_Zichtbaar ='". checkboxWaardes('Intro_Zichtbaar') ."', Comp_Zichtbaar ='". checkboxWaardes('Comp_Zichtbaar') ."', "; 
						$query = $query . "Melding_Zichtbaar = '". checkboxWaardes('Melding_Zichtbaar') ."', Stats_Zichtbaar = '". checkboxWaardes('Stats_Zichtbaar') ."', ";
						$query = $query . "Instel_Zichtbaar = '". checkboxWaardes('Inst_Zichtbaar') ."', Toevoegen = '". checkboxWaardes('Toevoeg_Rechten') ."', ";
						$query = $query . "Bewerken = '". checkboxWaardes('Bewerk_Rechten') ."', Verwijderen = '". checkboxWaardes('Verwijder_Rechten') ."', Admin_Rechten='". $row['Admin_Rechten'] ."'";
						$query = $query . " WHERE Groep_ID = '" . $_GET['c'] . "'";

						

						if (mysql_query($query)) echo("De gewijzigde gebruikersgroep \"". $_POST['groepsnaam'] ."\" is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van de gebruikersgroep \"". $_POST['groepsnaam'] ."\"!! De gebruikersgroep is niet bijgewerkt!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige type of selecteer links een gebruikersgroep uit de treeview.</a>');
					}
					//voor het eerst in deze pagina of fouten ontdekt voor het opslaan
					else {
						if (isset($_GET['c']) && $_GET['c'] != 0 ) {					
							$query = 'SELECT * FROM gebruikers_groepen WHERE Groep_ID = '. $_GET['c'];
					  	$resultaat = mysql_query($query);  	
					  	$row = mysql_fetch_array($resultaat);
							if ($row['Vaste_gegevens'] == 1) echo('<b>Dit is een vaste groep!<br>Deze groep kan niet aangepast worden!</b><br>');
				?>
		    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
	    			<table>
	    				<tr><td>Groepsnaam:</td><td><input name="groepsnaam" type="text" <?php if ($row['Vaste_gegevens'] == 1) echo('DISABLED'); ?>  value="<?php if (isset($_POST['groepsnaam'])) echo(htmlentities($_POST['groepsnaam'],ENT_QUOTES)); else echo($row['Groeps_Naam']); ?>">
			    					<?php if (isset($_POST['groepsnaam']) && $_POST['groepsnaam'] == '') echo("<b>* Er is geen naam voor deze groep gebruikers ingevuld!</b>");?></td>
	    				</tr>
	    				<tr>
	    					<td>Groepsparent:</td>
	    					<td><select name="groepsparent"  <?php if ($row['Vaste_gegevens'] == 1) echo('DISABLED'); ?>>
	    						<?php 
	    							$query = 'SELECT Groep_ID, Groeps_Naam FROM gebruikers_groepen WHERE Groep_ID >1';
										$resultaat = mysql_query($query);
				    			  if (isset($_POST['groepsparent'])) $selected = $_POST['groepsparent'];
				    			  else $selected = $row['Groep_Parent'];
	
	    							while ($data = mysql_fetch_array($resultaat)) {
											if ($data['Groep_ID'] != $_GET['c']) {
												echo("<option value=\"". $data['Groep_ID'] ."\" ");
												if ($selected == $data['Groep_ID'])
													echo ('SELECTED');
												echo(">". $data['Groeps_Naam'] ."</option>\r\n");
											}
										}
	    						 ?></select>
	    					</td>
	    				</tr>
	    				<tr>
	    					<td>Zichtbaar vanaf:</td>
	    					<td><select name="Comp_Type" <?php if ($row['Vaste_gegevens'] == 1) echo('DISABLED'); ?>>
	    						<?php 
	    							$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '". $_GET['c']."'";
	    							$resultaat = mysql_query($query);
	    							$data2 = mysql_fetch_array($resultaat);
	    							
	    							$query = 'SELECT Comp_Type, Type_Naam FROM comp_type';
										$resultaat = mysql_query($query);
				    			  if (isset($_POST['Comp_Type'])) $selected = $_POST['Comp_Type'];
				    			  else $selected = $data2['Comp_Type_ID'];
	
	    							while ($data = mysql_fetch_array($resultaat)) {
											echo("<option value=\"". $data['Comp_Type'] ."\" ");
											if ($selected == $data['Comp_Type'])
												echo ('SELECTED');
											echo(">". $data['Type_Naam'] ."</option>\r\n");
										}
	    						 ?></select>
	    					</td>
	    				</tr>
	    				<tr>
	    					<td>Onderliggende data vanaf component:</td>
	    					<td><?php 
    							$query = "SELECT onderliggende_Data FROM gebruikersgroeprechten WHERE Groep_ID = '". $_GET['c']."'";
    							$resultaat = mysql_query($query);
    							$data2 = mysql_fetch_array($resultaat);

			    				echo('<input id="onderliggende_data" name="onderliggende_data"  type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['onderliggende_data']) && ($_POST['onderliggende_data'] == 1 || $_POST['onderliggende_data'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($data2['onderliggende_Data'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
	    				</tr>
							<tr>
								<td>Intro scherm zichtbaar:</td>
								<td><?php 
			    				echo('<input id="Intro_Zichtbaar" name="Intro_Zichtbaar"  type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Intro_Zichtbaar']) && ($_POST['Intro_Zichtbaar'] == 1 || $_POST['Intro_Zichtbaar'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Intro_Zichtbaar'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Componentscherm zichtbaar:</td>
								<td><?php 
			    				echo('<input id="Comp_Zichtbaar" name="Comp_Zichtbaar" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Comp_Zichtbaar']) && ($_POST['Comp_Zichtbaar'] == 1 || $_POST['Comp_Zichtbaar'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Comp_Zichtbaar'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Meldingscherm zichtbaar:</td>
								<td><?php 
			    				echo('<input id="Melding_Zichtbaar" name="Melding_Zichtbaar" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Melding_Zichtbaar']) && ($_POST['Melding_Zichtbaar'] == 1 || $_POST['Melding_Zichtbaar'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Melding_Zichtbaar'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Statistiekenscherm zichtbaar:</td>
								<td><?php 
			    				echo('<input id="Stats_Zichtbaar" name="Stats_Zichtbaar" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Stats_Zichtbaar']) && ($_POST['Stats_Zichtbaar'] == 1 || $_POST['Stats_Zichtbaar'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Stats_Zichtbaar'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Instellingenscherm zichtbaar:</td>
								<td><?php 
			    				echo('<input id="Inst_Zichtbaar" name="Inst_Zichtbaar" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Inst_Zichtbaar']) && ($_POST['Inst_Zichtbaar'] == 1 || $_POST['Inst_Zichtbaar'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Instel_Zichtbaar'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Toevoegrechten:</td>
								<td><?php 
			    				echo('<input id="Toevoeg_Rechten" name="Toevoeg_Rechten" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Toevoeg_Rechten']) && ($_POST['Toevoeg_Rechten'] == 1 || $_POST['Toevoeg_Rechten'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Toevoegen'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Bewerkenrechten:</td>
								<td><?php 
			    				echo('<input id="Bewerk_Rechten" name="Bewerk_Rechten" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Bewerk_Rechten']) && ($_POST['Bewerk_Rechten'] == 1 || $_POST['Bewerk_Rechten'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Bewerken'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr>
								<td>Verwijderrechten:</td>
								<td><?php 
			    				echo('<input id="Verwijder_Rechten" name="Verwijder_Rechten" type="checkbox" ');
			    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
			    					if(isset($_POST['Verwijder_Rechten']) && ($_POST['Verwijder_Rechten'] == 1 || $_POST['Verwijder_Rechten'] == 'on')) 
			    						echo('CHECKED');
			    				}
			    				else if ($row['Verwijderen'] == 1) echo('CHECKED');
			    				if ($row['Vaste_gegevens'] == 1) echo(' DISABLED');
			    				echo('>');
			    			?></td>
							</tr>
							<tr><td><a href="javascript:document.theForm.submit();">Opslaan</a></td><td><input id="opslaan" name="opslaan" type="hidden" value="1"></td></tr>
	    			</table>
	    		</form>
	
				<?php 
					}
					else echo('Er is geen gebruikersgroep geselecteerd om te wijzigen.<br>Selecteer hiernaast een gebruikersgroep.'); 
				}
				?>
	    	
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");
	}
?>    	