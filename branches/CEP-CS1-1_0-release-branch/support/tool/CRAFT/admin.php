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
				echo('<script language="JavaScript">');
				echo("changeSubmenu(". $_GET['p'] . ",". $_GET['p'] . ",1)");
				echo("</script>");   	

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
     		//(instanties van) meldingen
     		else if ($pagina == 4) {
 					//bewerken van meldingen
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_meldingen/bewerken.php');
					//verwijderen van meldingen
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_meldingen/verwijderen.php');    			
    			//toevoegen van meldingen (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_meldingen/toevoegen.php');   			
    		}   	
    		//extra velden
    		else if ($pagina == 5) {
 					//bewerken van extra velden
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_extra_velden/bewerken.php');
					//verwijderen van extra velden
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_extra_velden/verwijderen.php');
    			//toevoegen van extra velden (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_extra_velden/toevoegen.php');   			
    		}
    		//gebruikersgroepen
    		else if ($pagina == 6) {
 					//bewerken van gebruikersgroepen
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_gebruikersgroep/bewerken.php');
					//verwijderen van gebruikersgroepen
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_gebruikersgroep/verwijderen.php');
					//versturen van een groepsmail
					else if (isset($_GET['s']) && $_GET['s'] == 4)
						include($_SESSION['pagina'] . 'admin_gebruikersgroep/groepsmail.php');
    			//toevoegen van gebruikersgroepen (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_gebruikersgroep/toevoegen.php');   			
    		}    		
    		//gebruikers
    		else if ($pagina == 7) {
 					//bewerken van gebruikers
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_gebruikers/bewerken.php');
					//verwijderen van gebruikers
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_gebruikers/verwijderen.php');
					//mailen van gebruikers
					else if (isset($_GET['s']) && $_GET['s'] == 4)
						include($_SESSION['pagina'] . 'admin_gebruikers/mailen.php');
    			//toevoegen van gebruikers (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_gebruikers/toevoegen.php');   			
    		}
    		//(externe) contacten
    		else if ($pagina == 8) {
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
    		//locaties
    		else if ($pagina == 9) {
 					//bewerken van locaties
					if (isset($_GET['s']) && $_GET['s'] == 2)
						include($_SESSION['pagina'] . 'admin_locaties/bewerken.php');
					//locaties van contacten
					else if (isset($_GET['s']) && $_GET['s'] == 3)
						include($_SESSION['pagina'] . 'admin_locaties/verwijderen.php');    			
    			//toevoegen van locaties (standaard actie)
    			else 
						include($_SESSION['pagina'] . 'admin_locaties/toevoegen.php');   			
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