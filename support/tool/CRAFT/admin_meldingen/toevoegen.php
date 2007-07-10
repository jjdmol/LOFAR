  <?php
	
	$_SESSION['admin_deel'] = 4;
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
    	<h2>Meldingen toevoegen</h2>

    	<form>
    		<table>
    			<tr>
    				<td>Type melding:</td>
    				<td><select></select></td>
    			</tr>
    			<tr>
    				<td>Component van invloed:</td>
    				<td><select></select></td>
    			</tr>
    			<tr>
    				<td>Gemeld door:</td>
    				<td><select></select></td>
    			</tr>
    			<tr>
    				<td>Melddatum:</td>
    				<td><input type="text"><input type="text"></td>
    			</tr>
    			<tr>
    				<td>Prob. beschrijving</td>
    				<td><textarea></textarea></td>
    			</tr>
    			<tr>
    				<td>Prob oplossing</td>
    				<td><textarea></textarea></td>
    			</tr>
    			<tr>
    				<td>Behandeld door:</td>
    				<td><select></select></td>
    			</tr>
    			<tr>
    				<td>Afgehandeld:</td>
    				<td><input type="checkbox"></td>
    			</tr>

    		</table>
    	</form>


	  </div>

<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?> 