<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 7;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');  
		
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
	    	
	    	<h2>Gebruikers toevoegen</h2>
			    
			    <?php
	 	    		function Valideer_Invoer() {
							//Gebr_Naam
							if (isset($_POST['Gebr_Naam'])) {
								if ($_POST['Gebr_Naam'] == '')
									return false;
							} else return false;
	 	    			
	 	    			//Wachtwoord
							if (isset($_POST['Wachtwoord'])) {
								if ($_POST['Wachtwoord'] == '')
									return false;
							} else return false;
	
	 	    			//Gebr_Email
							if (isset($_POST['Gebr_Email'])) {
								if ($_POST['Gebr_Email'] != '' && !mail_check($_POST['Gebr_Email']))
									return false;
							} else return false;
	
	 	    			return true;
	 	    		}
	 	    		
	 	    		
	 	    		if (Valideer_Invoer()) {
							$query = "INSERT INTO gebruiker (inlognaam, Wachtwoord, Start_Alg, Start_Comp, Start_Melding, Start_Stats, Groep_ID, Gebruiker_Taal, Emailadres, Laatst_Ingelogd) ";
							$query = $query . "VALUES ('". $_POST['Gebr_Naam'] ."', '". md5(strtolower($_POST['Wachtwoord'])) ."', '". $_POST['Alg_Start'] ."' ,'". $_POST['Comp_Start'] ."', '". $_POST['Melding_Start'];
							$query = $query . "', '". $_POST['Stats_Start'] ."', '" . $_POST['Gebr_Groep'] ."', '". $_POST['Gebr_Taal'] ."', '". $_POST['Gebr_Email'] ."', NOW())";
							
							if (mysql_query($query)) echo("De nieuwe gebruiker \"". $_POST['Gebr_Naam'] ."\" is aan het systeem toegevoegd<br>");
							else echo("De nieuwe gebruiker \"". $_POST['Gebr_Naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
							echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een gebruiker toe te voegen.</a>');   			
	 	    		} 	    		
	 	    		else {
	 	    			
			    ?>
			    <form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
	    			<table>
	    				<tr>
	    					<td>Gebruikersnaam:</td>
	    					<td><input name="Gebr_Naam" type="text" value="<?php if(isset($_POST['Gebr_Naam'])) echo(htmlentities($_POST['Gebr_Naam'], ENT_QUOTES)); ?>">
		    				  <?php if(isset($_POST['Gebr_Naam']) && $_POST['Gebr_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?>
		    				</td>
	    				</tr>
	    				<tr>
	    					<td>Groep:</td>
	    					<td><select name="Gebr_Groep">
	    						<?php 
	    							$query = 'SELECT Groep_ID, Groeps_Naam FROM gebruikers_groepen WHERE Groep_ID >1';
										$resultaat = mysql_query($query);
										
										$selectie = 'SELECTED';
							  		if(isset($_POST['Gebr_Groep'])) $selectie  = $_POST['Gebr_Groep'];
										
	    							while ($data = mysql_fetch_array($resultaat)) {
											echo("<option value=\"". $data['Groep_ID'] ."\" ");
								  		
								  		if($data['Groep_ID'] == $selectie || $selectie == 'SELECTED') {
								  			echo ('SELECTED');
								  			$selectie = -1;
								  		}
											echo(">". $data['Groeps_Naam'] ."</option>\r\n");
										}
	    						 ?></select></td>
	    				</tr>
	    				<tr>
	    					<td>Wachtwoord:</td>
	    					<td><input name="Wachtwoord" type="text">
		    				  <?php if(isset($_POST['Wachtwoord']) && $_POST['Wachtwoord'] == '') echo('<b>* Er is geen wachtwoord ingevoerd!</b>'); ?>
		    				</td>
	    				</tr>
	    				<tr>
	    					<td>E-mailadres:</td>
	    					<td><input name="Gebr_Email" type="text" value="<?php if(isset($_POST['Gebr_Email'])) echo(htmlentities($_POST['Gebr_Email'], ENT_QUOTES)); ?>">
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
	  								else $selectie = 1;
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
	  								else $selectie = 1;
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
	  								else $selectie = 1;
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
	  								else $selectie = 1;
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
	  								else $selectie = 1;
	  							?>
	
	   							<option value="1" <?php if($selectie == 1) echo('SELECTED'); ?>>Statistieken overzicht</option>
	   							<option value="2" <?php if($selectie == 2) echo('SELECTED'); ?>>Statistieken toevoegen</option>
	   							<option value="3" <?php if($selectie == 3) echo('SELECTED'); ?>>Statistieken bewerken</option>
	   							<option value="4" <?php if($selectie == 4) echo('SELECTED'); ?>>Statistieken verwijderen</option>
	   						</select></td>
	    				</tr>
	    				<tr>
			    			<td><input name="opslaan" type="hidden" value="1"></td>
	    					<td><a href="javascript:document.theForm.submit();">Toevoegen</a></td>
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
	}
?>    	