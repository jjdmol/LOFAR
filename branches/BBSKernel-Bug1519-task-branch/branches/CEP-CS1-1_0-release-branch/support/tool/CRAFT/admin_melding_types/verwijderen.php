  <?php
	if (isset($_SESSION['admin_deel'])) {
	
		$_SESSION['admin_deel'] = 3;
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
	    	<h2>Type melding verwijderen</h2>
				
				<?php
	   			if (isset($_POST['verwijderen']) && $_POST['verwijderen'] == 1 && isset($_POST['confirmatie']) && $_POST['confirmatie'] == 'on') {
						$query = "DELETE FROM melding_type WHERE Meld_Type_ID = " . $_POST['type_melding'];
						if (mysql_query($query)) echo("Het door u geselecteerde type component is uit het systeem verwijderd.<br>");
						else("Er is iets mis gegaan met het verwijderen van het type!! Het type is niet verwijderd!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om terug te keren naar het verwijderen scherm of selecteer links een type uit de treeview.</a>');
	   			}
	   			else {
	    			if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID = '". $_GET['c'] ."'";
							$resultaat = mysql_query($query);
							$row = mysql_fetch_row($resultaat);
							
							echo('U heeft het type melding "'. $row[0] .'" geselecteerd:<br>');
							
							//eerst kijken of er meldingen aangemaakt zijn van dit type
							$query = "SELECT COUNT(Meld_Lijst_ID) FROM melding_lijst WHERE Meld_Type_ID = '". $_GET['c'] ."' GROUP BY Meld_Lijst_ID";
							$resultaat = mysql_query($query);
							$row = mysql_fetch_row($resultaat);
	
							//er zijn geen meldingen van dit type.
							if ($row[0] == NULL) {
		 					?>
					    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
					    		<table>
					    			<tr><td><input type="hidden" name="type_melding" value="<?php echo($_GET['c']);?>">Weet u zeker dat u dit type melding verwijderen wilt?</td></tr>
					    			<tr><td><input type="CheckBox" name="confirmatie"> Ja, ik wil dit type melding verwijderen</td></tr>
					    			<tr><td><input type="hidden" name="verwijderen" value="1"><a href="javascript:document.theForm.submit();">Verwijderen</a></td></tr>
					    		</table>
					    	</form>
		 					<?php
							}
							//er zijn wel meldingen van dit type
							else echo("Dit type component heeft onderliggende type componenten, welke naar dit type component verwijzen.<br>Hierdoor kan dit type niet verwijderd worden!");
	    			}
						//er is geen melding geselecteerd
						else echo("Er is geen type melding geselecteerd om te verwijderen<br>Selecteer hiernaast een type component.");
					}
				?>
			
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>      	