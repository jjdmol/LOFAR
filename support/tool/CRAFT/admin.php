<?php
	session_start();
	
	$_SESSION['admin_deel'] = 1;
  
  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
  include_once($_SESSION['pagina'] . 'includes/vars.php');

  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {

  	//het includen van het menu en het hoofdscherm
		include_once($_SESSION['pagina'] . "includes/admin_top.php");
?>

	  <div id="hoofdscherm">    

    	<?php 
    		//hieronder wordt de te laden pagina bepaalt
    		//TODO controleren of die pagina wel geladen mag worden!!!!!!!
    		$pagina = $_GET['p'];
    		
    		//Types componenten
    		if ($pagina == 1) {
					//bewerken van types componenten 
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_component_types/bewerken.php');
					//verwijderen van types componenten
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_component_types/verwijderen.php');    			
    			//toevoegen van types componenten (standaard actie)
    			else
						include($_SESSION['pagina'] . 'admin_component_types/toevoegen.php');
    		}
    		//instanties van types componenten
    		else if ($pagina == 2) {
					//bewerken van componenten 
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_componenten/bewerken.php');
					//verwijderen van componenten
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_componenten/verwijderen.php');    			
    			//toevoegen van componenten (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_componenten/toevoegen.php');
    		}
    		//type meldingen
    		else if ($pagina == 3) {
 					//bewerken van type meldingen
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_melding_types/bewerken.php');
					//verwijderen van type meldingen
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_melding_types/verwijderen.php');    			
    			//toevoegen van type meldingen (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_melding_types/toevoegen.php');   			
    		}
     		//else if ($pagina == 4) include ('admin_gebruikers.php');
    		//else if ($pagina == 5) include ('admin_rest.php');
    		//else if ($pagina == 6) include ('admin_rest.php');
    		//(externe) contacten
    		else if ($pagina == 7) {
 					//bewerken van contacten
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_contacten/bewerken.php');
					//verwijderen van contacten
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_contacten/verwijderen.php');    			
    			//toevoegen van contacten (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_contacten/toevoegen.php');   			
    		}
    	?>
	
		</div> 	
	
<?
		//het include van het einde van de pagina
		include_once($_SESSION['pagina'] . "includes/admin_einde.php");    

  }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>