<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 4;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=1';
	    
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	    <div id="rechterdeel">

	    	<h2>Algemene statistieken:</h2>
	    		<table>
	    			<tr>
	    				<td>Aantal faciliteiten:</td><td></td>
	    			</tr>
	    			<tr>
	    				<td>Gemiddeld aantal componenten per faciliteit:</td><td></td>
	    			</tr>
	    			<tr>
	    				<td>Gemiddeld aantal niveaus per faciliteit:</td><td></td>
	    			</tr>
	    			<tr>
	    				<td></td><td></td>
	    			</tr>
	    		</table>
	    	Meeste 	componenten<br>
				Faciliteit met de minste type componenten<br>
				Aantal meldingen: 
				Gemiddelde aantal meldingen per faciliteit
				
				

	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 
