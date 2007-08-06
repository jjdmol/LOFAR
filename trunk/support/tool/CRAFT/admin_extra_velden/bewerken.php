<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 5;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');
		include_once($_SESSION['pagina'] . 'includes/datum_tijd_functies.php');

	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	?>

	<div id="linkerdeel">
		<?php 
			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_extra_velden/extra_velden_functies.php\"></script>");

			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree.js\"></script>");
			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_items.php\"></script>");
			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."includes/tree_tpl.js\"></script>");
		?>
		<script language="JavaScript">
		<!--//
 			new tree (TREE_ITEMS, TREE_TPL);
 		//-->
		</script> 
	
	</div>
  <div id="rechterdeel">
  	<h2>Extra velden bewerken</h2>
    	<?php

				function datumveld_Controle() {
  				//onderscheid maken tussen datum en tijd en alleen datum
  				if(strpos($_POST['standaard'] , " ")) {
  					$test=split("-",$datum_string);
  					return (Valideer_Tijd($test[1]) && Valideer_Datum($test[0]));
  				}
  				else return (Valideer_Datum($_POST['standaard']));
				}
				
				
				function Valideer_Invoer() {
					//contact naam
					if (isset($_POST['veldnaam'])) {
						if ($_POST['veldnaam'] == '')
							return false;
					} else return false;

    			//ingevulde waardes controle
    			//controleren van de datum waarde
    			if ($_POST['datatype'] == 4) 
    				return datumveld_Controle();

    			return true;
				}

				
				function Converteer_Datatype_Naar_DBVeld($datatype) {
					//1 = Integer
					if ($datatype == 1) 		 return "Type_Integer";
					//2 = Double
					else if ($datatype == 2) return "Type_Double";
					//3 = Text
					else if ($datatype == 3) return "Type_Text";
					//4 = DateTime
					else if ($datatype == 4) return "Type_DateTime";
					//5 = TinyText
					else if ($datatype == 5) return "Type_TinyText";
					//Een leeg veld teruggeven, hiervan flipt de query dus wordt er gestopt
					else return "";
				}


				//valideren of er opgeslagen moet worden of dat de gegevens weergegeven moeten worden.
				if (Valideer_Invoer()){
					//bijwerken
					//datatype en standaard waarde... in de datatabel bijwerken
					//wanneer het datatype verandert is, dan het oude veld op NULL zetten en het nieuwe een waarde toekennen
					if ($_POST['hidden_type'] != $_POST['datatype'])
						$query = "UPDATE datatabel SET ". Converteer_Datatype_Naar_DBVeld($_POST['hidden_type']) ." = NULL, ". Converteer_Datatype_Naar_DBVeld($_POST['datatype']) ." = '". $_POST['standaard'] ."'";
					// en anders alleen de standaard waarde updaten
					else
						$query = "UPDATE datatabel SET ". Converteer_Datatype_Naar_DBVeld($_POST['datatype']) ." = '"	. $_POST['standaard'] . "'";
					
					$errorlevel = 0;
					//het updaten van de datatabel
					if (mysql_query($query)) {
						$errorlevel  = 1;
						
						//de query voor het updaten van de extra_velden tabel
						$query = "UPDATE extra_velden SET Veld_Naam = '". $_POST['veldnaam'] ."', DataType = '". $_POST['datatype'] ."', Is_Verplicht = ";
						//de verplicht checkbox vertalen naar sql
						if (isset($_POST['verplicht']) && ($_POST['verplicht'] == 'on' || $_POST['verplicht'] == '1'))
							$query = $query . "'1' ";
						else $query = $query . "'0' ";
						$query = $query . " WHERE Kolom_ID = '" . $_GET['c'] . "'";
						
						//het updaten van de extra_velden tabel
						if (mysql_query($query)) {
							$errorlevel  = 2;
						}					
					}

					//foutcode vertalen en een melding genereren voor de gebruiker
					if ($errorlevel == 2) echo("Het extra veld \"". $_POST['veldnaam'] ."\" is succesvol in het systeem bijgewerkt!<br>");
					else if ($errorlevel == 0) echo("Het extra veld \"". $_POST['veldnaam'] ."\" kon niet in het systeem bijgewerkt worden!<br>Er ging iets fout met het bijwerken van de datatabel.<br>");
					else if ($errorlevel == 1) echo("Het extra veld \"". $_POST['veldnaam'] ."\" kon niet in het systeem bijgewerkt worden!<br>Het dataveld is echter wel bijgewerkt!<br>");
					echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige veld of selecteer links een veld uit de treeview.</a>');

				}
				else {
	    		if (isset($_GET['c']) && $_GET['c'] != 0 ) {
						$query = 'SELECT * FROM extra_velden WHERE Kolom_ID = '. $_GET['c'];
				  	$resultaat = mysql_query($query);  	
				  	$row = mysql_fetch_array($resultaat);
		    	
		    	?>
						<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>&c=<?php echo($_GET['c']); ?>">
				    	<table>
				    		<tr>
				    			<td>Veldnaam:</td><td><input name="veldnaam" type="text" value="<?php if(isset($_POST['veldnaam'])) echo(htmlentities($_POST['veldnaam'], ENT_QUOTES)); else echo(htmlentities($row['Veld_Naam'], ENT_QUOTES)); ?>">
			    				  <?php if(isset($_POST['veldnaam']) && $_POST['veldnaam'] == '') echo('<b>* Er is geen veldnaam ingevoerd!</b>'); ?>
			    				</td>
				    		</tr>
				    		<tr>
				    			<td>Datatype:</td>
				    			<td>
				    				<?php 
			    						//datatype alleen veranderen, wanneer er nog geen instanties van dit veld zijn aangemaakt!!!
			    						// dus de aangemaakte instantie van dit veld tellen.. wanneer uitkomst = 0 dan mag het aangepast worden
			    						if ($row['Tabel_Type'] == 1) $query = "SELECT COUNT(Comp_Lijst_ID) FROM comp_koppel_extra WHERE Kolom_ID = '". $_GET['c'] ."'";
			    						else if ($row['Tabel_Type'] == 2) $query = "SELECT COUNT(Meld_Lijst_ID) FROM  melding_koppel_extra WHERE Kolom_ID = '". $_GET['c'] ."'";
			    						
											$resultaat = mysql_query($query);
											$data = mysql_fetch_row($resultaat);
											
											if ($data[0] == 0) {

					    					echo('<select name="datatype">');
				    						if (isset($_POST['datatype'])) $selected = $_POST['datatype'];
				    						else $selected = $row['DataType'];
					    					echo ('<option value="1"'); 
					    					if ($selected == 1) { echo(' SELECTED');} 
					    					echo('>Geheel getal (integer)</option>');
					    					echo ('<option value="2"'); 
					    					if ($selected == 2) { echo(' SELECTED'); } 
					    					echo('>Getal met decimalen (double)</option>');
					    					echo ('<option value="3"'); 
					    					if ($selected == 3) { echo(' SELECTED'); } 
					    					echo('>Text veld</option>');
					    					echo ('<option value="4"'); 
					    					if ($selected == 4) { echo(' SELECTED'); } 
					    					echo('>Datum/tijd veld (datetime)</option>');
					    					echo ('<option value="5"'); 
					    					if ($selected == 5) { echo(' SELECTED'); } 
					    					echo('>Bestandsverwijzing</option>');
				    						echo('</select>');
				    					}
				    					else {
				    						if($row['DataType'] == 1) 		 echo("Geheel getal (integer)");
				    						else if($row['DataType'] == 2) echo("Getal met decimalen (double)");
				    						else if($row['DataType'] == 3) echo("Text veld");
				    						else if($row['DataType'] == 4) echo("Datum/tijd veld (datetime)");
				    						else if($row['DataType'] == 5) echo("Bestandsverwijzing");
				    						else echo("");
				    					}
			    					?>
				    			</td>
				    		</tr>
				    		<tr>
				    			<?php
				    				$query = "SELECT * FROM datatabel WHERE Data_Kolom_ID = '". $row['Data_Kolom_ID'] ."'";
								  	$resultaat = mysql_query($query);
								  	$data = mysql_fetch_array($resultaat);
										if ($row['DataType'] == 1) $waarde = $data['Type_Integer'];
										else if ($row['DataType'] == 2) $waarde = $data['Type_Double'];
										else if ($row['DataType'] == 3) $waarde = $data['Type_Text'];
										else if ($row['DataType'] == 4) $waarde = $data['Type_DateTime'];
										else if ($row['DataType'] == 5) $waarde = $data['Type_TinyText'];
										else $waarde = "";
				    			?>
				    			<td>Standaard waarde:</td><td><input name="standaard" type="text" value="<?php if(isset($_POST['standaard'])) echo(htmlentities($_POST['standaard'], ENT_QUOTES)); else echo(htmlentities($waarde, ENT_QUOTES)); ?>">
		    				  <?php if(isset($_POST['standaard']) && ($_POST['datatype'] == 4) && !datumveld_Controle()) echo('<b>* Er is een foutieve standaard waarde ingevoerd!</b>'); ?></td>
				    		</tr>
				    		<tr>
				    			<td>Koppelen aan:</td>
				    			<td>
				    				<?php
			    						if ($row['Tabel_Type'] == 1) { echo('Component type'); } 
				    					if ($row['Tabel_Type'] == 2) { echo('Melding type'); } 				    				
				    				?>
				    			</td>
				    		</tr>
				    		<tr>
				    			<td>Component:</td><td>
				  				<?php
				  				 
			  						if ($row['Tabel_Type'] == 1)
			  							$query = "SELECT Type_Naam FROM comp_type WHERE Comp_Type IN (SELECT Comp_Type_ID FROM type_comp_koppel_extra WHERE Kolom_ID = '". $_GET['c'] ."')";
			  						else if ($row['Tabel_Type'] == 2)
			  							$query = "SELECT Melding_Type_Naam FROM melding_type WHERE Meld_Type_ID IN (SELECT Meld_Type_ID FROM type_melding_koppel_extra WHERE Kolom_ID = '". $_GET['c'] ."')";
			  					
								  	$resultaat = mysql_query($query);
								  	$data = mysql_fetch_array($resultaat);
								  	echo($data[0]);
				  				?>
				  				</td>
				    		</tr>
				    		<tr>
				    			<td>Verplicht:</td>
				    			<td><input name="verplicht" type="checkbox"    				
				    				<?php
				    		  	if(isset($_POST['opslaan'])) {
				    		  		if (($_POST['verplicht']) && ($_POST['verplicht'] == 1 || $_POST['verplicht'] == 'on'))	echo(' CHECKED');
				    		  	}
				    		  	else if($row['Is_verplicht'] == 1) echo(' CHECKED');
  							?>></td>
				    		</tr>
				    		<tr>
				    			<td>
				    				<input type="hidden" name="hidden_data_kolom" id="hidden_data_kolom" value="<?php echo($row['Data_Kolom_ID']); ?>">
				    				<input type="hidden" name="hidden_type" id="hidden_type" value="<?php echo($row['DataType']); ?>">
				    				<input type="hidden" name="hidden_component" id="hidden_component">
				    				<input id="opslaan" name="opslaan" type="hidden" value="1">
				    			</td>
				    			<td><a href="javascript:document.theForm.submit();">Opslaan</a></td>
				    		</tr>
				    	</table>
				    </form>
			    <?php
	    		}
					else echo('Er is geen contact geselecteerd om te wijzigen.<br>Selecteer hiernaast een contact.');
				}
    	?>
  </div>
	
<?php
	  }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>