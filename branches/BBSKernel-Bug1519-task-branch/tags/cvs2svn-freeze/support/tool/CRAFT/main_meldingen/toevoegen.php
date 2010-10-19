<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 3;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=2';
	  
		if (isset($_GET['o'])) {
			$_SESSION['type_overzicht'] = $_GET['o'];
		} else if (!isset($_SESSION['type_overzicht'])) $_SESSION['type_overzicht'] = 2;
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
    include_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<div id="boom_knoppen_container">
		  		<div id="boom_schakel_knop">
		  			<?php
		  				if ($_SESSION['type_overzicht'] == '1')
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=2\">Geef type overzicht weer</a>");
							else
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=1\">Geef comp. overzicht weer</a>");
		  			?>
					</div>
	  		</div>
	  		<?php	
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."algemene_functionaliteit/melding_toevoegen_functies.php\"></script>");

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

	    	<h2>Meldingen toevoegen</h2>
				<?php
					include_once($_SESSION['pagina'] . 'algemene_functionaliteit/melding_toevoegen.php');					
				?>
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
