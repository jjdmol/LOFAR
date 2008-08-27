<?php
  //toevoegen van de file waarin bekeken wordt of er ingelogd is en dergelijke
  require_once('includes/login_funcs.php');
				
?>

<h2>Instellingen</h2>
<?php
	
	//controle van ingevoerde gegevens
	function Validatie_Opslaan() {
		if(isset($_POST['opslaan']) && $_POST['opslaan'] == 0)
			return false;

		if (isset($_POST['naam'])) {
			if ($_POST['naam'] == '')
				return false;
		} else return false;
			
		if (isset($_POST['email']) && ($_POST['email'] == '' || strpos($_POST['email'], '@') == -1 ))
			return false;

		return true;
	}
	
	//de ingevoerde gegevens valideren en daarna opslaan
	if(Validatie_Opslaan()) {
		//opslaan
		$query = "UPDATE gebruiker SET inlognaam= '". $_POST['naam'] . "', Emailadres='". $_POST['email'] ."', Start_Alg='". $_POST['start'] ."', Start_Comp='". $_POST['Start_Comp'] ."', Gebruiker_Taal='".$_POST['taal']."'
		, Start_Melding='". $_POST['Start_Melding'] ."', Start_Stats='". $_POST['Start_Stats'] ."' WHERE Werknem_ID = '" .$_SESSION['gebr_id']. "'";

		//de "nieuwe" gegevens in de sessievariabelen zetten, zodat deze meteen geladen/bruikbaar zijn
		$_SESSION['gebr_naam']  = $_POST['naam'];
		$_SESSION['gebr_email'] = $_POST['email'];
		$_SESSION['taal']			  = $_POST['taal'];
		$_SESSION['start_tabblad'] = $_POST['start'];
		$_SESSION['start_comp'] 	 = $_POST['Start_Comp'];
		$_SESSION['start_melding'] = $_POST['Start_Melding'];
		$_SESSION['start_stats'] 	 = $_POST['Start_Stats'];

		if (mysql_query($query)) echo("Uw persoons-gegevens zijn gewijzigd<br>");
		else("Er is iets mis gegaan met het opslaan uw gegevens. Probeer het nog een keer.");
		echo('<a href="main.php?p=5">Klik hier om terug te keren naar uw gegevens of selecteer uit het menu een andere optie.</a>');
	}
	else {
		//het ophalen van de gegevens van de gebruiker uit de database om deze te tonen
		$query = "SELECT * FROM gebruiker WHERE Werknem_ID = '".$_SESSION['gebr_id']."'";
		$result = mysql_query($query);
		$row = mysql_fetch_array($result);
?>

<form name="theForm" method="post" action="main.php?p=5">
	<table>
		<tr>
			<td>Werknemer ID:</td>
			<td><?php echo($row['Werknem_ID']) ?></td>
		</tr>
	  <tr>
	 		<td>Inlognaam:</td>
	 		<td><input name="naam" type="text" value="<?php if(isset($_POST['naam'])) echo($_POST['naam']); else echo($row['inlognaam']) ?>"><?php if(isset($_POST['naam']) && $_POST['naam'] == '') echo('<b id="type_naam">* Er is geen inlognaam ingevoerd!</b>'); ?></td>
	 	</tr>
		<tr>
			<td>Wijzig wachtwoord:</td>
			<td><a href="<?php echo($_SESSION['pagina']); ?>main/wachtwoord.php" target="_blank">Klik hier om uw wachtwoord te wijzigen</a></td>
		</tr>
		<tr>
			<td>E-mail adres:</td>
			<td><input name="email" type="text" value="<?php if(isset($_POST['email'])) echo($_POST['email']); else  echo($row['Emailadres']) ?>">
			<?php 
				if(isset($_POST['email']) && ($_POST['email'] == '' || strpos($_POST['email'], '@') < 1 )) echo('<b id="type_email">* Er is geen (geldig) e-mailadres ingevoerd!</b>'); 
			?></td>
		</tr>
		<tr>
			<td>Ingestelde taal:</td>
			<td>
				<select name="taal">
					<option value="1">Nederlands</option>
				</select>
			</td>
		</tr>
		<tr>
			<td>Algemene startpagina:</td>
			<td><select name="start">
				<?php 
					if (isset($_POST['start'])) $startpagina = $_POST['start'];
					else $startpagina = $row['Start_Alg'];
			  ?>
			 	<option value="1" <?php if ($startpagina == 1) echo("SELECTED"); ?>>Start</option>	
				<option value="2" <?php if ($startpagina == 2) echo("SELECTED"); ?>>Componenten</option>	
				<option value="3" <?php if ($startpagina == 3) echo("SELECTED"); ?>>Meldingen</option>	
				<option value="4" <?php if ($startpagina == 4) echo("SELECTED"); ?>>Statistieken</option>	
				<option value="5" <?php if ($startpagina == 5) echo("SELECTED"); ?>>Instellingen</option>	
			</select><b id="melding_start"></b></td>
	  </tr>
		<tr>
			<td>Componenten startpagina:</td>
			<td>
				<select name="Start_Comp">
					<?php 
						if(isset($_POST['Start_Comp'])) $start_comp = $_POST['Start_Comp'];
						else $start_comp = $row['Start_Comp'];
					?>

					<option value="1" <?php if ($start_comp == 1) echo("SELECTED"); ?>>Comp. overzicht</option>
					<?php 
						if ($_SESSION['toevoegen'] == 1) {
							echo('<option value="2"'); 
							if ($start_comp == 2) echo("SELECTED"); 
							echo(">Comp. toevoegen</option>");
						}
						if ($_SESSION['bewerken'] == 1) {
							echo('<option value="3"'); 
							if ($start_comp == 3) echo("SELECTED"); 
							echo(">Comp. bewerken</option>");
						}
						if ($_SESSION['verwijderen'] == 1) {
							echo('<option value="4"'); 
							if ($start_comp == 4) echo("SELECTED"); 
							echo(">Comp. verwijderen</option>");
						}
					?>
					<option value="5" <?php if ($start_comp == 5) echo("SELECTED"); ?>>Comp. zoeken</option>
				</select>
			</td>
		</tr>
		<tr>
			<td>Meldingen startpagina:</td>
			<td>
				<select name="Start_Melding">
					<?php 
						if(isset($_POST['Start_Melding'])) $start_melding = $_POST['Start_Melding'];
						else $start_melding = $row['Start_Melding'];
					?>

					<option value="1" <?php if ($start_melding == 1) echo("SELECTED"); ?>>Melding overzicht</option>
					<?php 
						if ($_SESSION['toevoegen'] == 1) {
							echo('<option value="2"'); 
							if ($start_melding == 2) echo("SELECTED"); 
							echo(">Melding toevoegen</option>");
						}
						if ($_SESSION['bewerken'] == 1) {
							echo('<option value="3"'); 
							if ($start_melding == 3) echo("SELECTED"); 
							echo(">Melding bewerken</option>");
						}
						if ($_SESSION['verwijderen'] == 1) {
							echo('<option value="4"'); 
							if ($start_melding == 4) echo("SELECTED"); 
							echo(">Melding verwijderen</option>");
						}
					?>
					<option value="5" <?php if ($start_melding == 5) echo("SELECTED"); ?>>Melding zoeken</option>
				</select>
			</td>
		</tr>
		<tr>
			<td>Statistieken startpagina:</td>
			<td>
				<select name="Start_Stats">
					<?php 
						if(isset($_POST['Start_Stats'])) $start_stats = $_POST['Start_Stats'];
						else $start_stats = $row['Start_Stats'];
					?>
					<option value="1" <?php if ($start_stats == 1) echo("SELECTED"); ?>>Type componenten</option>
					<option value="2" <?php if ($start_stats == 2) echo("SELECTED"); ?>>Componenten</option>
					<option value="3" <?php if ($start_stats == 3) echo("SELECTED"); ?>>Type meldingen</option>
					<option value="4" <?php if ($start_stats == 4) echo("SELECTED"); ?>>Meldingen</option>
					<option value="5" <?php if ($start_stats == 5) echo("SELECTED"); ?>>Historie</option>
				</select>
			</td>
		</tr>
		<tr>
			<td align="right"><input name="opslaan" type="hidden" value="1"><a href="javascript:document.theForm.submit();">Wijzigen</a></td>
		  <td></td>
		</tr>
	</table>
</form>

<?php } ?>
