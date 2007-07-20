<?php
	if (isset($_SESSION['admin_deel'])) {
		$_SESSION['admin_deel'] = 6;
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
	    	
	    	<h2>Gebruikersgroepen verwijderen</h2>
	
				<?php
	  			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
						$errorlevel = 0;
						//eerst de gebruikersgroeprechten verwijderen
						$query = "DELETE FROM gebruikersgroeprechten WHERE Groep_ID = '". $_POST['groep'] ."'";
						if (mysql_query($query)) {
							$errorlevel = 1;
							//hierna de gebruikersgroep verwijderen
							$query = "DELETE FROM gebruikers_groepen WHERE Groep_ID = " . $_POST['groep'];
							if (mysql_query($query)) {$errorlevel = 2;}
						}	
						
						if ($errorlevel == 2) echo("De door u geselecteerde gebruikersgroep is uit het systeem verwijderd.<br>");
						else if ($errorlevel == 0) echo ("Er is iets mis gegaan met het verwijderen van de geselecteerde gebruikersgroep!! De gebruikersgroep is niet verwijderd!<br>"); 
						else if ($errorlevel == 1) echo ("Er is iets mis gegaan met het verwijderen van de geselecteerde gebruikersgroep!! De groepsrechten zijn echter wel verwijderd!<br>");
						echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een gebruikersgroep uit de treeview.</a>');
	  			}
	  			else {
	  				
	    			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = "SELECT Groeps_Naam, Vaste_gegevens FROM gebruikers_groepen WHERE Groep_ID = '". $_GET['c'] ."'";
							$resultaat = mysql_query($query);
							$row = mysql_fetch_row($resultaat);
							
							echo('U heeft de gebruikersgroep "'. $row[0] .'" geselecteerd:<br>');
							
							if ($row[1] == 0) {
							
								//eerst kijken of er gebruikers onder deze groep hangen
								$query = "SELECT COUNT(Groep_ID) FROM gebruiker WHERE Groep_ID = '". $_GET['c'] ."' GROUP BY Groep_ID";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								//er zijn geen gebruikers behorende bij deze groep.
								if ($row[0] == NULL) {
									
									//kijken of deze groep deel uitmaakt van een hierarchie
									$query2 = "SELECT COUNT(Groep_ID) FROM gebruikers_groepen WHERE Groep_Parent = '". $_GET['c'] ."'";
									$resultaat2 = mysql_query($query2);
									$row2 = mysql_fetch_row($resultaat2);
									//geen childs gevonden, dit is dus het onderste deel van een hierarchie
									//er mag verwijderd worden!!!!!!!!!!!!!!!
									if ($row2[0] == NULL || $row2[0] ==0) {
										//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
										?>
								    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
								    		<table>
								    			<tr><td><input type="hidden" name="groep" value="<?php echo($_GET['c']);?>">Weet u zeker dat u deze gebruikersgroep verwijderen wilt?</td></tr>
								    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil deze gebruikersgroep verwijderen</td></tr>
								    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
								    		</table>
								    	</form>
										<?php
									}
									else echo("<br>Deze gebruikersgroep heeft onderliggende gebruikersgroepen, welke naar deze gebruikersgroep verwijzen.<br>Hierdoor kan deze gebruikersgroep niet verwijderd worden!");
								}
								//wel gebruikers behorende bij deze groep, dus stoppen (met een melding)
								else echo("<br>Er zijn gebruikers van deze gebruikersgroep horen.<br>Hierdoor is het niet mogelijk om deze gebruikersgroep te verwijderen!");
							}
							else echo("<br>De geselecteerde gebruikersgroep is een vaste groep.<br>Hierdoor is het niet mogelijk om deze gebruikersgroep te verwijderen!");
						}
						else echo("Er is geen gebruikersgroep geselecteerd om te verwijderen<br>Selecteer hiernaast een gebruikersgroep.");
					}
				
				?>
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	