<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 2;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=4';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');
		
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
	    	
	    	<h2>Componenten verwijderen</h2>
	    	<?php
	    		if (isset($_GET['c'])) {
	    			$query = "SELECT * FROM comp_lijst WHERE Comp_Lijst_ID = '".$_GET['c']."'";
				  	$resultaat = mysql_query($query);  	
				  	$row = mysql_fetch_array($resultaat);
				  	echo("U heeft het component \"". $row['Comp_Naam'] ."\" (".$_GET['c'].") geselecteerd<br>");
				  	echo("Om dit component te verwijderen dient u contact op te nemen met de administrator van uw groep<br>");
				  	echo("Ook kunt u contact opnemen met de algemene administrators<br>");
						echo("<br>De administrator(s), welke dit component verwijderen kunnen, zijn:<br>");
						echo("<table>");
						//de groepen ophalen, welke toegang hebben tot dit type component
						$Collectie = Check_groepen($row['Comp_Type_ID']);
						for ($i = 0; $i < Count($Collectie); $i++) {
							$query = "SELECT Groep_ID, Admin_Rechten FROM gebruikers_groepen WHERE Groep_ID = '".$Collectie[$i]."'";
						  $resultaat = mysql_query($query);
					  	$row = mysql_fetch_array($resultaat);
							//kijken of de groep adminrechten heeft
							if ($row['Admin_Rechten'] == 1) {
								$query2 = "SELECT * FROM gebruiker WHERE Groep_ID = '".$row['Groep_ID']."'";
							  $rest = mysql_query($query2);
								//data weergeven
								while ($data = mysql_fetch_array($rest)) {
									echo("<tr><td>" . $data['Werknem_ID'] . "</td><td>". $data['inlognaam'] . "</td><td>" . $data['Emailadres'] . "</td></tr>");
								}
							}
						}
						echo("</table>");
	    		}
	    		else echo("Er is geen component geselecteerd!<br> Selecteer een component uit de boomstructuur.");
	    	?>
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
