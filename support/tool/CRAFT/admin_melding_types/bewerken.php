  <?php
  if(isset($_SESSION['admin_deel'])) {
	
		$_SESSION['admin_deel'] = 3;
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
	    	<h2>Type melding bewerken</h2>
				<?php
	    		
	    		function valideer_invoer() {
						if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
							return false;
	
						//het controleren van de naam
						if (isset($_POST['type_naam'])) {
							if ($_POST['type_naam'] == '')
								return false;
						} else return false;
	
						//het controleren van de aanwezigheid van een beschrijving
						if (isset($_POST['type_beschrijving']) && $_POST['type_beschrijving'] =='')
							return false;
	
	    			return true;
	    		}
	    		
	    		//controle op ingevoerde waardes uitvoeren om zodoende erachter te komen of er opgeslagen mag worden.
	    		if (valideer_invoer()) {
						$query = "UPDATE melding_type SET Melding_Type_Naam = '". htmlentities($_POST['type_naam'], ENT_QUOTES) ."', Huidige_Status = '". $_POST['type_status'] ."', Algemene_Melding='";
						if (isset($_POST['type_algemeen']) && $_POST['type_algemeen'] == 'on') 
							$query = $query . "1', ";
						else $query = $query . "0', ";
						$query = $query . "Stand_Beschrijving='". htmlentities($_POST['type_beschrijving'], ENT_QUOTES) ."', Stand_Oplossing='". htmlentities($_POST['type_oplossing'], ENT_QUOTES) ."' WHERE Meld_Type_ID = '" . $_GET['c'] . "'";
						
						if (mysql_query($query)) echo("Het gewijzigde type melding \"". $_POST['type_naam'] ."\" is in het systeem bijgewerkt<br>");
						else("Er is iets mis gegaan met het opslaan van het type melding\"". $_POST['type_naam'] ."\"!! Het type melding is niet bijgewerkt!");
						echo('<a href="'.$_SESSION['huidige_pagina'].'&c='.$_GET['c']. '">Klik hier om terug te keren naar het vorige type melding of selecteer links een type melding uit de treeview.</a>');
	
	    		}
	    		else {
		    		if (isset($_GET['c']) && $_GET['c'] != 0 ) {
							$query = "SELECT * FROM melding_type WHERE Meld_Type_ID ='". $_GET['c'] ."'";
							$resultaat = mysql_query($query);
					  	$row = mysql_fetch_array($resultaat);
					?>
			    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>&c=<?php echo($_GET['c']); ?>">
			    		<table>
			    			<tr>
			    				<td>Type ID:</td>
			    				<td><?php echo($row['Meld_Type_ID']); ?></td>
			    			</tr>
			    			<tr>
			    				<td>Type naam:</td>
			    				<td><input name="type_naam" type="text" value="<?php if (isset($_POST['type_naam'])) echo(htmlentities($_POST['type_naam'], ENT_QUOTES)); else echo($row['Melding_Type_Naam']); ?>">
					    			<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['type_naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>');?></td>
			    			</tr>
			    			<tr>
			    				<td>Huidige status:</td>
			    				<td><select name="type_status">
										<?php
											$query2 = "SELECT Status_ID, Status FROM status";
											$result = mysql_query($query2);
	
											if (isset($_POST['type_status'])) $selectie = $_POST['type_status'];
								  		else $selectie = $row['Huidige_Status'];
	
											while ($data = mysql_fetch_array($result)) {
												echo("<option value=\"". $data['Status_ID'] ."\"");
												if ($data['Status_ID'] == $selectie) echo(" SELECTED");
												echo(">". $data['Status'] ."</option>\r\n");
											}
										?>
			    				</select></td>
			    			</tr>
			    			<tr>
			    				<td>Algemene melding:</td>
			    				<td>
		    						<?php 
					    				echo('<input id="type_algemeen" name="type_algemeen" type="checkbox" ');
					    				if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1) {
					    					if(isset($_POST['type_algemeen']) && ($_POST['type_algemeen'] == 1 || $_POST['type_algemeen'] == 'on')) 
					    					echo('CHECKED');
					    				}
					    				else if ($row['Algemene_Melding'] == 1) echo('CHECKED');
					    				echo('>');
					    			?></td>
			    			</tr>
			    			<tr>
			    				<td>Standaard omschrijving:</td>
			    				<td><textarea name="type_beschrijving" rows="5" cols="35"><?php if (isset($_POST['type_beschrijving'])) echo(htmlentities($_POST['type_beschrijving'], ENT_QUOTES)); else echo($row['Stand_Beschrijving']); ?></textarea>
				    			<?php if(isset($_POST['opslaan']) && $_POST['opslaan'] == 1 && $_POST['type_beschrijving'] == '') echo('<b id="type_naam">* Er is geen naam ingevoerd!</b>');?></td>
	
			    			</tr>
			    			<tr>
			    				<td>Standaard oplossing:</td>
			    				<td><textarea name="type_oplossing" rows="5" cols="35"><?php if (isset($_POST['type_oplossing'])) echo(htmlentities($_POST['type_oplossing'], ENT_QUOTES)); else echo($row['Stand_Oplossing']); ?></textarea></td>
			    			</tr>
			    			<?php 
			    			
			    				$query = "SELECT COUNT(Kolom_ID) FROM Type_Melding_Koppel_Extra WHERE Meld_Type_ID = '". $_GET['c']."'";
									$resultaat = mysql_query($query);
									$data = mysql_fetch_row($resultaat);
									
									if ($data[0] > 0) {
			    				
				    				?>
					    			<tr>
					    				<td>Extra velden:</td>
						  				<td><iframe id="frame_extra_velden" name="frame_extra_velden" align="middle" marginwidth="0" marginheight="0" src="<?php echo($_SESSION['pagina']); ?>admin_melding_types/type_melding_extra_velden.php?c=<?php echo($_GET['c']); ?>" width="400" height="100" ALLOWTRANSPARENCY frameborder="0" scrolling="auto"></iframe></td>
					    			</tr>
			    					<?php
			    				}
			    			?>
				    		<tr>
				    			<td><input name="opslaan" type="hidden" value="1"></td>
				    			<td><a href="javascript:document.theForm.submit();">Opslaan</a></td>
			    			</tr>	    		
			    		</table>
			    	</form>
		
						<?php
			    		}
							else echo('Er is geen type melding geselecteerd om te wijzigen.<br>Selecteer hiernaast een type melding.'); 
						}
	    	?>
					
	    </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>      	