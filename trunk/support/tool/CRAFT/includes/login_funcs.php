<?php
  //includen van de database instellingen	
	include_once('vars.php');

	//geheime string gebruikt voor MD5 encryptie om de gegevens 
	//van 1 van de cookies te versleutelen (tegen tampering)
	$geheime_hash_string = 'uxdqbrrppvydqpbgtln3xspyv';
	
	$LOGGED_IN = false;
	unset($LOGGED_IN);
	
	//functie om te controleren of er al ingelogd is...
	function user_isloggedin()
	{
		//het globaal maken van de twee variabelen
		global $geheime_hash_string, $LOGGED_IN;
		
		//controleren of er al stiekem al ingelogd is
		if (isSet($LOGGED_IN)) {
			return $LOGGED_IN;
		}
		
		//controleren of de waardes uit de cookies overeenkomen met de waardes wat het zijn moet
		if (isset($_COOKIE['gebruiker']) && isset($_COOKIE['hash'])) {
			$hash = md5($_COOKIE['gebruiker'] . $geheime_hash_string);
			if ($hash == $_COOKIE['hash'])
				return true;
			else 
				return false;	
		}
		else 
			return false;
	}
	
	//functie welke het inloggen van de gebruiker regelt...
	function user_login() 
	{
		//controleren of er geen inloggegevens missen
		if (!isset($_POST['gebruiker']) || !isset($_POST['wachtwoord'])) {
			$feedback = 'Missende of niet aanwezige inloggegevens';
			return 'Missende of niet aanwezige inloggegevens';
		}
		else 
		{
			//variabelen laden
			$gebruiker = strtolower($_POST['gebruiker']);
			$wachtwoord = strtolower($_POST['wachtwoord']);
			$gecodeerd_wachtwoord = md5($wachtwoord);
			
			//in de database kijken of de ingevoerde gegevens in de db voorkomen. 
			$query = "SELECT * FROM gebruiker WHERE inlognaam = '$gebruiker' AND wachtwoord = '$gecodeerd_wachtwoord'";
			$result = mysql_query($query);
			$row = mysql_fetch_array($result);
			
			//als er niets gevonden is, dan een melding
			if (!$result || mysql_num_rows($result) < 1) 
			  return 'incorrecte gegevens ingevoerd';
			else { //wel wat gevonden dus cookies aanmaken en aangeven dat het inloggen gelukt is.
				if(user_setup($row['inlognaam'])) {
					//als bovenstaande functie goed is uitgevoerd, dan de informatie over de gebruiker opslaan.
					//dit gebeurt in de sessievariabele
					$_SESSION['gebr_id']    = $row['Werknem_ID'];					
					$_SESSION['gebr_naam']  = $row['inlognaam'];
					$_SESSION['gebr_email'] = $row['Emailadres'];
					$_SESSION['pag_start']  = $row['Intro_Zichtbaar'];
					$_SESSION['pag_comp']   = $row['Comp_Zichtbaar'];
					$_SESSION['pag_meld']   = $row['Melding_Zichtbaar'];
					$_SESSION['pag_stats']  = $row['Stats_Zichtbaar'];
					$_SESSION['pag_instel'] = $row['Instel_Zichtbaar'];
					$_SESSION['groep_id']		= $row['Groep_ID'];
					$_SESSION['taal']			  = $row['Gebruiker_Taal'];
					$_SESSION['start_tabblad'] = $row['Start_Alg'];
				}
				
				return 1;
			}
		}
	}
	
	//functie om de gebruiker uit te loggen, oftewel de cookies weggooien
	function user_logout()
	{
		setcookie('gebruiker', '', (time()+2592000), '/', '', 0);
		setcookie('hash', '', (time()+2592000), '/', '', 0);		
		session_unset();
	}
	
	//functie welke de cookies aanmaakt, zodat het systeem weet dat de gebruiker ingelogd is...
	function user_setup($res)
	{
		global $geheime_hash_string;
		//als de meegegeven waarde leeg is, dan false teruggeven en anders de cookies aanmaken
		if ($res=='')
		  return false;
		$user_name = strtolower($res);
		$id_hash = md5($user_name.$geheime_hash_string);
		setcookie('gebruiker', $user_name, (time()+2592000), '/', '', 0);
		setcookie('hash', $id_hash, (time()+2592000), '/', '', 0);				
		return true;
	}

?>