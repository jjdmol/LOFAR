<?php
$name      = $_REQUEST["name"];
$realname  = str_replace("_","/",$name);
$namearray = explode('_',$name);
$shortname = $namearray[sizeof($namearray)-1];
$docname   = "group__".$shortname.".html";
?>
<html>
<head>
<title> Package Page </title>
</head>
<body bgcolor="#FFFFFF">

<script type="text/JavaScript" language="Javascript1.2">
var jname= '<?php echo $name; ?>';
var u_id="../scripts/make_links.php?name="+jname;
window.open(u_id,"links");
</script>

<?php
if (file_exists("../../docxxhtml/".$docname)) {
?>
<script type="text/JavaScript" language="Javascript1.2">
var docname= '../../docxxhtml/<?php echo $docname; ?>';
window.open(docname,"description");
</script>
<?php
} else {
?>
<center>
<h3> No doxygen documentation for
<?php 
echo $realname; 
?> 
 available yet.
</h3>
</center>

<?php
    }
?>
</body>
</html>
