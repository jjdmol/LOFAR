<?php

	if (isset($_SESSION['admin_deel'])){
	
		$_SESSION['admin_deel'] = 1;
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
	    	<h2>Type component verwijderen</h2>
	    		
	    		<?php
	    			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
							$query = "DELETE FROM comp_type WHERE Comp_Type = " . $_POST['component'];
							if (mysql_query($query)) echo("Het door u geselecteerde type component is uit het systeem verwijderd.<br>");
							else("Er is iets mis gegaan met het verwijderen van het type!! Het type is niet verwijderd!");
							echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een type uit de treeview.</a>');
	    				
	    				
	    			}
	    			else {
	    			
		    			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
								$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type = '". $_GET['c'] ."'";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								
								echo('U heeft het type "'. $row[0] .'" geselecteerd:<br>');
								
								//eerst kijken of er componenten aangemaakt zijn van dit type
								$query = "SELECT COUNT(Comp_Type_ID) FROM comp_lijst WHERE Comp_Type_ID = '". $_GET['c'] ."' GROUP BY Comp_Type_ID";
								$resultaat = mysql_query($query);
								$row = mysql_fetch_row($resultaat);
								//er zijn geen componenten van dit type.
								if ($row[0] == NULL) {
									
									//kijken of dit component deel uitmaakt van een hierarchie
									$query2 = "SELECT Count(Type_Parent) FROM comp_type c WHERE Type_Parent = ". $_GET['c'] ." GROUP BY Type_Parent";
									$resultaat2 = mysql_query($query2);
									$row2 = mysql_fetch_row($resultaat2);
									//geen childs gevonden, dit is dus het onderste deel van een hierarchie
									//er mag verwijderd worden!!!!!!!!!!!!!!!
									if ($row2[0] == NULL) {
										//FORMPJE MAKEN!!!!!!!!!!!!!!!!!!!!!
										?>
								    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
								    		<table>
								    			<tr><td><input type="hidden" name="component" value="<?php echo($_GET['c']);?>">Weet u zeker dat u dit component verwijderen wilt?</td></tr>
								    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil dit component verwijderen</td></tr>
								    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
								    		</table>
								    	</form>
										<?php
									}
									else echo("Dit type component heeft onderliggende type componenten, welke naar dit type component verwijzen.<br>Hierdoor kan dit type niet verwijderd worden!");
								}
								//wel componenten van dit type, dus stoppen (met een melding)					
								else echo("Er zijn instanties van dit type component aangemaakt.<br>Hierdoor is het niet mogelijk om dit type component te verwijderen!");
							}
							else echo("Er is geen type component geselecteerd om te verwijderen<br>Selecteer hiernaast een type component.");
						}
			    	?>
		    		
		    	
		    </div>
		<?php  
				
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");
	}
?>