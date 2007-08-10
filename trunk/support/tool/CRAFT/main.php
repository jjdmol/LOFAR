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
    		$pagina = $_GET['p'];
    		if ($pagina == 1) include ($_SESSION['pagina'] .'main/start.php');
    		else if ($pagina == 2) {					
					//toevoegen van componenten 
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'main_componenten/toevoegen.php');
					//bewerken van componenten 
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'main_componenten/bewerken.php');
					//verwijderen van componenten
					else if (isset($_GET['s']) && $_GET['s'] == 4)
						include($_SESSION['pagina'] . 'main_componenten/verwijderen.php');    			
					//zoeken van componenten
					else if (isset($_GET['s']) && $_GET['s'] == 5)
						include($_SESSION['pagina'] . 'main_componenten/zoeken.php');    			
    			//overzicht van componenten (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'main_componenten/overzicht.php');					
    		}
    		else if ($pagina == 3) { 			
					//toevoegen van meldingen 
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'main_meldingen/toevoegen.php');
					//bewerken van meldingen 
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'main_meldingen/bewerken.php');
					//verwijderen van meldingen
					else if (isset($_GET['s']) && $_GET['s'] == 4)
						include($_SESSION['pagina'] . 'main_meldingen/verwijderen.php');    			
					//zoeken van meldingen
					else if (isset($_GET['s']) && $_GET['s'] == 5)
						include($_SESSION['pagina'] . 'main_meldingen/zoeken.php');    			
    			//overzicht van meldingen (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'main_meldingen/overzicht.php');
    		}
    		//statistieken
    		else if ($pagina == 4) {
    			//Type Componenten
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'main_statistieken/type_componenten.php');
					//componenten 
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'main_statistieken/componenten.php');
					//Type Meldingen 
					else if (isset($_GET['s']) && $_GET['s'] == 4)
						include($_SESSION['pagina'] . 'main_statistieken/type_meldingen.php');
					//meldingen
					else if (isset($_GET['s']) && $_GET['s'] == 5)
						include($_SESSION['pagina'] . 'main_statistieken/meldingen.php');    			
					//historie
					else if (isset($_GET['s']) && $_GET['s'] == 6)
						include($_SESSION['pagina'] . 'main_statistieken/historie.php');
    			//algemene statistieken
    			else 
						include($_SESSION['pagina'] . 'main_statistieken/algemeen.php');
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
