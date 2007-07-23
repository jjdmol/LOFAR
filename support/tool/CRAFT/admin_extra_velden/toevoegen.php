<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 5;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');
		
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
    	
    		function Valideer_Invoer() {
					//contact naam
					if (isset($_POST['veldnaam'])) {
						if ($_POST['veldnaam'] == '')
							return false;
					} else return false;
    			
    			return true;
    		}
    	
    		if(Valideer_Invoer()) {
    			//extra_velden / datatabel / type_comp_koppel_extra / type_melding_koppel_extra
    			//integer
    			if ($_POST['datatype'] == 1) 		  $query = "INSERT INTO datatabel (Type_Integer) VALUES('".$_POST['standaard']."')";
    			//double
    			else if ($_POST['datatype'] == 2) $query = "INSERT INTO datatabel (Type_Double) VALUES('".$_POST['standaard']."')"; 
    			//text
    			else if ($_POST['datatype'] == 3) $query = "INSERT INTO datatabel (Type_Text) VALUES('".$_POST['standaard']."')"; 
    			//datumtijd
    			else if ($_POST['datatype'] == 4) $query = "INSERT INTO datatabel (Type_DateTime) VALUES('".$_POST['standaard']."')"; 
    			//bestandsverwijzing
    			else if ($_POST['datatype'] == 5) $query = "INSERT INTO datatabel (Type_TinyText) VALUES('".$_POST['standaard']."')"; 
    			
    			$errorlevel = 0;
    			if (mysql_query($query)) {
    				$errorlevel = 1;
    				
    				$Veld_ID = mysql_insert_id();
    				$query = "INSERT INTO extra_velden (Data_Kolom_ID, Aangemaakt_Door, Veld_Naam, DataType, Type_Beschrijving, Is_Verplicht)";
    				$query = $query . "VALUES ('".$Veld_ID."', '".$_SESSION['gebr_id'] ."', '".$_POST['veldnaam']."', '".$_POST['datatype']."', '1', ";
						//de verplicht checkbox vertalen naar sql taal ;)
						if (isset($_POST['verplicht']) && ($_POST['verplicht'] == 'on' || $_POST['verplicht'] == '1'))
							$query = $query . "1') ";
						else $query = $query . "0') ";
	    			
	    			if (mysql_query($query)) {
  	  				$errorlevel = 2;
	
	    				$Veld_ID = mysql_insert_id();	  				
  	  				if ($_POST['koppel'] == 1)
  	  					$query = "INSERT INTO Type_Comp_Koppel_Extra (Kolom_ID, Comp_Type_ID) VALUES('".$Veld_ID. "', '". $_POST['hidden_component'] ."')";
  	  				else if ($_POST['koppel'] == 2)
  	  					$query = "INSERT INTO Type_Melding_Koppel_Extra (Kolom_ID, Meld_Type_ID) VALUES('".$Veld_ID. "', '". $_POST['hidden_component'] ."')";
		    			
		    			if (mysql_query($query)) 
  		  				$errorlevel = 3;
  	  			}
    			}
    			
    			//if $errorlevel == 
    			
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
			    			<td>Standaard waarde:</td><td><input name="standaard" type="text"></td>
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
			    			<td>Datatype:</td>
			    			<td>
			    				<select name="datatype">
			    					<?php
			    						if (isset($_POST['datatype'])) $selected = $_POST['datatype'];
			    						else $selected = 'SELECTED';

				    					echo ('<option value="1"'); 
				    					if ($selected == 1 || $selected == 'SELECTED') { echo(' SELECTED');} 
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
			    			<td>Verplicht:</td>
			    			<td><input name="verplicht" type="checkbox"    				
			    				<?php
				    		  	if(isset($_POST['verplicht']) && ($_POST['verplicht'] == 1 || $_POST['verplicht'] == 'on'))	echo(' CHECKED');
  							?>></td>
			    		</tr>
			    		<tr>
			    			<td><input type="hidden" name="hidden_component" id="hidden_component"></td>
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