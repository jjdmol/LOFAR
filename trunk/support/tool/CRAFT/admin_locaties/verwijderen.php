<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 9;
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
	    	
	    	<h2>Locaties verwijderen</h2>
		
	    		<?php
	    			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
							$query = "DELETE FROM comp_locatie WHERE Locatie_ID = " . $_POST['Loc_ID'];
							if (mysql_query($query)) echo("De door u geselecteerde locatie is uit het systeem verwijderd.<br>");
							else("Er is iets mis gegaan met het verwijderen van de locatie!! De locatie is niet verwijderd!");
							echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een locatie uit de treeview.</a>');
	    			}
	    			else {
		    			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
								$query = "SELECT Loc_Naam FROM comp_locatie WHERE Locatie_ID = '". $_GET['c'] ."'";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								
								echo('U heeft het type "'. $row[0] .'" geselecteerd:<br>');
								
								//eerst kijken of er componenten aangemaakt zijn welke deze locatie gebruiken
								$query = "SELECT COUNT(Comp_Type_ID) FROM comp_lijst WHERE Comp_Locatie = '". $_GET['c'] ."' GROUP BY Comp_Locatie";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								//er zijn geen componenten met deze locatie, dus er mag verwijderd worden
								if ($row[0] == NULL) {
									//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
									?>
							    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>&c=<?php echo($_GET['c']); ?>">
							    		<table>
							    			<tr><td><input type="hidden" name="Loc_ID" value="<?php echo($_GET['c']);?>">Weet u zeker dat u deze locatie wilt verwijderen wilt?</td></tr>
							    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil deze locatie verwijderen</td></tr>
							    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
							    		</table>
							    	</form>
									<?php
								}
								//wel componenten met deze locatie, dus stoppen (met een melding)					
								else echo("Deze locatie wordt gebruikt door een of meerdere componenten.<br>Hierdoor is het niet mogelijk om deze locatie te verwijderen!");
							}
							else echo("Er is geen locatie geselecteerd om te verwijderen<br>Selecteer hiernaast een locatie.");
						}
			    	?>
	    	
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>    	