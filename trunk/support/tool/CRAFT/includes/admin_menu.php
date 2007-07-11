<?php 
	if (isset($_SESSION['admin_deel'])){
?>

<div id="hoofdbalk">
	<div id="admin_hoofdbalk">
  	<div id="spacerbalk">
    </div> 
    <div id="hoofdmenu">
			<div ID="hoofdmenuoptie6"><a class="hoofdmenuoptie" id="hoofdoptie1" onmouseover="changeSubmenu(1,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=1&s=1">Comp.Types</a></div>
			<div ID="hoofdmenuoptie7"><a class="hoofdmenuoptie" id="hoofdoptie2" onmouseover="changeSubmenu(2,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=2&s=1">Componenten</a></div>
			<div ID="hoofdmenuoptie8"><a class="hoofdmenuoptie" id="hoofdoptie3" onmouseover="changeSubmenu(3,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=3&s=1">Meldingtypes</a></div>
			<div ID="hoofdmenuoptie9"><a class="hoofdmenuoptie" id="hoofdoptie4" onmouseover="changeSubmenu(4,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=4&s=1">Meldingen</a></div>
			<div ID="hoofdmenuoptie10"><a class="hoofdmenuoptie" id="hoofdoptie5" onmouseover="changeSubmenu(5,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=5&s=1">Gebruikersgroepen</a></div>
			<div ID="hoofdmenuoptie11"><a class="hoofdmenuoptie" id="hoofdoptie6" onmouseover="changeSubmenu(6,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=6&s=1">Gebruikers</a></div>
			<div ID="hoofdmenuoptie12"><a class="hoofdmenuoptie" id="hoofdoptie7" onmouseover="changeSubmenu(7,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=7&s=1">Contacten</a></div>
			<div ID="hoofdmenuoptie13"><a class="hoofdmenuoptie" id="hoofdoptie8" onmouseover="changeSubmenu(8,<?php echo($_GET['p']); ?>,1)" href="admin.php?p=8&s=1">Locatie</a></div>
		</div>
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
	  	<?php if (isset($_SESSION['admin_deel']) && $_SESSION['admin_deel'] == 0) { 
	  		echo("<li><A name=\"Admin\" href=\"admin.php?p=0\">Admin-gedeelte</A></li>");
	  	 } else 
	  		echo("<li><A name=\"Admin\" href=\"main.php\">Hoofdapplicatie</A></li>");
	  	?>
	  	<li><A name="LogUit" href="index.php">Uitloggen</A></li>
		</ul>
	</div>
</div>

<?php
	}
?>
