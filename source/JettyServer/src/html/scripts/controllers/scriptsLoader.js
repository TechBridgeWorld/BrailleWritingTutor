

function loadAllScripts () {
	
	$.ajax({
		url : '/loadScripts.do?name=listing',
		type: 'GET',
		success : function(data){
			__updateScriptsDropdown(data);
		},
		error : function(data){
			window.LOG_ERROR("Errors");
		}
	});

}

function __updateScriptsDropdown(allScripts){

}