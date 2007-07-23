<?php

	session_start();
?>

function switchDocument()
{
	var z=document.getElementById('koppel').value;
	document.getElementById('frame_component').src = "<?php echo($_SESSION['pagina']); ?>admin_extra_velden/type_keuze.php?c=" + z;	
}

function submitFunctie() 
{
	var z = document.frames['frame_component'].document.getElementById('sType').value;
	document.getElementById('hidden_component').value = z;

	document.theForm.submit();
}