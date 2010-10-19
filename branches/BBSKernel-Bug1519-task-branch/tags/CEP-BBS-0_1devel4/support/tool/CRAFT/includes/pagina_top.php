<html>
	<head>
		<title>LOFAR-CRAFT</title>

		<?php
			$pos  = strripos($_SERVER['PHP_SELF'], '/');
			$help = (substr($_SERVER['PHP_SELF'] ,0, $pos) . "/");
	
			$css_url1 = ("'<style type=\"text/css\" media=\"all\"> @import \"".$help."includes/stylesheet_1024.css\"; </style>'");
			$css_url2 = ("'<style type=\"text/css\" media=\"all\"> @import \"".$help."includes/stylesheet_1280.css\"; </style>'");
		?>
		
		<!-- Het toevoegen van de stylesheet, dit is afhankelijk van de scherm resolutie -->
		<script type="text/javascript">
  		
  		if (screen.width > 1024)
    		document.write(<?php echo($css_url2); ?>);
    	else document.write(<?php echo($css_url1); ?>);
		</script>


  	<!-- Het toevoegen van het javascript bestandje met de benodigde functies --> 
		<?php 
			if (isset($_SESSION['gebr_id'])) {
				echo("<script type=\"text/javascript\" src=\"".$help."includes/functies.php\"></script>"); 
			}
			
		?>
		

	</head>
	
	<body 
  <?php 
	  //controleren of er een startpagina opgeslagen is.
	  //dit is benodigd om de startpagina in het menu te highlighten, na het inloggen
	  //if (isset($_SESSION['pag_start']) && isset($_GET['p'])) 
		//	echo('onload="changeSubmenu('.$_SESSION['pag_start'].','.$_GET['p'].',0)">');
		//else 
		echo(">");
		?>
	  <div id="container">

			<?php
				//het includen van het menu en het hoofdscherm
				include_once('..'. $help .'includes/menu.php');
			?>