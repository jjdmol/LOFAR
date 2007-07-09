  	<?php
	
	$_SESSION['admin_deel'] = 6;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
  
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
    	
    	<h2>Gebruikers toevoegen</h2>
    		<form>
    			<table>
    				<tr>
    					<td>Gebruikersnaam:</td>
    					<td><input name="Gebr_Naam" type="text"></td>
    				</tr>
    				<tr>
    					<td>Groep:</td>
    					<td><select name="Gebr_Groep"></select></td>
    				</tr>
    				<tr>
    					<td>Wachtwoord:</td>
    					<td><input name="Wachtwoord" type="password"></td>
    				</tr>
    				<tr>
    					<td>E-mailadres:</td>
    					<td><input name="Gebr_Email" type="text"></td>
    				</tr>
    				<tr>
    					<td>Gebruikerstaal:</td>
    					<td><select name="Gebr_Taal"></select></td>
    				</tr>
    				<tr>
    					<td>Algemene startpagina:</td>
    					<td><select name="Alg_Start"></select></td>
    				</tr>
    				<tr>
    					<td>Componenten startpagina:</td>
    					<td><select></select></td>
    				</tr>
    				<tr>
    					<td>Meldingen startpagina:</td>
    					<td><select></select></td>
    				</tr>
    				<tr>
    					<td>Statistieken startpagina:</td>
    					<td><select></select></td>
    				</tr>
    				<tr>
    					<td>Intro scherm zichtbaar:</td>
    					<td><input type="checkbox"></td>
    				</tr>
    				<tr>
    					<td>Componentenscherm zichtbaar:</td>
    					<td><input type="checkbox"></td>
    				</tr>
    				<tr>
    					<td>Meldingenscherm zichtbaar:</td>
    					<td><input type="checkbox"></td>
    				</tr>
    				<tr>
    					<td>Statistiekenscherm zichtbaar:</td>
    					<td><input type="checkbox"></td>
    				</tr>
    				<tr>
    					<td>Instellingenscherm zichtbaar:</td>
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