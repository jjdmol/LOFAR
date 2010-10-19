<?php

	session_start();


  //controleren of er iemand ingelogd is aan de hand van het meegegeven persoon id
  if (isset($_SESSION['gebr_id'])) {
		include_once("../includes/vars.php");
		$pad = "/LOFAR-craft/main/wachtwoord.php"; 
?>

		<html>
			<head></head>
			<body>
				<h2>Wijzigen van het wachtwoord:</h2>			

<?php
		
		function Valideer_Opslaan() {
			if (!(isset($_POST['opslaan']) && $_POST['opslaan'] == 1)) return false;
			
			//controleren of het nieuwe wachtwoord ingevuld en aan elkaar gelijk is.
			if(isset($_POST['nieuw1']) && isset($_POST['nieuw2'])) {
				if(!($_POST['nieuw1'] != '' && $_POST['nieuw2'] != '' && strtolower($_POST['nieuw1']) == strtolower($_POST['nieuw2'])))
					return false;
			}
			else return false;
			
			//controleren of het oude wachtwoord ingevuld is en of deze overeenkomt met t wachtwoord uit de db
			if($_POST['oud'] != '') {
				$query = "SELECT Wachtwoord FROM gebruiker WHERE Werknem_ID ='".$_SESSION['gebr_id']."'";
				$result = mysql_query($query);
				$row = mysql_fetch_array($result);

				$wachtwoord = md5(strtolower($_POST['oud']));

				if($wachtwoord != $row['Wachtwoord']) 
					return false;
			}
			else return false;			
			
			return true;
		}


		if (Valideer_Opslaan()) {
			$query = "UPDATE gebruiker SET Wachtwoord = '".md5(strtolower($_POST['nieuw1']))."' WHERE Werknem_ID ='".$_SESSION['gebr_id']."'";
			$result = mysql_query($query);

			echo("Uw wachtwoord is bijgewerkt.<br>U kunt dit scherm afsluiten.");
		}
		else {
		
?>
			Voer hieronder uw oude wachtwoord en tweemaal uw nieuwe wachtwoord in:
			<form name="bestand" action="<?php echo($pad); ?>" method="post" enctype="multipart/form-data">
				<table>
					<tr>
						<td>Uw oude wachtwoord:</td>
						<td><input name="oud" value="" type="password"></td>
					</tr>
					<tr>
						<td>Uw nieuwe wachtwoord:</td>
						<td><input name="nieuw1" value="" type="password">
						<?php 
						  if (isset($_POST['opslaan']) && isset($_POST['nieuw1']) && ($_POST['nieuw1'] == '' || $_POST['nieuw1'] != $_POST['nieuw2']))
						  	echo("De nieuwe wachtwoorden zijn niet geldig!");
						?>
						</td>
					</tr>
					<tr>
						<td>Nogmaals uw nieuwe wachtwoord:</td>
						<td><input name="nieuw2" value="" type="password">
						<?php 
						  if (isset($_POST['opslaan']) && isset($_POST['nieuw2']) && ($_POST['nieuw2'] == '' || $_POST['nieuw1'] != $_POST['nieuw2']))
						  	echo("De nieuwe wachtwoorden zijn niet geldig!");
						?>
						</td>
					</tr>
					<tr>
						<td><input type="hidden" name="opslaan" value="1"></td>
						<td><a href="javascript:document.bestand.submit();">Bijwerken</a></td>
					</tr>
				</table>
			</form>
<?
		}
		echo("</body>");
		echo("</html>");
	}
?>