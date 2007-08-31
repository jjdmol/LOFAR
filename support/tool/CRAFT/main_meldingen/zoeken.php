<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 3;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=5';
	  
		if (isset($_GET['o'])) {
			$_SESSION['type_overzicht'] = $_GET['o'];
		} else if (!isset($_SESSION['type_overzicht'])) $_SESSION['type_overzicht'] = 2;
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');


	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {

			function Datum_Geldigheid($datum, $tijd){
				if (Valideer_Datum($datum) && $datum != '00-00-0000' && Valideer_Tijd($tijd) && $tijd != '00:00')
					return true;
				else return false;
			}

	  	?>
	  	<div id="linkerdeel">
	  		<div id="boom_knoppen_container">
		  		<div id="boom_schakel_knop">
		  			<?php
		  				if ($_SESSION['type_overzicht'] == '1')
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=2\">Geef type overzicht weer</a>");
							else
				  			echo("<a href=\"".$_SESSION['huidige_pagina']. "&o=1\">Geef comp. overzicht weer</a>");
		  			?>
					</div>
	  		</div>
	  		<?php	
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree.js\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_items.php\"></script>");
					echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_tpl.js\"></script>");
	  		?>
				<script language="JavaScript">
				<!--//
					
					//functie om de invoer in het textveld te wissen wanneer er een select waarde gekozen wordt
					function ResetTextveld(selectveld, textveld)
					{
						if (document.getElementById(selectveld).value != -1 )
							document.getElementById(textveld).value = '';	
					}

					//functie om het selectveld te wijzigen naar NONE SELECTED wanneer er iets in het textveld ingevoerd wordt
					function ResetSelectveld(textveld, selectveld)
					{
						if (document.getElementById(textveld).value != '' )
							document.getElementById(selectveld).value = -1;	
					}
				
		 			new tree (TREE_ITEMS, TREE_TPL);
	   		//-->
				</script> 
			
			</div>
	    <div id="rechterdeel">

	    	<h2>Meldingen zoeken</h2>
	    	<?php
	    	
	    		function Valideer_Zoeken() {
	    			if (!(isset($_POST['opslaan'])) || $_POST['opslaan'] != 1)
	    				return false;

	  				//meld_datum
	  				if (isset($_POST['Meld_Datum1']) && isset($_POST['Meld_Datum2'])) {
	  					if ($_POST['Meld_Datum1'] != '00-00-0000' || $_POST['Meld_Datum2'] != '00-00-0000' || $_POST['Meld_Tijd1'] != '00:00' || $_POST['Meld_Tijd2'] != '00:00') {
								if (Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum1'], $_POST['Meld_Tijd1']) > Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum2'], $_POST['Meld_Tijd2'])) return false;
								if (Datum_Geldigheid($_POST['Meld_Datum1'], $_POST['Meld_Tijd1']) == false ) return false;
								if (Datum_Geldigheid($_POST['Meld_Datum2'], $_POST['Meld_Tijd2']) == false ) return false;
	  					}
	  				}
	    			
	    			return true;
	    		}
	    	
	    	
	    	
	    		if(Valideer_Zoeken()) {

	    			//basisquery: selecteert alle meldingen... de criteria wordt hierachter geplakt
	  				$query = "SELECT * FROM melding_lijst WHERE Meld_Lijst_ID > 1";

						//type melding
	  				if(isset($_POST['type_melding']) && $_POST['type_melding'] != -1)
							$query = $query . " AND Meld_Type_ID = '".$_POST['type_melding']."'";

						//Component
	  				if(isset($_POST['Component']) && $_POST['Component'] != -1)
							$query = $query . " AND Comp_Lijst_ID = '".$_POST['Component']."'";

	  				//Meldingsdatum
	  				if (isset($_POST['Meld_Datum1']) && isset($_POST['Meld_Tijd1']) && isset($_POST['Meld_Datum2']) && isset($_POST['Meld_Tijd2']) &&
	  					 Datum_Geldigheid($_POST['Meld_Datum1'], $_POST['Meld_Tijd1']) && Datum_Geldigheid($_POST['Meld_Datum2'], $_POST['Meld_Tijd2'])) {
							$query = $query . " AND Meld_Datum >= '".Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum1'], $_POST['Meld_Tijd1'])."' AND Meld_Datum <= '".Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum2'], $_POST['Meld_Tijd2'])."'";	
	  				}

						//Gemeld door
						if (isset($_POST['Gemeld1']) && $_POST['Gemeld2'] == -1 )
							$query = $query . " AND Gemeld_Door IN (SELECT Werknem_ID FROM gebruiker WHERE inlognaam LIKE '%".$_POST['Gemeld1']."%')";
						else if ($_POST['Gemeld2'] > 0)
							$query = $query . " AND Gemeld_Door = '".$_POST['Gemeld2']."'";

						//Status
						if (isset($_POST['Status1']) && $_POST['Status2'] == -1 )
							$query = $query . " AND Huidige_Status IN (SELECT Status_ID FROM status WHERE Status LIKE '%".$_POST['Status1']."%')";
						else if ($_POST['Status2'] > 0)
							$query = $query . " AND Huidige_Status = '".$_POST['Status2']."'";

						//Beschrijving
						if(isset($_POST['beschrijving']) && $_POST['beschrijving'] != ''){
							$query = $query . " AND Prob_Beschrijving LIKE '%".$_POST['beschrijving']."%'";
						}
						
						//Oplossing
						if(isset($_POST['oplossing']) && $_POST['oplossing'] != ''){
							$query = $query . " AND Prob_Oplossing LIKE '%".$_POST['oplossing']."%'";
						}

						//behandeld door
						if (isset($_POST['Behandeld1']) && $_POST['Behandeld2'] == -1 )
							$query = $query . " AND Behandeld_Door IN (SELECT Werknem_ID FROM gebruiker WHERE inlognaam LIKE '%".$_POST['Behandeld1']."%')";
						else if ($_POST['Behandeld2'] > 0)
							$query = $query . " AND Behandeld_Door = '".$_POST['Behandeld2']."'";

						//afgehandeld
						if (isset($_POST['afgehandeld']) && $_POST['afgehandeld'] != -1 ){
							$query = $query . " AND Afgehandeld = '".$_POST['afgehandeld']."'";							
						}

						//locatie
						if (isset($_POST['Locatie1']) && $_POST['Locatie2'] == -1 )
							$query = $query . " AND Melding_Locatie IN (SELECT Locatie_ID FROM comp_locatie WHERE Loc_Naam LIKE '%".$_POST['Locatie1']."%')";
						else if ($_POST['Locatie2'] > 0)
							$query = $query . " AND Melding_Locatie = '".$_POST['Locatie2']."'";


	  				//de samengestelde query uitvoeren
	  			  $resultaat = mysql_query($query);
				  	echo("Met de (door u) ingevoerde criteria zijn de volgende meldingen gevonden:<br><br>");
				  	
				  	//de resultaten weergeven
				  	echo("<table border=\"1\">");
				  	while ($data = mysql_fetch_array($resultaat)) {
							$quer = "SELECT Comp_Naam FROM comp_lijst WHERE Comp_Lijst_ID = '". $data['Comp_Lijst_ID'] ."'";
		  			  $rest = mysql_query($quer);
							$row = mysql_fetch_array($rest);
							
							echo("<tr><td>".$data['Meld_Lijst_ID']."</td><td>".substr($data['Prob_Beschrijving'],0,40)."...</td><td>".$row['Comp_Naam']."</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/melding_info.php?c=".$data['Meld_Lijst_ID']."  \" target=\"_blank\">Info</a></td></tr>\n");
				  	}
				  	echo("</table><br><a href=".$_SESSION['huidige_pagina'].">Klik hier om terug te keren naar het zoekscherm</a>");
	    		}
	    		else {
	    	?>
	    	Vul hieronder 1 of meerdere zoekcriteria in.<br>Laat de velden waar u niet op wilt zoeken blanco.
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
		    	<table>
		    		<tr>
							<td>Type melding:</td>
							<td><select name="type_melding"><?php
			    				if (isset($_POST['type_melding']))
			    					$selected = $_POST['type_melding'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT Meld_Type_ID, Melding_Type_Naam FROM melding_type";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Meld_Type_ID']."\"");
										if($selected == $data['Meld_Type_ID']) echo(" SELECTED");
										echo(">".$data['Melding_Type_Naam']."</option>");
									}
		    				?></select></td>
						</tr>
		    		<tr>
							<td>Component:</td>
							<td><select name="Component">
								<?php
			    				if (isset($_POST['Component'])) 
			    					$selected = $_POST['Component'];
			    				else $selected = -1;
			    				//Type ophalen uit gebruikersgroeprechten
			    				$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '".$_SESSION['groep_id']."'";
			    			  $resultaat = mysql_query($query);
									$data = mysql_fetch_array($resultaat);
									echo("<option value=\"-1\"");
									if ($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
									Vul_Componenten_Select_Box(Bepaal_Types(), $selected);
								?></select>
							</td>
						</tr>
		    		<tr>
							<td>Gemeld:</td>
		    			<td>
		    				tussen&nbsp<input name="Meld_Datum1" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum1'])) echo($_POST['Meld_Datum1']); else echo("00-00-0000"); ?>">
    					  <input name="Meld_Tijd1" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd1'])) echo($_POST['Meld_Tijd1']); else echo("00:00"); ?>">&nbspen&nbsp
			    			<input name="Meld_Datum2" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum2'])) echo($_POST['Meld_Datum2']); else echo("00-00-0000"); ?>">
    					  <input name="Meld_Tijd2" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd2'])) echo($_POST['Meld_Tijd2']); else echo("00:00"); ?>">
							</td>
						</tr>
		    		<tr>
							<td>Gemeld door:</td>
		    			<td><input type="text" name="Gemeld1" id="Gemeld1" value="<?php if (isset($_POST['Gemeld1'])) echo($_POST['Gemeld1']); ?>" onchange="ResetSelectveld('Gemeld1', 'Gemeld2');">&nbspof&nbsp
		    				<select name="Gemeld2" id="Gemeld2" onchange="ResetTextveld('Gemeld2', 'Gemeld1');">
		    				<?php
			    				if (isset($_POST['Leverancier2']))
			    					$selected = $_POST['Leverancier2'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if ($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)){
										echo("<option value=\"".$data['Werknem_ID']."\"");
										if ($selected == $data['Werknem_ID']) echo(" SELECTED");
										echo(">".$data['inlognaam']."</option>");
									}
		    				?></select>
		    			</td>
						</tr>
		    		<tr>
							<td>Status</td>
		    			<td><input type="text" name="Status1" id="Status1" value="<?php if (isset($_POST['Status1'])) echo($_POST['Status1']); ?>" onchange="ResetSelectveld('Status1', 'Status2');">&nbspof&nbsp
		    				<select name="Status2" id="Status2" onchange="ResetTextveld('Status2', 'Status1');">
		    				<?php
			    				if (isset($_POST['Status2']))
			    					$selected = $_POST['Status2'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if ($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT Status_ID, Status FROM status";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Status_ID']."\"");
										if ($selected == $data['Status_ID']) echo(" SELECTED");
										echo(">".$data['Status']."</option>");
									}
		    				?></select>
		    			</td>
						</tr>
		    		<tr>
							<td>Beschrijving:</td>
							<td><textarea name="beschrijving"><?php if (isset($_POST['beschrijving'])) echo($_POST['beschrijving']); ?></textarea></td>
						</tr>
		    		<tr>
							<td>Oplossing:</td>
							<td><textarea name="oplossing"><?php if (isset($_POST['oplossing'])) echo($_POST['oplossing']); ?></textarea></td>
						</tr>
		    		<tr>
							<td>Behandeld door:</td>
		    			<td><input type="text" name="Behandeld1" id="Behandeld1" value="<?php if (isset($_POST['Behandeld1'])) echo($_POST['Behandeld1']); ?>" onchange="ResetSelectveld('Behandeld1', 'Behandeld2');">&nbspof&nbsp
		    				<select name="Behandeld2" id="Behandeld2" onchange="ResetTextveld('Behandeld2', 'Behandeld1');">
		    				<?php
			    				if (isset($_POST['Behandeld2']))
			    					$selected = $_POST['Behandeld2'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if ($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT Werknem_ID, inlognaam FROM gebruiker";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Werknem_ID']."\"");
										if ($selected == $data['Werknem_ID']) echo(" SELECTED");
										echo(">".$data['inlognaam']."</option>");
									}
		    				?></select>
		    			</td>
						</tr>
		    		<tr>
							<td>Afgehandeld</td>
							<td>
								<select name="afgehandeld">
									<?php 
										if(isset($_POST['afgehandeld'])) $selected = $_POST['afgehandeld'];
										else $selected = -1;
									?>
									<option value="-1" <?php if($selected == -1) echo("SELECTED"); ?>>None selected</option>
									<option value="1" <?php if($selected == 1) echo(" SELECTED"); ?>>Ja</option>
									<option value="0" <?php if($selected == 0) echo(" SELECTED"); ?>>Nee</option>
								</select></td>
						</tr>
		    		<tr>
		    			<td>Locatie:</td>
		    			<td><input type="text" name="Locatie1" id="Locatie1" value="<?php if (isset($_POST['Locatie1'])) echo($_POST['Locatie1']); ?>" onchange="ResetSelectveld('Locatie1', 'Locatie2');">&nbspof&nbsp
		    				<select name="Locatie2" onchange="ResetTextveld('Locatie2', 'Locatie1');">
		    				<?php
			    				if (isset($_POST['Locatie2']))
			    					$selected = $_POST['Locatie2'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if ($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT Locatie_ID, Loc_Naam FROM comp_locatie";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Locatie_ID']."\"");
										if ($selected == $data['Locatie_ID']) echo(" SELECTED");
										echo(">".$data['Loc_Naam']."</option>");
									}
		    				?></select>
		    			</td>
		    		</tr>
		    		<tr>
		    			<td><input type="hidden" name="opslaan" value="1"></td>
							<td><a href="javascript:document.theForm.submit();">Zoeken</a>
		    		</tr>
					</table>
				</form>
		    <?php
		  		}
		    ?>
	    </div>	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?> 