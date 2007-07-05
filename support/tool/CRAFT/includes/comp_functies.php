<?php

	session_start();

?>

function switchDocument(naam)
{
	var y=document.getElementById('comp_type').value;
	document.getElementById('frame_parent').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_parent.php?c=" + y + naam;
	document.getElementById('frame_naam').src = "<?php echo($_SESSION['pagina']); ?>admin_componenten/comp_naam.php?c=" + y + naam;	
}

function submitComponentToevoegen() 
{
	var x = document.frames['frame_parent'].document.getElementById('sComp_Parent').value;
	var y = document.frames['frame_naam'].document.getElementById('sComp_Naam').value;
	var aantal  = document.frames['frame_naam'].document.getElementById('sComp_Aantal').value;
	var maximum = document.frames['frame_naam'].document.getElementById('sComp_Max').value;
	document.getElementById('hidden_type').value = x;
	document.getElementById('hidden_naam').value = y;
	document.getElementById('hidden_aantal').value = aantal;
	document.getElementById('hidden_maximum').value = maximum;
	document.theForm.submit();
}