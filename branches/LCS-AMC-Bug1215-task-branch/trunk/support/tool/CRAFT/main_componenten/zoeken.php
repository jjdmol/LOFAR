<?php	
	if(isset($_SESSION['main_deel'])){
		$_SESSION['main_deel'] = 2;
		$_SESSION['type_overzicht'] = 1;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'main.php?p='.$_SESSION['main_deel'].'&s=5';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');
		
		
		function Datum_Geldigheid($datum, $tijd){
			if ($datum == '00-00-0000' && $tijd == '00:00')
				return false;
			else if (Valideer_Datum($datum) && Valideer_Tijd($tijd))
				return true;
			else return false;
		}
		
		
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."algemene_functionaliteit/comp_toevoegen_functies.php\"></script>");
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
	    	
	    	<h2>Componenten zoeken</h2>
	    	<?php
	    		
	    		function Valideer_Zoeken() {
	    			if (!(isset($_POST['opslaan'])) || $_POST['opslaan'] != 1)
	    				return false;

	  				//Leverdatum
	  				if (isset($_POST['Lever_Datum1']) && isset($_POST['Lever_Datum2'])) {
	  					if ($_POST['Lever_Datum1'] != '00-00-0000' || $_POST['Lever_Datum2'] != '00-00-0000' || $_POST['Lever_Tijd1'] != '00:00' || $_POST['Lever_Tijd2'] != '00:00') {
								if (Datum_Tijd_Naar_DB_Conversie($_POST['Lever_Datum1'], $_POST['Lever_Tijd1']) > Datum_Tijd_Naar_DB_Conversie($_POST['Lever_Datum2'], $_POST['Lever_Tijd2'])) return false;
								if (Datum_Geldigheid($_POST['Lever_Datum1'], $_POST['Lever_Tijd1']) == false ) return false;
								if (Datum_Geldigheid($_POST['Lever_Datum2'], $_POST['Lever_Tijd2']) == false ) return false;
	  					}
	  				}
	  				
	  				//fabricatiedatum
	  				if (isset($_POST['Fabr_Datum1']) && isset($_POST['Fabr_Datum2'])) {
	  					if ($_POST['Fabr_Datum1'] != '00-00-0000' || $_POST['Fabr_Datum2'] != '00-00-0000' || $_POST['Fabr_Tijd1'] != '00:00' || $_POST['Fabr_Tijd2'] != '00:00') {
								if (Datum_Tijd_Naar_DB_Conversie($_POST['Fabr_Datum1'], $_POST['Fabr_Tijd1']) > Datum_Tijd_Naar_DB_Conversie($_POST['Fabr_Datum2'], $_POST['Fabr_Tijd2'])) return false;
								if (Datum_Geldigheid($_POST['Fabr_Datum1'], $_POST['Fabr_Tijd1']) == false ) return false;
								if (Datum_Geldigheid($_POST['Fabr_Datum2'], $_POST['Fabr_Tijd2']) == false ) return false;
	  					}
	  				}
	  				
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
	    			
	    			//basisquery: selecteert alle componenten... de criteria wordt hierachter geplakt
	  				//wanneer er ook op status gezocht moet worden, dan moeten 2 tabellen doorgezocht worden
	  				if(isset($_POST['Status']) && $_POST['Status'] != -1) {
							$query = "SELECT c.* FROM comp_lijst c, melding_lijst m WHERE c.Comp_Lijst_ID > 1";
	  				}
	  				else $query = "SELECT c.* FROM comp_lijst c WHERE c.Comp_Lijst_ID > 1";
	  				
	  				//naam component
	  				if(isset($_POST['naam']))
							$query = $query . " AND c.Comp_Naam LIKE '%".$_POST['naam']."%'";

						//type component
	  				if(isset($_POST['Type']) && $_POST['Type'] != -1)
							$query = $query . " AND c.Comp_Type_ID = '".$_POST['Type']."'";

						//status
	  				if(isset($_POST['Status']) && $_POST['Status'] != -1) {
	  					$query = $query . " AND c.laatste_melding = m.Meld_Lijst_ID AND m.Huidige_Status = '". $_POST['Status'] ."'";
	  				}

						//locatie
						if (isset($_POST['Locatie1']) && $_POST['Locatie2'] == -1 )
							$query = $query . " AND c.Comp_Locatie IN (SELECT Locatie_ID FROM comp_locatie WHERE Loc_Naam LIKE '%".$_POST['Locatie1']."%')";
						else if ($_POST['Locatie2'] > 0)
							$query = $query . " AND c.Comp_Locatie = '".$_POST['Locatie2']."'";

						//parent 
	  				if(isset($_POST['Parent']) && $_POST['Parent'] != -1)
							$query = $query . " AND c.Comp_Parent = '".$_POST['Parent']."'";
						
						//verantwoordelijke voor dit component
						if (isset($_POST['Verantwoordelijke1']) && $_POST['Verantwoordelijke2'] == -1 )
							$query = $query . " AND c.Comp_Verantwoordelijke IN (SELECT Werknem_ID FROM gebruiker WHERE inlognaam LIKE '%".$_POST['Verantwoordelijke1']."%')";
						else if ($_POST['Verantwoordelijke2'] > 0)
							$query = $query . " AND c.Comp_Verantwoordelijke = '".$_POST['Verantwoordelijke2']."'";

						//fabricant voor dit component
						if (isset($_POST['Fabricant1']) && $_POST['Fabricant2'] == -1 )
							$query = $query . " AND c.Contact_Fabricant IN (SELECT Contact_ID FROM contact WHERE Contact_Naam LIKE '%".$_POST['Fabricant1']."%')";
						else if ($_POST['Fabricant2'] > 0)
							$query = $query . " AND c.Contact_Fabricant = '".$_POST['Fabricant2']."'";

						//leverancier voor dit component
						if (isset($_POST['Leverancier1']) && $_POST['Leverancier2'] == -1 )
							$query = $query . " AND c.Contact_Leverancier IN (SELECT Contact_ID FROM contact WHERE Contact_Naam LIKE '%".$_POST['Leverancier1']."%')";
						else if ($_POST['Leverancier2'] > 0)
							$query = $query . " AND c.Contact_Leverancier = '".$_POST['Leverancier2']."'";

	  				//Leverdatum
	  				if (isset($_POST['Lever_Datum1']) && isset($_POST['Lever_Tijd1']) && isset($_POST['Lever_Datum2']) && isset($_POST['Lever_Tijd2']) &&
	  					 Datum_Geldigheid($_POST['Lever_Datum1'], $_POST['Lever_Tijd1']) && Datum_Geldigheid($_POST['Lever_Datum2'], $_POST['Lever_Tijd2']))
							$query = $query . " AND c.Lever_Datum >= '".Datum_Tijd_Naar_DB_Conversie($_POST['Lever_Datum1'], $_POST['Lever_Tijd1'])."' AND c.Lever_Datum <= '".Datum_Tijd_Naar_DB_Conversie($_POST['Lever_Datum2'], $_POST['Lever_Tijd2'])."'";

	  				//Fabricatiedatum
	  				if (isset($_POST['Fabr_Datum1']) && isset($_POST['Fabr_Tijd1']) && isset($_POST['Fabr_Datum2']) && isset($_POST['Fabr_Tijd2']) &&
	  					 Datum_Geldigheid($_POST['Fabr_Datum1'], $_POST['Fabr_Tijd1']) && Datum_Geldigheid($_POST['Fabr_Datum2'], $_POST['Fabr_Tijd2']))
							$query = $query . " AND c.Fabricatie_Datum >= '".Datum_Tijd_Naar_DB_Conversie($_POST['Fabr_Datum1'], $_POST['Fabr_Tijd1'])."' AND c.Fabricatie_Datum <= '".Datum_Tijd_Naar_DB_Conversie($_POST['Fabr_Datum2'], $_POST['Fabr_Tijd2'])."'";

	  				//Aangemaakt:
	  				if (isset($_POST['Meld_Datum1']) && isset($_POST['Meld_Tijd1']) && isset($_POST['Meld_Datum2']) && isset($_POST['Meld_Tijd2']) &&
	  					 Datum_Geldigheid($_POST['Meld_Datum1'], $_POST['Meld_Tijd1']) && Datum_Geldigheid($_POST['Meld_Datum2'], $_POST['Meld_Tijd2'])) {
							$query = $query . " AND c.Comp_Lijst_ID IN (SELECT Comp_Lijst_ID FROM melding_lijst WHERE Voorgaande_Melding = 1 AND Meld_Datum >= '".Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum1'], $_POST['Meld_Tijd1'])."' AND Meld_Datum <= '".Datum_Tijd_Naar_DB_Conversie($_POST['Meld_Datum2'], $_POST['Meld_Tijd2'])."')";	  				
	  				}
						
	  				//de samengestelde query uitvoeren
	  			  $resultaat = mysql_query($query);
				  	echo("Met de (door u) ingevoerde criteria zijn de volgende componenten gevonden:<br><br>");
				  	
				  	//de resultaten weergeven
				  	echo("<table border=\"1\">");
				  	while ($data = mysql_fetch_array($resultaat))
							echo("<tr><td>".$data['Comp_Lijst_ID']."</td><td>".$data['Comp_Naam']."</td><td><a href=\"../".$_SESSION['pagina']."algemene_functionaliteit/comp_beschrijving.php?c=".$data['Comp_Lijst_ID']."  \" target=\"_blank\">Info</a></td></tr>\n");
				  	
				  	echo("</table><br><a href=".$_SESSION['huidige_pagina'].">Klik hier om terug te keren naar het zoekscherm</a>");
	    		}
	    		else {
	    	?>	    	
	    	Vul hieronder 1 of meerdere zoekcriteria in.<br>Laat de velden waar u niet op wilt zoeken blanco.
	    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
		    	<table>
		    		<tr>
		    			<td>Naam:</td>
		    			<td><input type="text" name="naam" id="naam" value="<?php if(isset($_POST['naam'])) echo($_POST['naam']); ?>"></td>
		    		</tr>
		    		<tr>
		    			<td>Type:</td>
		    			<td>
		    				<select name="Type">
								<?php
			    				if (isset($_POST['Type']))
			    					$selected = $_POST['Type'];
			    				else $selected = -1;

			    				//Type ophalen uit gebruikersgroeprechten
			    				$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '".$_SESSION['groep_id']."'";
			    			  $resultaat = mysql_query($query);
									$data = mysql_fetch_array($resultaat);
									echo("<option value=\"-1\"");
									if($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
									Vul_Component_Types_Select_Box($data[0], $selected, false);
								?></select>
		    			</td>
		    		</tr>
		    		<tr>
		    			<td>Status:</td>
		    			<td>
		    				<select name="Status">
		    				<?php
			    				if (isset($_POST['Status']))
			    					$selected = $_POST['Status'];
			    				else $selected = -1;

									echo("<option value=\"-1\"");
									if($selected == -1) echo(" SELECTED");
									echo(">None selected</option>");
			    				$query = "SELECT * FROM status WHERE Status_ID";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Status_ID']."\"");
										if ($selected == $data['Status_ID']) echo(" SELECTED");
										echo(">".$data['Status']."</option>");
							  	}									
								?>
								</select>
		    			</td>
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
									if($selected == -1) echo(" SELECTED");									
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
		    			<td>Parent:</td>
		    			<td>
		    				<select name="Parent">
								<?php
			    				if (isset($_POST['Parent'])) 
			    					$selected = $_POST['Parent'];
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
		    			<td>Verantwoordelijke:</td>
		    			<td><input type="text" name="Verantwoordelijke1" id="Verantwoordelijke1" value="<?php if (isset($_POST['Verantwoordelijke1'])) echo($_POST['Verantwoordelijke1']); ?>" onchange="ResetSelectveld('Verantwoordelijke1', 'Verantwoordelijke2');">&nbspof&nbsp
		    				<select name="Verantwoordelijke2" id="Verantwoordelijke2" onchange="ResetTextveld('Verantwoordelijke2', 'Verantwoordelijke1');">
		    				<?php
			    				if (isset($_POST['Verantwoordelijke2']))
			    					$selected = $_POST['Verantwoordelijke2'];
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
		    			<td>Fabricant:</td>
		    			<td><input type="text" name="Fabricant1" id="Fabricant1" value="<?php if (isset($_POST['Fabricant1'])) echo($_POST['Fabricant1']); ?>" onchange="ResetSelectveld('Fabricant1', 'Fabricant2');">&nbspof&nbsp
		    				<select name="Fabricant2" id="Fabricant2" onchange="ResetTextveld('Fabricant2', 'Fabricant1');">
		    				<?php
			    				if (isset($_POST['Fabricant2']))
			    					$selected = $_POST['Fabricant2'];
			    				else $selected = -1;
			    				
									echo("<option value=\"-1\" SELECTED>None selected</option>");
			    				$query = "SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Contact_ID']."\"");
										if ($selected == $data['Contact_ID']) echo(" SELECTED");
										echo(">".$data['Contact_Naam']."</option>");
									}
		    				?></select>
		    			</td>
		    		</tr>
		    		<tr>
		    			<td>Leverancier:</td>
		    			<td><input type="text" name="Leverancier1" id="Leverancier1" value="<?php if (isset($_POST['Leverancier1'])) echo($_POST['Leverancier1']); ?>" onchange="ResetSelectveld('Leverancier1', 'Leverancier2');">&nbspof&nbsp
		    				<select name="Leverancier2" id="Leverancier2" onchange="ResetTextveld('Leverancier2', 'Leverancier1');">
		    				<?php
			    				if (isset($_POST['Leverancier2']))
			    					$selected = $_POST['Leverancier2'];
			    				else $selected = -1;

									echo("<option value=\"-1\" SELECTED>None selected</option>");
			    				$query = "SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1";
			    			  $resultaat = mysql_query($query);
							  	while ($data = mysql_fetch_array($resultaat)) {
										echo("<option value=\"".$data['Contact_ID']."\"");
										if ($selected == $data['Contact_ID']) echo(" SELECTED");
										echo(">".$data['Contact_Naam']."</option>");
									}
		    				?></select>
		    			</td>
		    		</tr>
		    		<tr>
		    			<td>Leverdatum:</td>
		    			<td>
		    				tussen&nbsp<input name="Lever_Datum1" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Lever_Datum1'])) echo($_POST['Lever_Datum1']); else echo("00-00-0000"); ?>">
    					  <input name="Lever_Tijd1" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Lever_Tijd1'])) echo($_POST['Lever_Tijd1']); else echo("00:00"); ?>">&nbspen&nbsp
			    			<input name="Lever_Datum2" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Lever_Datum2'])) echo($_POST['Lever_Datum2']); else echo("00-00-0000"); ?>">
    					  <input name="Lever_Tijd2" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Lever_Tijd2'])) echo($_POST['Lever_Tijd2']); else echo("00:00"); ?>">
							</td>
		    		</tr>
		    		<tr>
		    			<td>Fabricatiedatum:</td>
		    			<td>
		    				tussen&nbsp<input name="Fabr_Datum1" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Fabr_Datum1'])) echo($_POST['Fabr_Datum1']); else echo("00-00-0000"); ?>">
    					  <input name="Fabr_Tijd1" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Fabr_Tijd1'])) echo($_POST['Fabr_Tijd1']); else echo("00:00"); ?>">&nbspen&nbsp
			    			<input name="Fabr_Datum2" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Fabr_Datum2'])) echo($_POST['Fabr_Datum2']); else echo("00-00-0000"); ?>">
    					  <input name="Fabr_Tijd2" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Fabr_Tijd2'])) echo($_POST['Fabr_Tijd2']); else echo("00:00"); ?>">
							</td>
		    		</tr>
		    		<tr>
		    			<td>Aangemaakt:</td>
		    			<td>
		    				tussen&nbsp<input name="Meld_Datum1" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum1'])) echo($_POST['Meld_Datum1']); else echo("00-00-0000"); ?>">
    					  <input name="Meld_Tijd1" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd1'])) echo($_POST['Meld_Tijd1']); else echo("00:00"); ?>">&nbspen&nbsp
			    			<input name="Meld_Datum2" type="text" size="8" maxlength="10" value="<?php if(isset($_POST['Meld_Datum2'])) echo($_POST['Meld_Datum2']); else echo("00-00-0000"); ?>">
    					  <input name="Meld_Tijd2" type="text" size="2" maxlength="5" value="<?php if(isset($_POST['Meld_Tijd2'])) echo($_POST['Meld_Tijd2']); else echo("00:00"); ?>">
							</td>
		    		</tr>
		    		<tr>
		    			<td><input type="hidden" name="opslaan" value="1"></td>
							<td><a href="javascript:document.theForm.submit();">Zoeken</a></td>
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