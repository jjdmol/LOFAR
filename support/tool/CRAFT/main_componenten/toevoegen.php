<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 2;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=2';
	  
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

	    	<h2>Componenten toevoegen</h2>
	    		<form>
	    			<table>
	    				<tr>
	    					<td>Component type:</td>
	    					<td></td>
	    				</tr>
	    			</table>
	    		</form>
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
