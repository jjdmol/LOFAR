<div id="hoofdbalk">
	<div id="hoofdbalk1">
  	<div id="spacerbalk">
    </div> 
    <div id="hoofdmenu">
 	  	<?php if(isset($_SESSION['pag_start']) && $_SESSION['pag_start'] == 1) { ?>
 	  			<div ID="hoofdmenuoptie1"><a class="hoofdmenuoptie" id="hoofdoptie1" onmouseover="changeSubmenu(1,<?php echo($_GET['p']); ?>,0)" href="main.php?p=1">Start</a></div>
 	  	<?php } if(isset($_SESSION['pag_comp']) && $_SESSION['pag_comp'] == 1) { ?>
 	  			<div ID="hoofdmenuoptie2"><a class="hoofdmenuoptie" id="hoofdoptie2" onmouseover="changeSubmenu(2,<?php echo($_GET['p']); ?>,0)" href="main.php?p=2">Componenten</a></div>
 	  	<?php } if(isset($_SESSION['pag_meld']) && $_SESSION['pag_meld'] == 1) { ?>
	  			<div ID="hoofdmenuoptie3"><a class="hoofdmenuoptie" id="hoofdoptie3" onmouseover="changeSubmenu(3,<?php echo($_GET['p']); ?>,0)" href="main.php?p=3">Meldingen</a></div>
 	  	<?php } if(isset($_SESSION['pag_stats']) && $_SESSION['pag_stats'] == 1) { ?>
	  			<div ID="hoofdmenuoptie4"><a class="hoofdmenuoptie" id="hoofdoptie4" onmouseover="changeSubmenu(4,<?php echo($_GET['p']); ?>,0)" href="main.php?p=4">Statistieken</a></div>
 	  	<?php } if(isset($_SESSION['pag_instel']) && $_SESSION['pag_instel']== 1) { ?>
	  			<div ID="hoofdmenuoptie5"><a class="hoofdmenuoptie" id="hoofdoptie5" onmouseover="changeSubmenu(5,<?php echo($_GET['p']); ?>,0)" href="main.php?p=5">Instellingen</a></div>	
 	  	<?php } ?>
		</div>
	</div>   
  <div id="logo">
  </div> 
</div>
<div id="subbalk">	
	<div id="submenu">
		<ul>
	  	<li><A name="suboptie1" href="#section1"></A></li>
	    <li><A name="suboptie2" href="#section1"></A></li>
	    <li><A name="suboptie3" href="#section1"></A></li>
	    <li><A name="suboptie4" href="#section1"></A></li>
	    <li><A name="suboptie5" href="#section1"></A></li>
	    <li><A name="suboptie6" href="#section1"></A></li>
		</ul>
	</div>
	<div id="alg_opties">
		<ul>
	  	<?php 
	  		if (isset($_SESSION['admin_rechten'])&& $_SESSION['admin_rechten'] == 1) {
		  		if (isset($_SESSION['admin_deel']) && $_SESSION['admin_deel'] == 0) { 
		  			echo("<li><A name=\"Admin\" href=\"admin.php?p=1&s=1\">Admin-gedeelte</A></li>");
		  	 	} else 
		  			echo("<li><A name=\"Admin\" href=\"main.php\">Hoofdapplicatie</A></li>");
	  		}
	  	?>
	  	<li><A name="LogUit" href="index.php">Uitloggen</A></li>
		</ul>
	</div>
</div>
