<?php
	session_start();
  
	$_SESSION['admin_deel'] = 0;  
  
  require_once('includes/login_funcs.php');

  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {

  	//het includen van het menu en het hoofdscherm
		include_once($_SESSION['pagina'] . "includes/pagina_top.php");

	?>
  <div id="hoofdscherm">    
  	
    	<?php 
    		//hieronder wordt de te laden pagina bepaalt
    		//TODO controleren of die pagina wel geladen mag worden!!!!!!!
    		$pagina = $_GET['p'];
    		if ($pagina == 1) include ($_SESSION['pagina'] .'main/start.php');
    		else if ($pagina == 2) include ($_SESSION['pagina'] .'main/componenten.php');
    		else if ($pagina == 3) include ($_SESSION['pagina'] .'main/meldingen.php');
    		else if ($pagina == 4) include ($_SESSION['pagina'] .'main/statistieken.php');
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
