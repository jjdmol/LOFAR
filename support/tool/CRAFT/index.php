<?php
  //het starten van een session  
	session_start();
    
  //login functionaliteit includen
  require_once('includes/login_funcs.php');
   
  //de gebruikers uitloggen (wanneer ze ingelogd zijn)
  if ($LOGGED_IN = user_isloggedin()) {
  	user_logout();
  	$_COOKIE['gebruiker'] = '';
  	unset($LOGGED_IN);
  }
  $feedback = '';
  if (isset($_POST['submit']) && ($_POST['submit'] == 'Login')) {
  	//proberen in te loggen
  	$feedback = user_login();

  	//wanneer er ingelogd is, dan doorsturen naar de ingestelde startpagina
  	if ($feedback == 1) {
			//het bepalen van de root en de map waarin dit bestand staat
			//dit kan dan gebruikt worden voor de verwijzingen welke in dit systeem gebruikt zijn
			$pos  = strripos($_SERVER['PHP_SELF'], '/');
			$_SESSION['pagina'] = '..' . (substr($_SERVER['PHP_SELF'] ,0, $pos) . "/");

			//de startpagina bepalen
			$startpagina = "?p=" . $_SESSION['start_tabblad'];
			if($_SESSION['start_tabblad'] == 2)
				$startpagina = $startpagina . "&s=" . $_SESSION['start_comp'];
			else if($_SESSION['start_tabblad'] == 3)
				$startpagina = $startpagina . "&s=" . $_SESSION['start_melding'];
			else if($_SESSION['start_tabblad'] == 4)
				$startpagina = $startpagina . "&s=" . $_SESSION['start_stats'];
  		
  		
  		header("Location: ". $_SESSION['pagina'] ."main.php" . $startpagina);

  	}
  }
	
	//het begin van de pagina includen (inclusief menu's
	include_once("includes/pagina_top.php");

	$php_self = $_SERVER['PHP_SELF'];
?>
	<div id="hoofdscherm">
		<br><center>
			<h2>LOFAR craft</h2>
			<h3>Inlogscherm</h3></center>

		<center>
			<form name="inloggen" method="post" action="<?php echo($php_self); ?>">
				<table>
					<tr>
						<td>Inlognaam:</td><td><INPUT name="gebruiker"></td>
					</tr>
					<tr>
						<td>Wachtwoord:</td><td><INPUT name="wachtwoord" type="password"></td>
					</tr>
				</table>

				<BUTTON NAME="submit" type="submit" value="Login">Login</BUTTON>
			</form>
		<?php 
			//er is iets fouts gegaan met het inloggen, dus een melding tonen
			if ($feedback != '') echo("<p class=\"foutmelding\">". $feedback. "</p>");
		?>
		
		</center>
	</div>

<?php
	//het einde van de pagina includen
	include_once("includes/pagina_einde.php");    
?>
