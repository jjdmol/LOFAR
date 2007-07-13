<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 5;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_gebruikersgroep/groepen_functies.php\"></script>");
	
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
	    	
	    	<h2>Gebruikersgroepen toevoegen</h2>
	    		
					<?php
						
						function Valideer_Invoer() {
							if (isset($_POST['groepsnaam'])) {
								if($_POST['groepsnaam'] == '')
									return false;
							} else return false;
							
							return true;
						}
						
						
						function Omzetten_Checkbox($waarde){
							if ($waarde == 'true')
								return '1';
							else return '0';
						}
						
						
						if(Valideer_Invoer()) {
	
	    				//Bekijken of de toe te voegen groep een afgeleide van een admin-groep is
	    				//dit kan door de waarde van het "admin_rechten" veld van de geselecteerde parent te bekijken
	    				$query = "SELECT Admin_Rechten FROM gebruikers_groepen WHERE Groep_ID = '". $_POST['groepsparent'] ."'";
	    				$resultaat = mysql_query($query);
							$row = mysql_fetch_array($resultaat);

	    				$query = "INSERT INTO gebruikers_groepen (Groeps_Naam, Admin_Rechten, Groep_Parent, Intro_Zichtbaar, Comp_Zichtbaar, Melding_Zichtbaar, Stats_Zichtbaar, ";
	    				$query = $query . "Instel_Zichtbaar, Toevoegen, Bewerken, Verwijderen) VALUES ('". htmlentities($_POST['groepsnaam'], ENT_QUOTES) ."', '". $row['Admin_Rechten'] ."', '". $_POST['groepsparent'] ."', '";
							$query = $query . Omzetten_Checkbox($_POST['hidden_intro']) ."', '". Omzetten_Checkbox($_POST['hidden_comp']) ."', '". Omzetten_Checkbox($_POST['hidden_melding']) ."', '". Omzetten_Checkbox($_POST['hidden_stats']) ."', '";
							$query = $query . Omzetten_Checkbox($_POST['hidden_inst']) ."', '" . Omzetten_Checkbox($_POST['hidden_toevoeg']) ."', '". Omzetten_Checkbox($_POST['hidden_bewerk']) ."', '". Omzetten_Checkbox($_POST['hidden_verwijder']) ."')";
							
							if (mysql_query($query)) echo("De nieuwe groep \"". $_POST['groepsnaam'] ."\" is aan het systeem toegevoegd<br>");
							else echo("De nieuwe groep \"". $_POST['groepsnaam'] ."\" kon niet aan het systeem toegevoegd worden!.");
	    				echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een groep toe te voegen.</a>');
						}
						else {
						
					?>
	    		
	    		<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>">
	    			<table>
	    				<tr><td>
			    			<table>
			    				<tr><td>Groepsnaam:</td><td><input name="groepsnaam" type="text">
			    					<?php if (isset($_POST['groepsnaam']) && $_POST['groepsnaam'] == '') echo("<b>* Er is geen naam voor deze groep gebruikers ingevuld!</b>");?></td>
			    				</tr>
			    				<tr>
			    					<td>Groepsparent:</td>
			    					<td><select name="groepsparent" id="groepsparent" onchange="switchDocument();">
			    						<?php 
			    							$query = 'SELECT Groep_ID, Groeps_Naam FROM gebruikers_groepen WHERE Groep_ID >1';
												$resultaat = mysql_query($query);
							    			$selected = 'SELECTED';
	
			    							while ($data = mysql_fetch_array($resultaat)) {
													echo("<option value=\"". $data['Groep_ID'] ."\" ");
													if ($selected == 'SELECTED') {
														echo ($selected);
														$selected = $data['Groep_ID'];
													}
													echo(">". $data['Groeps_Naam'] ."</option>\r\n");
												}
			    						 ?></select>
			    					</td>
			    				</tr>
			    			</table>
	    				</td></tr>
	    				<tr><td><iframe id="frame_gegevens" name="frame_gegevens" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_gebruikersgroep/groep_gegevens.php?c=<?php echo($selected); ?>" width="450" height="195" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td></tr>
	    				<tr><td>
	    					<input name="hidden_inst" id="hidden_inst" type="hidden" value="">	
	    					<input name="hidden_comp" id="hidden_comp" type="hidden" value="">	
	    					<input name="hidden_intro" id="hidden_intro" type="hidden" value="">	
	    					<input name="hidden_stats" id="hidden_stats" type="hidden" value="">	
	    					<input name="hidden_bewerk" id="hidden_bewerk" type="hidden" value="">	
	    					<input name="hidden_melding" id="hidden_melding" type="hidden" value="">	
	    					<input name="hidden_toevoeg" id="hidden_toevoeg" type="hidden" value="">	
	    					<input name="hidden_verwijder" id="hidden_verwijder" type="hidden" value="">	
	    					<a href="javascript:submitGroepToevoegen();">Toevoegen</a>
	    				</td></tr>
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
	}
?>    	