<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 7;
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
	    	
	    	<h2>Gebruikers verwijderen</h2>
				<?php
	    			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
							$query = "DELETE FROM gebruiker WHERE Werknem_ID = " . $_POST['gebruiker'];
							if (mysql_query($query)) echo("De door u geselecteerde gebruiker is uit het systeem verwijderd.<br>");
							else("Er is iets mis gegaan met het verwijderen van de gebruiker!! De gebruiker is niet verwijderd!");
							echo('<a href="'.$_SESSION['huidige_pagina'].'&s=3">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een gebruiker uit de treeview.</a>');
	    			}
	    			else {
			  			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
								$query = "SELECT inlognaam FROM gebruiker WHERE Werknem_ID = '". $_GET['c'] ."'";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								
								echo('U heeft "'. $row[0] .'" geselecteerd:<br>');
								
								//kijken of de gebruiker niet de ingelogde persoon is							
								if ($_GET['c'] != $_SESSION['gebr_id']) {
									
									//controleren of er verwijzingen naar deze gebruiker zijn, 
									//wanneer deze er zijn dan (in verband met de historie) deze gebruiker niet verwijderen
									$query = "SELECT COUNT(Meld_Lijst_ID) FROM melding_lijst WHERE Gemeld_Door = '".$_GET['c']."' OR Behandeld_Door = '".$_GET['c']."'";
									$resultaat = mysql_query($query);
									$row = mysql_fetch_row($resultaat);
									if ($row[0] == 0) {
										//controleren of er verwijzingen naar deze gebruiker zijn, 
										//wanneer deze er zijn dan (in verband met de historie) deze gebruiker niet verwijderen
										$query = "SELECT COUNT(Comp_Lijst_ID) FROM comp_lijst WHERE Comp_verantwoordelijke = '".$_GET['c']."'";
										$resultaat = mysql_query($query);
										$row = mysql_fetch_row($resultaat);
										if ($row[0] == 0) {
											//controleren of er verwijzingen naar deze gebruiker zijn, 
											//wanneer deze er zijn dan (in verband met de historie) deze gebruiker niet verwijderen
											$query = "SELECT COUNT(Comp_Type) FROM comp_type WHERE Aangemaakt_Door = '".$_GET['c']."' OR Type_Verantwoordelijke = '".$_GET['c']."'";
											$resultaat = mysql_query($query);
											$row = mysql_fetch_row($resultaat);
											if ($row[0] == 0) {
									
												//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
												?>
										    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
										    		<table>
										    			<tr><td><input type="hidden" name="gebruiker" value="<?php echo($_GET['c']);?>">Weet u zeker dat u deze gebruiker verwijderen wilt?</td></tr>
										    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil deze gebruiker verwijderen</td></tr>
										    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
										    		</table>
										    	</form>
												<?php
											}
											else echo("<br>Er bestaan type componenten, welke naar de geselecteerde gebruiker wijzen!");										
										}
										else echo("<br>Er bestaan componenten, welke naar de geselecteerde gebruiker wijzen!");
									}
									else echo("<br>Er bestaan meldingen, welke naar de geselecteerde gebruiker wijzen!");
								}
								else echo("<br>Dit bent u zelf. U kunt uzelf niet verwijderen!");
							}
							else echo("Er is geen gebruiker geselecteerd om te verwijderen<br>Selecteer hiernaast een gebruiker.");
						}
					?>    	
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	