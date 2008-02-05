<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=4';
	    
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

	    	<h2>Meldingen</h2>
					<?php
						$query = "SELECT count(Meld_Lijst_ID), count(distinct(Comp_Lijst_ID)) FROM melding_lijst";
					  $res = mysql_query($query);
						$row = mysql_fetch_array($res);
						echo("Momenteel zijn er " . $row[0] . " meldingen in het systeem opgeslagen.<br>");
						echo("Deze meldingen zijn verdeeld over " . $row[1] . " componenten (gemiddeld " . ($row[0] / $row[1]) .  " meldingen per component)<br>");
//<br><br>
//Meest actieve persoon qua meldingen inbrengen 
//Datum / Tijd waarin de meeste meldingen gemeld worden.

					?>
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
