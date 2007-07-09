<?php

	session_start();

?>

function switchDocument()
{
	var y=document.getElementById('groepsparent').value;
	document.getElementById('frame_gegevens').src = "<?php echo($_SESSION['pagina']); ?>admin_gebruikersgroep/groep_gegevens.php?c=" + y;
}

function submitGroepToevoegen() 
{
	document.getElementById('hidden_inst').value = document.frames['frame_gegevens'].document.getElementById('Inst_Zichtbaar').checked;
	document.getElementById('hidden_comp').value = document.frames['frame_gegevens'].document.getElementById('Comp_Zichtbaar').checked;
	document.getElementById('hidden_intro').value = document.frames['frame_gegevens'].document.getElementById('Intro_Zichtbaar').checked;
	document.getElementById('hidden_stats').value = document.frames['frame_gegevens'].document.getElementById('Stats_Zichtbaar').checked;
	document.getElementById('hidden_bewerk').value = document.frames['frame_gegevens'].document.getElementById('Bewerk_Rechten').checked;
	document.getElementById('hidden_melding').value = document.frames['frame_gegevens'].document.getElementById('Melding_Zichtbaar').checked;
	document.getElementById('hidden_toevoeg').value = document.frames['frame_gegevens'].document.getElementById('Toevoeg_Rechten').checked;
	document.getElementById('hidden_verwijder').value = document.frames['frame_gegevens'].document.getElementById('Verwijder_Rechten').checked;

	document.theForm.submit();
}