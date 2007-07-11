<?php

	session_start();

?>

function switchMelding() 
{
	var y=document.getElementById('Type_Melding').value;
	document.getElementById('frame_beschrijving').src = "<?php echo($_SESSION['pagina']); ?>admin_meldingen/probleem_beschrijving.php?c=" + y;
	document.getElementById('frame_oplossing').src = "<?php echo($_SESSION['pagina']); ?>admin_meldingen/probleem_oplossing.php?c=" + y;
}


function SubmitMeldingToevoegen() 
{
	var s = document.frames['frame_beschrijving'].document.getElementById('sStatus').value;
	var o = document.frames['frame_oplossing'].document.getElementById('sProb_Oplossing').value;
	var b = document.frames['frame_beschrijving'].document.getElementById('sProb_Beschrijving').value;
	
	document.getElementById('hidden_status').value = s;
	document.getElementById('hidden_oplossing').value = o;
	document.getElementById('hidden_beschrijving').value = b;
	
	document.theForm.submit();
}