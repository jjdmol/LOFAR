<?php
  //toevoegen van de file waarin bekeken wordt of er ingelogd is en dergelijke
  require_once('includes/login_funcs.php');
				
?>

<h2>Instellingen</h2>
<?php
	
	function Validatie_Opslaan() {
		if(isset($_POST['opslaan']) && $_POST['opslaan'] == 0)
			return false;

		if (isset($_POST['naam'])) {
			if ($_POST['naam'] == '')
				return false;
		} else return false;
			
		if (isset($_POST['wachtwoord']) && $_POST['wachtwoord'] == '')
			return false;
			
		if (isset($_POST['email']) && ($_POST['email'] == '' || strpos($_POST['email'], '@') == -1 ))
			return false;

		return true;
	}
	
	//de ingevoerde gegevens valideren en daarna opslaan
	if(Validatie_Opslaan()) {
		//opslaan
		$query = "UPDATE gebruiker SET inlognaam= '". $_POST['naam'] . "', Wachtwoord= '". md5($_POST['wachtwoord']) ."'
		, Emailadres='". $_POST['email'] ."', Start_Alg='". $_POST['start'] ."' WHERE Werknem_ID = '" .$_SESSION['gebr_id']. "'";
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
			<td>Wachtwoord:</td>
			<td><input name="wachtwoord" value="" type="password"><?php if(isset($_POST['wachtwoord']) && $_POST['wachtwoord'] == '') echo('<b id="type_wachtwoord">* Er is geen wachtwoord ingevoerd!</b>'); ?></td>
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
			<td><select></select></td>
		</tr>
		<tr>
			<td>Algemene startpagina:</td>
			<td><select name="start">
				<?php 
					if (isset($_POST['start'])) $startpagina = $_POST['start'];
					else $startpagina = $row['Start_Alg'];
			  ?>
			 	<option value="1" <?php if ($startpagina == 1) echo("SELECTED"); ?>>Intro</option>	
				<option value="2" <?php if ($startpagina == 2) echo("SELECTED"); ?>>Componenten</option>	
				<option value="3" <?php if ($startpagina == 3) echo("SELECTED"); ?>>Meldingen</option>	
				<option value="4" <?php if ($startpagina == 4) echo("SELECTED"); ?>>Statistieken</option>	
				<option value="5" <?php if ($startpagina == 5) echo("SELECTED"); ?>>Instellingen</option>	
			</select><b id="melding_start"></b></td>
	  </tr>
		<tr>
			<td>Componenten startpagina:</td>
			<td><?php echo($row['Start_Comp']) ?></td>
		</tr>
		<tr>
			<td>Meldingen startpagina:</td>
			<td><?php echo($row['Start_Melding']) ?></td>
		</tr>
		<tr>
			<td>Statistieken startpagina:</td>
			<td><?php echo($row['Start_Stats']) ?></td>
		</tr>
		<tr>
			<td align="right"><input name="opslaan" type="hidden" value="1"><a href="javascript:document.theForm.submit();">Wijzigen</a></td>
		  <td></td>
		</tr>
	</table>
</form>

<?php } ?>