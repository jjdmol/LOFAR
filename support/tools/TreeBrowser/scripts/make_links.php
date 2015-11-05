<?php
$name = $_REQUEST["name"];
$realname= str_replace("~","/",$name);
$isUsed=file_exists("../dep/".$name.".used.html");
$isUses=file_exists("../dep/".$name.".uses.html");
$isFlat=file_exists("../dep/".$name.".flat.html");
?>

<html>
<head>
<link href="../../docxxhtml/doxygen.css" rel="stylesheet" type="text/css">
<title> Links Page </title>
</head>
<body>
<center>
<b>Package  
<?php 
echo $realname; 
?> 
</b>
<?php
if ($isUsed) {
?>
  --  <a href="../dep/<?php echo $name;?>.used.html" title="(shows in a
  recursive way the packages where <?php echo $realname;?> is used)" 
  target="depend"> Used</a>
<?php
} else {
?>
  --  Not Used
<?php
}
?>
<?php
if ($isUses) {
?>
  --  <a href="../dep/<?php echo $name;?>.uses.html" title="(shows in a
  recursive way the packages used by <?php echo $realname;?>)" 
  target="depend"> Uses</a>
<?php
} else {
?>
  --  No Uses
<?php
}
?>
<?php
if ($isFlat) {
?>
  --  <a href="../dep/<?php echo $name;?>.flat.html" title="(shows the
  packages used by <?php echo $realname;?>)" target="depend"> Flat</a>
<?php
}
?>
  ---  <a href="http://svn.astron.nl/CDash/index.php?project=LOFAR" 
  target="_blank">Build results</a>
</center>
</body>
</html>






