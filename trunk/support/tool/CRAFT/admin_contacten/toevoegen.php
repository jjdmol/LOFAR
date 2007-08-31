<?php
	if(isset($_SESSION['admin_deel'])){
		$_SESSION['admin_deel'] = 8;
	  $_SESSION['huidige_pagina'] = $_SESSION['pagina'] . 'admin.php?p='.$_SESSION['admin_deel'].'&s=1';
	  
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
	    	<h2>Extern contact toevoegen</h2>
	    	
	    	<?php
	    		
	    		function Valideer_Invoer() {
						if (isset($_POST['opslaan']) && $_POST['opslaan'] == 0) 
							return false;
						
						//contact naam
						if (isset($_POST['Contact_Naam'])) {
							if ($_POST['Contact_Naam'] == '')
								return false;
						} else return false;
	    			
	    			if (isset($_POST['parent_gegevens']) && $_POST['parent'] == 1)
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
	
	    			//e-mail
						if (isset($_POST['Contact_Email'])) {
							if ($_POST['Contact_Email'] != '' && !mail_check($_POST['Contact_Email']))
								return false;
						} else if (!isset($_POST['parent_gegevens'])) return false;
	   			
	/*
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
	    		
	    		//kijken of er opgeslagen mag worden
	    		if (Valideer_Invoer()) {
						if (isset($_POST['parent_gegevens']) && $_POST['parent_gegevens'] == 'on') 
							$parent = 1;
						else $parent = 0;
						
						$query = "INSERT INTO contact (Contact_Naam, Contact_Parent, Contact_Adres1, Contact_Adres2, Contact_Postcode, Contact_Woonplaats, ";
						$query = $query . "Contact_Telefoon_Vast, Contact_Telefoon_Mobiel, Contact_Email, Contact_Fax, Contact_Functie, Contact_Parent_Gegevens) ";
						$query = $query . "VALUES ('". htmlentities($_POST['Contact_Naam'], ENT_QUOTES) ."', '". $_POST['parent'] ."', '". htmlentities($_POST['Contact_Adres1'], ENT_QUOTES) ."' ,'". htmlentities($_POST['Contact_Adres2'], ENT_QUOTES) ."', ";
						$query = $query . "'". htmlentities($_POST['Contact_Postcode'], ENT_QUOTES) ."', '". htmlentities($_POST['Contact_Woonplaats'], ENT_QUOTES) ."', '" . htmlentities($_POST['Contact_Telefoon_Vast'], ENT_QUOTES) ."', '". htmlentities($_POST['Contact_Telefoon_Mobiel'], ENT_QUOTES) ."', ";
						$query = $query . "'". htmlentities($_POST['Contact_Email'], ENT_QUOTES) ."', '". htmlentities($_POST['Contact_Telefoon_Fax'], ENT_QUOTES) ."', '". $_POST['Contact_Functie'] ."', '". $parent ."')";
						
						if (mysql_query($query)) echo("Het nieuwe contact \"". $_POST['Contact_Naam'] ."\" is aan het systeem toegevoegd<br>");
						else echo("Het nieuwe contact \"". $_POST['Contact_Naam'] ."\" kon niet aan het systeem toegevoegd worden!.");
						echo('<a href="'.$_SESSION['huidige_pagina'].'">Klik hier om nog een contact toe te voegen.</a>');
	    			
	    		}
	    		else {
	    	?>
			    	<form name="theForm" method="post" action="<?php echo($_SESSION['huidige_pagina']);?>">
				    	<table>
				    		<tr>
				    			<td>Naam:</td>
				    			<td><input name="Contact_Naam" type="text" value="<?php if(isset($_POST['Contact_Naam'])) echo(htmlentities($_POST['Contact_Naam'], ENT_QUOTES)); ?>">
			    				  <?php if(isset($_POST['Contact_Naam']) && $_POST['Contact_Naam'] == '') echo('<b>* Er is geen naam ingevoerd!</b>'); ?></td>
				    		</tr>
								<tr>
									<td>Parent van dit contact:</td>
									<td><select name="parent">
					    			<?php 
					    				$query = 'SELECT Contact_ID, Contact_Naam FROM contact';
					    			  $resultaat = mysql_query($query);
								  		if (isset($_POST['parent'])) $selectie = $_POST['parent'];
								  		else if(isset($_GET['c'])) $selectie = $_GET['c'];
											
									  	while ($data = mysql_fetch_array($resultaat)) {
									  		echo('<option value="'. $data['Contact_ID'] .'"');
									  		if(isset($selectie) && $data['Contact_ID'] == $selectie)
									  			echo('SELECTED');
									  		echo('>'. $data['Contact_Naam'] .'</option>');
									  	}
					    			?></select>
			    				  <?php if(isset($_POST['parent_gegevens']) && $_POST['parent'] == 1) echo('<b>* Er dienen adresgegevens ingevoerd te worden!</b>'); ?></td>
					    		</td>
								</tr>
				    			<td>Functie:</td>
				    			<td><input name="Contact_Functie" type="text" value="<?php if(isset($_POST['Contact_Functie'])) echo(htmlentities($_POST['Contact_Functie'], ENT_QUOTES)); ?>"></td>
				    		</tr>
								<tr>
									<td>Gegevens van parent overnemen:</td>
									<td><input name="parent_gegevens" type="checkbox" <?php if(isset($_POST['opslaan'])) {if (isset($_POST['parent_gegevens']) && $_POST['parent_gegevens'] == 'on' ) echo("CHECKED"); } else echo("CHECKED"); ?>></td>
								</tr>
				    		<tr>
					   		<tr>
				    			<td>Adres veld 1:</td>
				    			<td><input name="Contact_Adres1" type="text" value="<?php if(isset($_POST['Contact_Adres1'])) echo(htmlentities($_POST['Contact_Adres1'], ENT_QUOTES)); ?>">
			    				  <?php if(!isset($_POST['parent_gegevens']) && isset($_POST['Contact_Adres1']) && $_POST['Contact_Adres1'] == '') echo('<b>* Er is geen adres ingevoerd!</b>'); ?></td>
				    		</tr>
				    		<tr>
				    			<td>Adres veld 2:</td>
				    			<td><input name="Contact_Adres2" type="text" value="<?php if(isset($_POST['Contact_Adres2'])) echo(htmlentities($_POST['Contact_Adres2'], ENT_QUOTES)); ?>"></td>
				    		</tr>
				    		<tr>
				    			<td>Postcode:</td>
				    			<td><input name="Contact_Postcode" type="text" value="<?php if(isset($_POST['Contact_Postcode'])) echo(htmlentities($_POST['Contact_Postcode'], ENT_QUOTES)); ?>">
			    				  <?php 
			    				  	if (isset($_POST['Contact_Postcode']))
			    				  		$postcode = $_POST['Contact_Postcode'];
			    				  	else $postcode = '';
			    				  	
			    				  	if(isset($_POST['opslaan'])) {
												if(isset($_POST['parent_gegevens'])) {
													if ($postcode != '' && !postcode_check($postcode))
				   				  				echo('<b>* Er is geen geldige postcode ingevoerd!</b>');
												}
												else {
													if ($postcode == '' || !postcode_check($postcode))
				   				  				echo('<b>* Er is geen geldige postcode ingevoerd!</b>');
												}
			    				  	}
			    				  ?>
			    				</td>
				    		</tr>
				    		<tr>
				    			<td>Woonplaats:</td>
				    			<td><input name="Contact_Woonplaats" type="text" value="<?php if(isset($_POST['Contact_Woonplaats'])) echo(htmlentities($_POST['Contact_Woonplaats'], ENT_QUOTES)); ?>">
			    				  <?php if(!isset($_POST['parent_gegevens']) && isset($_POST['Contact_Woonplaats']) && $_POST['Contact_Woonplaats'] == '') echo('<b>* Er is geen woonplaats ingevoerd!</b>'); ?></td>			    				
				    		</tr>
				    		<tr>
				    			<td>E-mail:</td>
				    			<td><input name="Contact_Email" type="text" value="<?php if(isset($_POST['Contact_Email'])) echo(htmlentities($_POST['Contact_Email'], ENT_QUOTES)); ?>">
			    				  <?php 
			    				  	if (isset($_POST['Contact_Email']))
			    				  		$mail = $_POST['Contact_Email'];
			    				  	else $mail = '';
	
			    				  	if(isset($_POST['opslaan'])) {
												if(isset($_POST['parent_gegevens'])) {
													if ($mail != '' && !mail_check($mail))
				   				  				echo('<b>* Er is geen geldig e-mailadres ingevoerd!</b>');
												}
												else {
													if ($mail == '' || !mail_check($mail))
				   				  				echo('<b>* Er is geen geldig e-mailadres ingevoerd!</b>');
												}
			    				  	}
			    				  ?>
			    				</td>
				    		</tr>
				    		<tr>
				    			<td>Telefoon (vast):</td>
				    			<td><input name="Contact_Telefoon_Vast" type="text" value="<?php if(isset($_POST['Contact_Telefoon_Vast'])) echo(htmlentities($_POST['Contact_Telefoon_Vast'], ENT_QUOTES)); ?>">
			    				  <?php 
			    				  	if (isset($_POST['Contact_Telefoon_Vast']))
			    				  		$vast = $_POST['Contact_Telefoon_Vast'];
			    				  	else $vast = '';
			    				  	
/*			    				  	if(isset($_POST['opslaan'])) {
												if(isset($_POST['parent_gegevens'])) {
													if ($vast != '' && !telefoon_check($vast))
				   				  				echo('<b>* Er is geen geldig vast nummer ingevoerd!</b>');
												}
												else {
													if ($vast == '' || !telefoon_check($vast))
				   				  				echo('<b>* Er is geen geldig vast nummer ingevoerd!</b>');
												}
			    				  	}*/
			    				  ?>
			    				</td>
				    		</tr>
				    		<tr>
				    			<td>Telefoon (mobiel):</td>
				    			<td><input name="Contact_Telefoon_Mobiel" type="text" value="<?php if(isset($_POST['Contact_Telefoon_Mobiel'])) echo(htmlentities($_POST['Contact_Telefoon_Mobiel'], ENT_QUOTES)); ?>">
			    				  <?php 
			    				  	if (isset($_POST['Contact_Telefoon_Mobiel']))
			    				  		$mobiel = $_POST['Contact_Telefoon_Mobiel'];
			    				  	else $mobiel = '';
	
			    				/*  	if(isset($_POST['opslaan'])) {
												if(isset($_POST['parent_gegevens'])) {
													if ($mobiel != '' && !telefoon_check($mobiel))
				   				  				echo('<b>* Er is geen geldig mobiel nummer ingevoerd!</b>');
												}
												else {
													if ($mobiel == '' || !telefoon_check($mobiel))
				   				  				echo('<b>* Er is geen geldig mobiel nummer ingevoerd!</b>');
												}
			    				  	}*/
			    				  ?>
			    				</td>
				    		</tr>
				    		<tr>
				    			<td>Telefoon (fax):</td>
				    			<td><input name="Contact_Telefoon_Fax" type="text" value="<?php if(isset($_POST['Contact_Telefoon_Fax'])) echo(htmlentities($_POST['Contact_Telefoon_Fax'], ENT_QUOTES)); ?>">
			    				  <?php 
			    				  	if (isset($_POST['Contact_Telefoon_Fax']))
			    				  		$fax = $_POST['Contact_Telefoon_Fax'];
			    				  	else $fax = '';
	
			    				 /* 	if(isset($_POST['opslaan'])) {
												if ($fax != '' && !telefoon_check($fax))
			   				  				echo('<b>* Er is geen geldig fax nummer ingevoerd!</b>');
			    				  	}*/
			   						?>
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
			    ?>
		    
		  </div>
	
	<?php  
	      }
		//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
		else header("Location: index.php");  
	}
?>