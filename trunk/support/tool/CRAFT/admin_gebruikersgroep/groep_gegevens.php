<html>
	<head></head>
	<body bgcolor="#B3CCE6">
		<?php
			include_once("../includes/vars.php");
			
			function waarde_checkbox($waarde) {
				if (isset($waarde) && $waarde == '1')
					return 'CHECKED';
				else return '';
			}
			
			if (isset($_GET['c']) && $_GET['c'] != 0) {	
			  $query = "SELECT * FROM gebruikers_groepen WHERE Groep_ID = '".$_GET['c']."'";
				$resultaat = mysql_query($query);
				$data = mysql_fetch_array($resultaat);
			?>
				<form name="sTest">
				<table>
					<tr>
						<td>Intro scherm zichtbaar:</td>
						<td><input name="Intro_Zichtbaar" id="Intro_Zichtbaar" type="checkbox" value="<?php echo($data['Intro_Zichtbaar']); ?>"></td>
					</tr>
					<tr>
						<td>Componentscherm zichtbaar:</td>
						<td><input name="Comp_Zichtbaar" id="Comp_Zichtbaar" type="checkbox" value="<?php echo($data['Comp_Zichtbaar']); ?>"></td>
					</tr>
					<tr>
						<td>Meldingscherm zichtbaar:</td>
						<td><input name="Melding_Zichtbaar" id="Melding_Zichtbaar" type="checkbox" value="<?php echo($data['Melding_Zichtbaar']); ?>"></td>
					</tr>
					<tr>
						<td>Statistiekenscherm zichtbaar:</td>
						<td><input name="Stats_Zichtbaar" id="Stats_Zichtbaar" type="checkbox" value="<?php echo($data['Stats_Zichtbaar']); ?>"></td>
					</tr>
					<tr>
						<td>Instellingenscherm zichtbaar:</td>
						<td><input name="Inst_Zichtbaar" id="Inst_Zichtbaar" type="checkbox" value="<?php echo($data['Instel_Zichtbaar']); ?>"></td>
					</tr>
					<tr>
						<td>Toevoegrechten:</td>
						<td><input name="Toevoeg_Rechten" id="Toevoeg_Rechten" type="checkbox" value="<?php echo($data['Toevoegen']); ?>"></td>
					</tr>
					<tr>
						<td>Bewerkenrechten:</td>
						<td><input name="Bewerk_Rechten" id="Bewerk_Rechten" type="checkbox" value="<?php echo($data['Bewerken']); ?>"></td>
					</tr>
					<tr>
						<td>Verwijderrechten:</td>
						<td><input name="Verwijder_Rechten" id="Verwijder_Rechten" type="checkbox" value="<?php echo($data['Verwijderen']); ?>"></td>
					</tr>
				</table>
				</form>
		<?php
			}
		?>
	</body>
</html>