  	<?php
	
	$_SESSION['admin_deel'] = 1;
  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
  
  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	
  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {
  	
  	?>
  	<div id="linkerdeel">
  		<?php 
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
    	
    	<h2>Type component bewerken</h2>
    	
    	<?php 
				
				function Validatie_Opslaan(){		
					if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
						return false;
					
					if (isset($_POST['naam'])) {
						if ($_POST['naam'] == '')
							return false;
					} else return false;
					
					//Minimum niet negatief
					if (isset($_POST['minimum']) && $_POST['minimum'] < 0)
						return false;
					
					//maximum niet groter dan minimum
					if (isset($_POST['minimum']) && isset($_POST['maximum']) && $_POST['maximum'] < $_POST['minimum']) 
						return false;

					//Reserve tussen minimum en maximum										
					if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && 
						($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
							return false;
					
					//controleren of de parent component gewijzigd wordt. 
					//dit mag alleen wanneer er geen childs zijn OF geen aangemaakt componenten
					//$query = 
					
					return true;
				}
				
				//eerst een validatie doen om de ingevoerde gegevens te controleren en te kijken of er opgeslagen mag worden...
				if(Validatie_Opslaan()) {
					$query = "UPDATE comp_type SET Type_Naam = '". $_POST['naam'] ."', Type_Parent = '". $_POST['parent'] ."', Structuur_Entry='";
					if (isset($_POST['entry']) && $_POST['entry'] == 'on') 
						$query = $query . "1', ";
					else $query = $query . "0', ";
					$query = $query . "Min_Aantal='". $_POST['minimum'] ."', Max_Aantal='". $_POST['maximum'] ."', Reserve_Minimum='". $_POST['reserve'] ."', ";
					$query = $query . "Type_Verantwoordelijke='". $_POST['verantwoordelijke'] ."', Geleverd_Door='".$_POST['leverancier']."', Gefabriceerd_Door='".$_POST['fabricant']."'";
					$query = $query . " WHERE Comp_Type = '" . $_GET['c'] . "'";
					
					if (mysql_query($query)) echo("Het gewijzigde type \"". $_POST['naam'] ."\" is in het systeem bijgewerkt<br>");
					else("Er is iets mis gegaan met het opslaan van het type \"". $_POST['naam'] ."\"!! Het type is niet bijgewerkt!");
					echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige type of selecteer links een type uit de treeview.</a>');					
				}
				else {
				
					if (isset($_GET['c']) && $_GET['c'] != 0 ) {
					
						$query = 'SELECT * FROM comp_type WHERE Comp_Type = '. $_GET['c'];
				  	$resultaat = mysql_query($query);  	
				  	$row = mysql_fetch_array($resultaat);
	    	?>
	    	
			    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
				    	<table>
				    		<tr>
				    			<td>Type ID:</td>
				    			<td><?php echo($row['Comp_Type']); ?> </td>
				    		</tr>
				    		<tr>
				    			<td>Naam van het type:</td>
				    			<td><input name="naam" type="text" value="<?php if (isset($_POST['naam'])) echo($_POST['naam']); else echo($row['Type_Naam']) ?>">
				    			<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['naam'] == '') echo('<b id="type_naam">* Er is geen naam ingevoerd!</b>');?></td>
				    		</tr>
				    		<tr>
				    			<td>Parent van het type:</td>
				    			<td><select name="parent">
					    			<?php 
					    				$query = 'SELECT Comp_Type, Type_Naam FROM comp_type';
					    			  $resultaat = mysql_query($query);
					    			  if (isset($_POST['parent'])) $selectie = $_POST['parent'];
					    			  else $selectie = $row['Type_Parent'];
									  	while ($data = mysql_fetch_array($resultaat)) {
									  		if ($data['Comp_Type'] != $_GET['c']) {
										  		echo('<option value="'. $data['Comp_Type'] .'"');
										  		if(isset($selectie) && isset($_GET['c']) &&  $data['Comp_Type'] == $selectie)
										  			echo('SELECTED');
										  		echo('>'. $data['Type_Naam'] .'</option>');
									  		}
									  	}
					    			?></select>
					    		</td>
				    		</tr>
				    		<tr>
				    			<td>Structurele entry:</td>
				    			<td>
				    			<?php 
				    				echo('<input id="entry" name="entry" type="checkbox" ');
				    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
				    					if(isset($_POST['entry']) && ($_POST['entry'] == 1 || $_POST['entry'] == 'on')) 
				    						echo('CHECKED');
				    				}
				    				else if ($row['Structuur_Entry'] == 1) echo('CHECKED');
				    				echo('>');
				    			?></td>
				    		</tr>
				    		<tr><td>Aangemaakt door:</td><td>
				    			<?php
										$_SESSION['comp'] = $_GET['c'];
										$_SESSION['comp_parent'] = $row['Type_Parent'];
										$query2 = 'SELECT inlognaam FROM gebruiker WHERE Werknem_ID = '. $row['Aangemaakt_Door'];
				  					$resultaat2 = mysql_query($query2); 
				  					$row2= mysql_fetch_array($resultaat2);
				    				echo($row2['inlognaam']);
				    			?></td>
				    		</tr>
				    		<tr><td>Aangemaakt op:</td><td><?php echo($row['Aanmaak_Datum']) ?></td><td></td></tr>
				    		<tr>
				    			<td>Gefabriceerd door:</td>
				    			<td><select name="fabricant">						    		
				    				<?php
						    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
					    			  $resultaat = mysql_query($query);
				  						if (isset($_POST['fabricant'])) $selectie = $_POST['fabricant'];
				  						else $selectie = $row['Gefabriceerd_Door'];
											
									  	while ($data = mysql_fetch_array($resultaat)) {
									  		echo('<option value="'. $data['Contact_ID'] .'"');
									  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
									  			echo('SELECTED');
									  		echo('>'. $data['Contact_Naam'] .'</option>');
									  	}
							    	?></select>
							    </td>
				    		</tr>
				    		<tr>
				    			<td>Geleverd door:</td>
				    			<td><select name="leverancier">
						    		<?php
						    			$query = 'SELECT Contact_ID, Contact_Naam FROM contact WHERE Contact_ID > 1';
					    			  $resultaat = mysql_query($query);
				  						if (isset($_POST['leverancier'])) $selectie = $_POST['leverancier'];
				  						else $selectie = $row['Geleverd_Door'];
											
									  	while ($data = mysql_fetch_array($resultaat)) {
									  		echo('<option value="'. $data['Contact_ID'] .'"');
									  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
									  			echo('SELECTED');
									  		echo('>'. $data['Contact_Naam'] .'</option>');
									  	}
							    	?></select>
				    			</td>
				    		</tr>
				    		<tr>
				    			<td>Minimaal aan te maken aantal:</td>
				    			<td><input name="minimum" type="text" value="<?php if(isset($_POST['minimum'])) echo($_POST['minimum']); else echo($row['Min_Aantal']); ?>">
				    			<?php
				    				if (isset($_POST['minimum']) && $_POST['minimum'] < 0)	
				    					echo('<b id="type_minimum">* Er is een negatief aantal ingevoerd.</b>');
				    			?>
				    			</td>
				    		</tr>
				    		<tr>
				    			<td>Maximaal aan te maken aantal:</td>
				    			<td><input name="maximum" type="text" value="<?php if(isset($_POST['maximum'])) echo($_POST['maximum']); else echo($row['Max_Aantal']); ?>">
				    			<?php
				    				if (isset($_POST['minimum']) && isset($_POST['maximum']) && $_POST['maximum'] < $_POST['minimum'])	
				    					echo('<b id="type_maximum">* Het maximum aantal is kleiner dan het minimum aantal.</b>');
				    				?></td>
				    		</tr>
				    		<tr>
				    			<td>Aantal op reserve:</td>
				    			<td><input name="reserve" type="text" value="<?php if(isset($_POST['reserve'])) echo($_POST['reserve']); else echo($row['Reserve_Minimum']); ?>">
				    			<?php
										if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && 
											($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
					    					echo('<b id="type_reserve">* De invoer ('.$_POST['reserve'] .') valt buiten de min. / max. waardes.</b>');
				    				?></td>
				    		</tr>
				    		<tr>
				    			<td>Type verantwoordelijke:</td>
				    			<td><select name="verantwoordelijke">
				    			<?php
										$query2 = 'SELECT Werknem_ID, inlognaam FROM gebruiker';
				  					$resultaat2 = mysql_query($query2); 
								  	if (isset($_POST['verantwoordelijke'])) $selectie = $_POST['verantwoordelijke'];
								  	else $selectie = $row['Type_Verantwoordelijke'];
								  	while ($data = mysql_fetch_array($resultaat2)) {
								  		echo('<option value="'. $data['Werknem_ID'] .'"');
								  		if(isset($selectie) && $data['Werknem_ID'] == $selectie)
								  			echo('SELECTED');
								  		echo('>'. $data['inlognaam'] .'</option>');
								  	}
				    			?></select></td>
				    		</tr>
				    		<tr>
									<td id="opslaan" align="right"><a href="javascript:document.theForm.submit();">Opslaan</a></td>
				    			<td><input id="opslaan" name="opslaan" type="hidden" value="1"></td>
				    		</tr>
				    	</table>
						</form> 		   	
	
			<?php 
				}
				else echo('Er is geen type component geselecteerd om te wijzigen.<br>Selecteer hiernaast een type component.'); 
			}
			?>
    </div>

<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>