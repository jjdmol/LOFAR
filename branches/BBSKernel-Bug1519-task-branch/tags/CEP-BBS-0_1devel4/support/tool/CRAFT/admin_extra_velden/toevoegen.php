<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 5;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
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
  	<h2>Extra velden toevoegen</h2>
    	
    	<?php
    	
				function type_Controle() {
					//1	Geheel getal (integer)
  				if ($_POST['datatype'] == 1)
						return is_numeric($_POST['waarde1']);
					//2	Getal met decimalen (double)
  				else if ($_POST['datatype'] == 2)
						return is_numeric($_POST['waarde1']);
					//3	Text veld
  				else if ($_POST['datatype'] == 3)
  					return true;
					//4	Datum/tijd veld (datetime)
  				else if ($_POST['datatype'] == 4)
	  				return (Valideer_Datum($_POST['waarde1']) && Valideer_Tijd($_POST['waarde2']));
					//5	Bestandsverwijzing
  				else if ($_POST['datatype'] == 5)
  					return true;
				}    	
    	
    		function Valideer_Invoer() {
					//contact naam
					if (isset($_POST['veldnaam'])) {
						if ($_POST['veldnaam'] == '')
							return false;
					} else return false;
    			
    			//ingevulde waardes controle
   				return type_Controle();
    			
    			return true;
    		}
    	
    		if(Valideer_Invoer()) {
    			//extra_velden / datatabel / type_comp_koppel_extra / type_melding_koppel_extra
    			//integer
    			if ($_POST['datatype'] == 1) 		  $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST['waarde1']."')";
    			//double
    			else if ($_POST['datatype'] == 2) $query = "INSERT INTO datatabel (Type_Double) VALUES('".$_POST['waarde1']."')"; 
    			//text
    			else if ($_POST['datatype'] == 3) $query = "INSERT INTO datatabel (Type_Text) VALUES('".$_POST['waarde1']."')"; 
    			//datumtijd
    			else if ($_POST['datatype'] == 4) $query = "INSERT INTO datatabel (Type_DateTime) VALUES('".Datum_Tijd_Naar_DB_Conversie($_POST['waarde1'], $_POST['waarde2'])."')"; 
    			//bestandsverwijzing
    			else if ($_POST['datatype'] == 5) $query = "INSERT INTO datatabel (Type_TinyText) VALUES('-1')"; 
    			
    			$errorlevel = 0;
    			if (mysql_query($query)) {
    				$errorlevel = 1;
    				
    				//extra velden wordt niet uitgevoerd!!!!
    				
    				$Veld_ID = mysql_insert_id();
    				$query = "INSERT INTO extra_velden (Data_Kolom_ID, Aangemaakt_Door, Veld_Naam, DataType, Type_Beschrijving, Tabel_Type, Is_Verplicht)";
    				$query = $query . "VALUES ('".$Veld_ID."', '".$_SESSION['gebr_id'] ."', '".$_POST['veldnaam']."', '".$_POST['datatype']."', '-1', '".$_POST['koppel']."', ";
						//de verplicht checkbox vertalen naar sql taal ;)
						if (isset($_POST['verplicht']) && ($_POST['verplicht'] == 'on' || $_POST['verplicht'] == '1'))
							$query = $query . "'1') ";
						else $query = $query . "'0') ";
   			
	    			if (mysql_query($query)) {
  	  				$errorlevel = 2;
	
	    				$Veld_ID = mysql_insert_id();	  				
  	  				if ($_POST['koppel'] == 1)
  	  					$query = "INSERT INTO type_comp_koppel_extra (Kolom_ID, Comp_Type_ID) VALUES('".$Veld_ID. "', '". $_POST['hidden_component'] ."')";
  	  				else if ($_POST['koppel'] == 2)
  	  					$query = "INSERT INTO type_melding_koppel_extra (Kolom_ID, Meld_Type_ID) VALUES('".$Veld_ID. "', '". $_POST['hidden_component'] ."')";
		    			
		    			if (mysql_query($query)) 
  		  				$errorlevel = 3;
  	  			}
    			}

					if ($errorlevel == 3) echo("Het extra veld \"". $_POST['veldnaam'] ."\" is succesvol aan het systeem toegevoegd!<br>");
					else if ($errorlevel == 0) echo("Het extra veld \"". $_POST['veldnaam'] ."\" kon niet aan het systeem toegevoegd worden!.<br>");
					else if ($errorlevel == 1) echo("Het extra veld \"". $_POST['veldnaam'] ."\" kon niet aan het systeem toegevoegd.<br>Het dataveld is echter wel aangemaakt!<br>");
					else if ($errorlevel == 2) echo("Het extra veld \"". $_POST['veldnaam'] ."\" is aan het systeem toegevoegd.<br>Alleen er is iets foutgegaan met het koppelen van het extra veld!<br>");
					echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een extra veld toe te voegen.</a>');
    		}
    		else {
    	?>
		    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>">
			    	<table>
			    		<tr>
			    			<td>Veldnaam:</td><td><input name="veldnaam" type="text">
		    				  <?php if(isset($_POST['veldnaam']) && $_POST['veldnaam'] == '') echo('<b>* Er is geen veldnaam ingevoerd!</b>'); ?>
		    				</td>
			    		</tr>
			    		<tr>
			    			<td>Datatype:</td>
			    			<td>
			    				<select name="datatype" onchange="switchType();">
			    					<?php
			    						if (isset($_POST['datatype'])) $selected = $_POST['datatype'];
			    						else $selected = 'SELECTED';

				    					echo ('<option value="1"'); 
				    					if ($selected == 1 || $selected == 'SELECTED') {
				    						 echo(' SELECTED');
				    						 $selected = 1;
				    					} 
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
			    					?>
			    				</select>
			    			</td>
			    		</tr>
			    		<tr>
			    			<td>Standaard waarde:</td><td>
			    				<?php
			    					$url = "";
			    					if (isset($_POST['waarde1']))
			    						$url = "&d1=". $_POST['waarde1'];
			    					if (isset($_POST['waarde2']))
			    						$url = $url. "&d2=". $_POST['waarde2'];
			    				?>
			    				<iframe id="frame_waardes" name="frame_waardes" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_extra_velden/datatype_keuze.php?c=<?php echo($selected . $url);  ?>" width="270" height="40" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe>
		    				  <?php if(isset($_POST['waarde1']) && type_Controle() == 0) echo('<b>* Er is een foutieve standaard waarde ingevoerd!</b>'); ?>
								</td>
			    		</tr>
			    		<tr>
			    			<td>Koppelen aan:</td>
			    			<td>
			    				<select name="koppel" id="koppel" onchange="switchDocument();">
			    					<?php 
			    						if (isset($_POST['koppel'])) $selected = $_POST['koppel'];
			    						else $selected = 'SELECTED';
			    						
			    						echo ('<option value="1"');
			    						if ($selected == 1 || $selected == 'SELECTED') { echo(' SELECTED'); $selected = 1;} 
			    						echo ('>Component type</option>');
				    					echo ('<option value="2"');
				    					if ($selected == 2) { echo(' SELECTED'); } 
				    					echo ('>Melding type</option>');
			    					?>
			    				</select>
			    			</td>
			    		</tr>
			    		<tr>
			    			<td>Component:</td>
			  				<?php
			  					if (isset($_POST['hidden_component'])) $component = $_POST['hidden_component'];
			  					else $component = -1;
			  				?>
			  				<td><iframe id="frame_component" name="frame_component" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_extra_velden/type_keuze.php?c=<?php echo($selected . "&p=" . $component);  ?>" width="450" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
			    		</tr>
			    		<tr>
			    			<td>Verplicht:</td>
			    			<td><input name="verplicht" type="checkbox"    				
			    				<?php
				    		  	if(isset($_POST['verplicht']) && ($_POST['verplicht'] == 1 || $_POST['verplicht'] == 'on'))	echo(' CHECKED');
  							?>></td>
			    		</tr>
			    		<tr>
			    			<td>
			    				<input type="hidden" name="hidden_component" id="hidden_component">
			    				<input type="hidden" name="waarde1" id="waarde1" value="">
			    				<input type="hidden" name="waarde2" id="waarde2" value="">
			    			</td>
			    			<td><a href="javascript:submitFunctie();">Toevoegen</a></td>
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