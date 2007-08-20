<?php
	session_start();
  
	$_SESSION['admin_deel'] = 0;  
 	$_SESSION['main_deel'] = $_SESSION['start_tabblad'];
 	$_SESSION['tab'] = $_GET['p'];

  require_once('includes/login_funcs.php');

  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {

  	//het includen van het menu en het hoofdscherm
		include_once($_SESSION['pagina'] . "includes/pagina_top.php");

	?>
  <div id="hoofdscherm">  	    
		<?php  			
				echo('<script language="JavaScript">');
				echo("changeSubmenu(". $_GET['p'] . ",". $_GET['p'] . ",0)");
				echo("</script>");   	

    		//hieronder wordt de te laden pagina bepaalt
    		//TODO controleren of die pagina wel geladen mag worden!!!!!!!
 		  	
 		  	//de P van de versturende ophalen. als deze anders is dan de P waar die heen moet,
 		  	//dan de startpagina openen, ALLEEN NIET WANNEER DEZE AFKOMSTIG IS VAN EEN OVERZICHT!!!
 		  	
 		  	//?p=
 		  	//echo($_SERVER['HTTP_REFERER']);
				$pos  = strripos($_SERVER['HTTP_REFERER'], '?p=');
				$vorige_pagina = substr($_SERVER['HTTP_REFERER'] ,($pos +3), 1);
    		
    		$pagina = $_GET['p'];

				if (isset($_GET['bypass']) && $_GET['bypass'] > 0)
					$start = $_GET['s'];
				else if ($pagina != $vorige_pagina) {
					if($pagina == 2)
						$start = $_SESSION['start_comp'];
					else if($pagina == 3)
						$start = $_SESSION['start_melding'];
					else if($pagina == 4)
						$start = $_SESSION['start_stats'];
    		} else if (isset($_GET['s'])) $start = $_GET['s'];
    		else $start = 1;

    			
    		if ($pagina == 1) include ($_SESSION['pagina'] .'main/start.php');
    		else if ($pagina == 2) {					
					//toevoegen van componenten 
					if ($start == 2)
						include($_SESSION['pagina'] . 'main_componenten/toevoegen.php');
					//bewerken van componenten 
					else if ($start == 3)
						include($_SESSION['pagina'] . 'main_componenten/bewerken.php');
					//verwijderen van componenten
					else if ($start == 4)
						include($_SESSION['pagina'] . 'main_componenten/verwijderen.php');    			
					//zoeken van componenten
					else if ($start == 5)
						include($_SESSION['pagina'] . 'main_componenten/zoeken.php');    			
    			//overzicht van componenten (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'main_componenten/overzicht.php');					
    		}
    		else if ($pagina == 3) { 			
					//toevoegen van meldingen 
					if ($start == 2)
						include($_SESSION['pagina'] . 'main_meldingen/toevoegen.php');
					//bewerken van meldingen 
					else if ($start == 3)
						include($_SESSION['pagina'] . 'main_meldingen/bewerken.php');
					//verwijderen van meldingen
					else if ($start == 4)
						include($_SESSION['pagina'] . 'main_meldingen/verwijderen.php');    			
					//zoeken van meldingen
					else if ($start == 5)
						include($_SESSION['pagina'] . 'main_meldingen/zoeken.php');    			
    			//overzicht van meldingen (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'main_meldingen/overzicht.php');
    		}
    		//statistieken
    		else if ($pagina == 4) {
					//componenten 
					if ($start == 2)
						include($_SESSION['pagina'] . 'main_statistieken/componenten.php');
					//Type Meldingen 
					else if ($start == 3)
						include($_SESSION['pagina'] . 'main_statistieken/type_meldingen.php');
					//meldingen
					else if ($start == 4)
						include($_SESSION['pagina'] . 'main_statistieken/meldingen.php');    			
					//historie
					else if ($start == 5)
						include($_SESSION['pagina'] . 'main_statistieken/historie.php');
    			//Type Componenten
    			else 
						include($_SESSION['pagina'] . 'main_statistieken/type_componenten.php');
    		}
    		else if ($pagina == 5) include ($_SESSION['pagina'] .'main/instellingen.php');
    	?>
	</div> 

	<?
		//het include van het einde van de pagina
		include_once($_SESSION['pagina'] . "includes/pagina_einde.php");    

  }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>
