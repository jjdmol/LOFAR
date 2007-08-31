<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 8;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=2';
	  
	  require_once($_SESSION['pagina'] . 'includes/login_funcs.php');
		include_once($_SESSION['pagina'] . 'includes/controle_functies.php');
	
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
	    	<h2>Extern contact bewerken</h2>
				<?php
					
					function Validatie_Opslaan() {
						if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
							return false;
						
						//contact naam
						if (isset($_POST['Contact_Naam'])) {
							if ($_POST['Contact_Naam'] == '')
								return false;
						} else return false;
	    			
	    			if (isset($_POST['parent_gegevens']) && $_POST['Contact_Parent'] == 1)
	    				return false; 
	    			
	    			//adres veld 1
						if (isset($_POST['Contact_Adres1'])) {
							if ($_POST['Contact_Adres1'] == '' && !isset($_POST['parent_gegevens']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	
	    			//woonplaats
						if (isset($_POST['Contact_Woonplaats'])) {
							if ($_POST['Contact_Woonplaats'] == '' && !isset($_POST['parent_gegevens']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	
	    			//postcode
						if (isset($_POST['Contact_Postcode'])) {
							if ($_POST['Contact_Postcode'] != '' && !postcode_check($_POST['Contact_Postcode']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
/*	
	    			//e-mail
						if (isset($_POST['Contact_Email'])) {
							if ($_POST['Contact_Email'] != '' && !mail_check($_POST['Contact_Email']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	   			
	
						if (isset($_POST['Contact_Telefoon_Vast'])) {
							if ($_POST['Contact_Telefoon_Vast'] != '' && !telefoon_check($_POST['Contact_Telefoon_Vast']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	
	
						if (isset($_POST['Contact_Telefoon_Mobiel'])) {
							if ($_POST['Contact_Telefoon_Mobiel'] != '' && !telefoon_check($_POST['Contact_Telefoon_Mobiel']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	
	    			//fax nummer (deze is niet verplicht, maar als deze ingevoerd is, dan toch controleren
						if (isset($_POST['Contact_Telefoon_Fax'])) {
							if ($_POST['Contact_Telefoon_Fax'] != '' && !telefoon_check($_POST['Contact_Telefoon_Fax']))
								return false;
						}
	*/
	    			return true;
					}
					
					if(Validatie_Opslaan()) {
						$query = "UPDATE contact SET Contact_Naam = '". htmlentities($_POST['Contact_Naam'], ENT_QUOTES) ."', Contact_Adres1 = '". htmlentities($_POST['Contact_Adres1'], ENT_QUOTES) ."', Contact_Adres2='". htmlentities($_POST['Contact_Adres2'], ENT_QUOTES) ."'";
						$query = $query . ", Contact_Postcode='". htmlentities($_POST['Contact_Postcode'], ENT_QUOTES) ."', Contact_Woonplaats = '". htmlentities($_POST['Contact_Woonplaats'], ENT_QUOTES) ."', Contact_Telefoon_Vast='". htmlentities($_POST['Contact_Telefoon_Vast'], ENT_QUOTES) ."'";
						$query = $query . ", Contact_Telefoon_Mobiel = '". htmlentities($_POST['Contact_Telefoon_Mobiel'], ENT_QUOTES) ."', Contact_Email='". htmlentities($_POST['Contact_Email'], ENT_QUOTES) ."', Contact_Fax = '". htmlentities($_POST['Contact_Telefoon_Fax'], ENT_QUOTES) ."'";
						$query = $query . ", Contact_Parent = '". $_POST['Contact_Parent'] ."', Contact_Functie='". $_POST['Contact_Functie'] ."', Contact_Parent_Gegevens=";
						if (isset($_POST['parent_gegevens']) && $_POST['parent_gegevens'] == 'on') 
							$query = $query . "'1'";
						else $query = $query . "'0'";
						$query = $query . " WHERE Contact_ID = '" . $_GET['c'] . "'";
						
						if (mysql_query($query)) echo("Het gewijzigde contact \"". $_POST['Contact_Naam'] ."\" is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van het contact \"". $_POST['Contact_Naam'] ."\"!! Het contact is niet bijgewerkt!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige contact of selecteer links een contact uit de treeview.</a>');
	
					}
					else {
						if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = 'SELECT * FROM contact WHERE Contact_ID = '. $_GET['c'];
					  	$resultaat = mysql_query($query);  	
					  	$row = mysql_fetch_array($resultaat);
					  	
					  	if (isset($_POST['opslaan']) && $_POST['opslaan'] == 1 )
					  	{
					  		if (isset($_POST['parent_gegevens']))
					  			$parent_gegevens = 1;
					  		else $parent_gegevens = 0;
					  	}
					  	else 
					  		$parent_gegevens = $row['Contact_Parent_Gegevens'];
					?>
	
				    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']); ?>&c=<?php echo($_GET['c']); ?>">
					    	<table>
					    		<tr>
					    			<td>Naam:</td>
					    			<td><input name="Contact_Naam" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Naam'], ENT_QUOTES)); else echo($row['Contact_Naam']); ?>">
						    			<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['Contact_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>');?></td>
					    		</tr>
									<tr>
										<td>Parent van dit contact:</td>
										<td><select name="Contact_Parent">
											<?php
						    				$query = 'SELECT Contact_ID, Contact_Naam FROM contact';
						    			  $result = mysql_query($query);
									  		if (isset($_POST['parent'])) $selectie = $_POST['parent'];
									  		else $selectie = $row['Contact_Parent'];
												
										  	while ($data = mysql_fetch_array($result)) {
										  		if ($data['Contact_ID'] != $row['Contact_ID']) {
											  		echo('<option value="'. $data['Contact_ID'] .'"');
											  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
											  			echo('SELECTED');
											  		echo('>'. $data['Contact_Naam'] .'</option>');
										  		}
										  	}
											?>
											</select>
						    		</td>
									</tr>
					    			<td>Functie:</td>
					    			<td><input name="Contact_Functie" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Functie'], ENT_QUOTES)); else echo($row['Contact_Functie']); ?>"></td>
					    		</tr>
									<tr>
										<td>Gegevens van parent overnemen:</td>
										<td><input name="parent_gegevens" type="checkbox" <?php if($parent_gegevens == 1) echo('CHECKED'); ?>></td>
									</tr>
					    		<tr>
						   		<tr>
					    			<td>Adres veld 1:</td>
					    			<td><input name="Contact_Adres1" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Adres1'], ENT_QUOTES)); else echo($row['Contact_Adres1']); ?>">
					    				<?php
					    					if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $parent_gegevens == 0 && $_POST['Contact_Adres1'] == '')
					    						echo('<b>* Er is geen adres ingevoerd!</b>');
					    				 ?>
										</td>
					    		</tr>
					    		<tr>
					    			<td>Adres veld 2:</td>
					    			<td><input name="Contact_Adres2" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Adres2'], ENT_QUOTES)); else echo($row['Contact_Adres2']); ?>"></td>
					    		</tr>
					    		<tr>
					    			<td>Postcode:</td>
					    			<td><input name="Contact_Postcode" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Postcode'], ENT_QUOTES)); else echo($row['Contact_Postcode']); ?>">
					    				<?php
					    					if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) { 
					    						if (($parent_gegevens == 1 && $_POST['Contact_Postcode'] != '' && !postcode_check($_POST['Contact_Postcode'])) ||
					    							($parent_gegevens == 0 && ($_POST['Contact_Postcode'] == '' || !postcode_check($_POST['Contact_Postcode']))))
					    						 		echo('<b>* Er is geen (geldige) postcode ingevoerd!</b>');
					    					}
					    				 ?>
				    				</td>
					    		</tr>
					    		<tr>
					    			<td>Woonplaats:</td>
					    			<td><input name="Contact_Woonplaats" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Woonplaats'], ENT_QUOTES)); else echo($row['Contact_Woonplaats']); ?>">
					    				<?php
					    					if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $parent_gegevens == 0 && $_POST['Contact_Woonplaats'] == '')
					    						echo('<b>* Er is geen woonplaats ingevoerd!</b>');
					    				 ?>
					    			</td>
					    		</tr>
					    		<tr>
					    			<td>E-mail:</td>
					    			<td><input name="Contact_Email" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Email'], ENT_QUOTES)); else echo($row['Contact_Email']); ?>">
					    				<?php
					 /*   					if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) { 
					    						if (($parent_gegevens == 1 && $_POST['Contact_Email'] != '' && !mail_check($_POST['Contact_Email'])) ||
					    							($parent_gegevens == 0 && ($_POST['Contact_Email'] == '' || !mail_check($_POST['Contact_Email']))))
					    						 		echo('<b>* Er is geen (geldig) e-mailadres ingevoerd!</b>');
					    					}*/
					    				 ?>
				    				</td>
					    		</tr>
					    		<tr>
					    			<td>Telefoon (vast):</td>
					    			<td><input name="Contact_Telefoon_Vast" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Telefoon_Vast'], ENT_QUOTES)); else echo($row['Contact_Telefoon_Vast']); ?>">
					    				<?php /* if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['Contact_Telefoon_Vast']!= '' && !telefoon_check($_POST['Contact_Telefoon_Vast'])) echo('<b>* Er is geen (geldig) telefoonnummer ingevoerd!</b>'); */ ?>
				    				</td>
					    		</tr>
					    		<tr>
					    			<td>Telefoon (mobiel):</td>
					    			<td><input name="Contact_Telefoon_Mobiel" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Telefoon_Mobiel'], ENT_QUOTES)); else echo($row['Contact_Telefoon_Mobiel']); ?>">
					    				<?php /* if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['Contact_Telefoon_Mobiel']!= '' && !telefoon_check($_POST['Contact_Telefoon_Mobiel'])) echo('<b>* Er is geen (geldig) telefoonnummer ingevoerd!</b>'); */ ?>
				    				</td>
					    		</tr>
					    		<tr>
					    			<td>Telefoon (fax):</td>
					    			<td><input name="Contact_Telefoon_Fax" type="text" value="<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) echo(htmlentities($_POST['Contact_Telefoon_Fax'], ENT_QUOTES)); else echo($row['Contact_Fax']); ?>">
					    				<?php /* if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['Contact_Telefoon_Fax']!= '' && !telefoon_check($_POST['Contact_Telefoon_Fax'])) echo('<b>* Er is geen (geldig) telefoonnummer ingevoerd!</b>'); */ ?>
				   					</td>
					    		</tr>
					    		<tr>
					    			<td><input name="opslaan" type="hidden" value="1"></td>
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