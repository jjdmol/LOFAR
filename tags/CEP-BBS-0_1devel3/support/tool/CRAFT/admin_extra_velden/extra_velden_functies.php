<?php

	session_start();
?>

function switchType()
{
	var datatype= document.getElementById('datatype').value;
	var waarde1 = document.getElementById('waarde1').value;
	var waarde2 = document.getElementById('waarde2').value;
	var url = '';
	if (waarde1 != '') url = ('&d1=' + waarde1);
	if (waarde2 != '') url = url + '&d2=' + waarde2;

	document.getElementById('frame_waardes').src = "<?php echo($_SESSION['pagina']); ?>admin_extra_velden/datatype_keuze.php?c=" + datatype + url;
}

function switchDocument()
{
	var z=document.getElementById('koppel').value;
	document.getElementById('frame_component').src = "<?php echo($_SESSION['pagina']); ?>admin_extra_velden/type_keuze.php?c=" + z;
}

function submitFunctie() 
{
	var z = document.frames['frame_component'].document.getElementById('sType').value;
	document.getElementById('hidden_component').value = z;
	
	var datatype= document.getElementById('datatype').value;
	
	
	//wanneer type een bestand is, dan zijn er geen waardes mogelijk 
	if (datatype != 5) {
		var waarde1 = document.frames['frame_waardes'].document.getElementById('data').value;
		document.getElementById('waarde1').value = waarde1;
		if (datatype == 4) {
			var waarde2 = document.frames['frame_waardes'].document.getElementById('tijd').value;
			document.getElementById('waarde2').value = waarde2;
		}
	}

	document.theForm.submit();
}