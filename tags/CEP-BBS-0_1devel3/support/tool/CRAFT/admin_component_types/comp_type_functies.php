<?php

	session_start();
?>

function switchDocument(selectie)
{
	var y=document.getElementById('parent').value;
	document.getElementById('frame_contact').src = "<?php echo($_SESSION['pagina']); ?>algemene_functionaliteit/type_verantwoordelijke.php?c=" + y + "&s=" + selectie;	
}

function submitTypeOpslaan() 
{
	var z = document.frames['frame_contact'].document.getElementById('sVerantwoordelijke').value;
	document.getElementById('hidden_verantwoordelijke').value = z;
	document.theForm.submit();
}