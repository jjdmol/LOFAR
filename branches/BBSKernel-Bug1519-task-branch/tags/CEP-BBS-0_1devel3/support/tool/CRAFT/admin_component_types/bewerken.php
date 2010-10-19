 <?php
	
	if (isset($_SESSION['admin_deel'])){
	
		$_SESSION['admin_deel'] = 1;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
	  require_once($_SESSION['pagina'] . 'algemene_functionaliteit/globale_functies.php');
		
	  //controleren of er iemand ingelogd is...
	  if ($LOGGED_IN = user_isloggedin()) {
	  	
	  	?>
	  	<div id="linkerdeel">
	  		<?php 
	  			echo("<script language=\"JavaScript\" src=\"". $_SESSION['pagina'] ."admin_component_types/comp_type_functies.php\"></script>");

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
						if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && ($_POST['reserve'] != 0) &&
							($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
								return false;
						
						return true;
					}
					
					//eerst een validatie doen om de ingevoerde gegevens te controleren en te kijken of er opgeslagen mag worden...
					if(Validatie_Opslaan()) {
						$query = "UPDATE comp_type SET Type_Naam = '". htmlspecialchars($_POST['naam']) ."', Type_Parent = '". $_POST['parent'] ."', Structuur_Entry='";
						if (isset($_POST['entry']) && $_POST['entry'] == 'on') 
							$query = $query . "1', ";
						else $query = $query . "0', ";
						$query = $query . "Min_Aantal='". htmlspecialchars($_POST['minimum']) ."', Max_Aantal='". htmlspecialchars($_POST['maximum']) ."', Reserve_Minimum='". htmlspecialchars($_POST['reserve']) ."', ";
						$query = $query . "Type_Verantwoordelijke='". $_POST['hidden_verantwoordelijke'] ."', Geleverd_Door='".$_POST['leverancier']."', Gefabriceerd_Door='".$_POST['fabricant']."'";
						$query = $query . " WHERE Comp_Type = '" . $_GET['c'] . "'";
						
						if (mysql_query($query)) echo("Het gewijzigde type \"". $_POST['naam'] ."\" is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van het type \"". $_POST['naam'] ."\"!! Het type is niet bijgewerkt!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige type of selecteer links een type uit de treeview.</a>');					
					}
					else {
					
						if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$type_selectie = -1;
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
					    			<td><input name="naam" type="text" value="<?php if (isset($_POST['naam'])) echo(htmlentities($_POST['naam'], ENT_QUOTES)); else echo($row['Type_Naam']) ?>">
					    			<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['naam'] == '') echo('<b id="type_naam">* Er is geen naam ingevoerd!</b>');?></td>
					    		</tr>
					    		<tr>
					    			<td>Parent van het type:</td>
					    			<td>
 						    			<select name="parent" id="parent" onchange="switchDocument(<?php if(isset($_POST['hidden_verantwoordelijke'])) echo($_POST['hidden_verantwoordelijke']); else echo("-1");?>);">
						    			<?php 
						    				//Type ophalen uit gebruikersgroeprechten
						    				$query = "SELECT Comp_Type_ID FROM gebruikersgroeprechten WHERE Groep_ID = '".$_SESSION['groep_id']."'";
						    			  $resultaat = mysql_query($query);
												$data = mysql_fetch_array($resultaat);
						    				
									  		if (isset($_POST['parent'])) $type_selectie = $_POST['parent'];
									  		else if(isset($_GET['c'])) $type_selectie = $row['Type_Parent'];
				
												Vul_Component_Types_Select_Box($data[0], $type_selectie, true);
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
					    			<td><input name="minimum" type="text" value="<?php if(isset($_POST['minimum'])) echo(htmlentities($_POST['minimum'],ENT_QUOTES)); else echo($row['Min_Aantal']); ?>">
					    			<?php
					    				if (isset($_POST['minimum']) && $_POST['minimum'] < 0)	
					    					echo('<b id="type_minimum">* Er is een negatief aantal ingevoerd.</b>');
					    			?>
					    			</td>
					    		</tr>
					    		<tr>
					    			<td>Maximaal aan te maken aantal:</td>
					    			<td><input name="maximum" type="text" value="<?php if(isset($_POST['maximum'])) echo(htmlentities($_POST['maximum'],ENT_QUOTES)); else echo($row['Max_Aantal']); ?>">
					    			<?php
					    				if (isset($_POST['minimum']) && isset($_POST['maximum']) && $_POST['maximum'] < $_POST['minimum'])	
					    					echo('<b id="type_maximum">* Het maximum aantal is kleiner dan het minimum aantal.</b>');
					    				?></td>
					    		</tr>
					    		<tr>
					    			<td>Aantal op reserve:</td>
					    			<td><input name="reserve" type="text" value="<?php if(isset($_POST['reserve'])) echo(htmlentities($_POST['reserve'],ENT_QUOTES)); else echo($row['Reserve_Minimum']); ?>">
					    			<?php
											if (isset($_POST['minimum']) && isset($_POST['maximum']) && isset($_POST['reserve']) && ($_POST['reserve'] != 0) &&
												($_POST['reserve'] > $_POST['maximum'] || $_POST['reserve'] < $_POST['minimum']))
						    					echo('<b id="type_reserve">* De invoer ('.$_POST['reserve'] .') valt buiten de min. / max. waardes.</b>');
					    				?></td>
					    		</tr>
			    				<?php
			    					if (isset($_POST['hidden_verantwoordelijke'])) 
    									$verantwoordelijke = $_POST['hidden_verantwoordelijke'];
    								else $verantwoordelijke = $row['Type_Verantwoordelijke'];
			    				
			    				?>
			    				<tr><td>Verantwoordelijke:</td><td><iframe id="frame_contact" name="frame_contact" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/type_verantwoordelijke.php?c=<?php echo($type_selectie . "&s=" . $verantwoordelijke);?>" width="300" height="26" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td></tr>
				    			<?php 
				    			
				    				$query = "SELECT COUNT(Kolom_ID) FROM Type_Comp_Koppel_Extra WHERE Comp_Type_ID = '". $_GET['c']."'";
										$resultaat = mysql_query($query);
										$data = mysql_fetch_row($resultaat);
										
										if ($data[0] > 0) {
				    				
					    				?>
						    			<tr>
						    				<td>Extra velden:</td>
							  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_component_types/comp_type_extra_velden.php?c=<?php echo($_GET['c']); ?>" width="400" height="100" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
						    			</tr>
				    					<?php
				    				}
				    			?>
					    		<tr>
										<td id="opslaan" align="right"><a href="javascript:submitTypeOpslaan();">Opslaan</a></td>
					    			<td><input name="hidden_verantwoordelijke" id="hidden_verantwoordelijke" type="hidden" value="-1"><input id="opslaan" name="opslaan" type="hidden" value="1"></td>
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
	}
?>