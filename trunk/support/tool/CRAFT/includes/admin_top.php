<?php
  require_once('includes/login_funcs.php');
	
  //controleren of er iemand ingelogd is...
  if ($LOGGED_IN = user_isloggedin()) {
  	
  	?>

<html>
	<head>
		<title>LOFAR-CRAFT</title>
		
		<?php
			$css_url1 = ("'<style type=\"text/css\" media=\"all\"> @import \"".$_SESSION['pagina']."includes/stylesheet_1024.css\"; </style>'");
			$css_url2 = ("'<style type=\"text/css\" media=\"all\"> @import \"".$_SESSION['pagina']."includes/stylesheet_1280.css\"; </style>'");
		?>
		<!-- Het toevoegen van de stylesheet, dit is afhankelijk van de scherm resolutie -->
		<script type="text/javascript">
  		if (screen.width > 1024)
    		document.write(<?php echo($css_url2); ?>);
    	else document.write(<?php echo($css_url1); ?>);
		</script>
  	
  	<!-- Het toevoegen van het javascript bestandje met de benodigde functies --> 
		<?php echo('<script type="text/javascript" src="'.$_SESSION['pagina'].'includes/functies.php"></script>'); ?>

	</head>
	
	<body bgcolor="#2E4C6B" 
  <?php 
	  //controleren of er een startpagina opgeslagen is.
	  //dit is benodigd om de startpagina in het menu te highlighten, na het inloggen
	  //if (isset($_SESSION['pag_start']) && isset($_GET['p'])) 
		//	echo('onload="changeSubmenu(1,1,1)">');
		//else 
		echo(">");
		?>

	  <div id="container">

			<?php
				//het includen van het menu en het hoofdscherm
				include("admin_menu.php");
			?>
			
<?php  
      }
	//niemand ingelogt, dus bezoeker naar de inlogpagina sturen
	else header("Location: index.php");  
?>