/** @file scriptsLoader.js
 *  @brief Handles the scripting plugin of the emulator.
 *
 * Allows the user to load and run saved scripts.
 *
 *  @author Zi Wang
 */


/**
 * Initialize the scripting plugin. 
 */
function initializeScripting(){
	$('#load_scripts').on('click',function(){
		loadAllScripts();
	});

	$('#run_script').on('click',function(){
		compileAndRun();
	});

	loadAllScripts();
}


/**
 * Get a list of script names and update 
 * the scripts dropdown
 */
function loadAllScripts () {
	
	$.ajax({
		url : '/loadScripts.do?name=listing',
		type: 'GET',
		success : function(data){
			__updateScriptsDropdown(data);
		},
		error : function(error){
			showScriptingMsg("Errors occurred in retrieving all scripts listing.<br />");
			window.LOG_ERROR("Errors occurred in retrieving all scripts listing.");
			window.LOG_ERROR(error);
		}
	});
}


function __updateScriptsDropdown(data){
	try{
		var allScripts = JSON.parse(data);
		var dropdown = $("#scripts_select");
		var options = "";
		for(var i=0;i<allScripts.length;i++){
		  options += "<option value='" + allScripts[i] +"'>" + allScripts[i] +"</option>";
		}
		dropdown.html(options);
	}
	catch(err){
		window.LOG_WARNING("Unable to parse the JSON string. Can't update scripts list.");	
	}
}

/**
 * Send ajax request to the server to compile the selected the script.
 * Run the script if there is no error. Otherwise display the error messages.
 */
function compileAndRun(){
	//first grab the script name
	var scriptName = $('#scripts_select').val();
	if(scriptName === undefined || scriptName.length === 0){
		showScriptingMsg("No script selected.");
		return;
	}
	$.ajax({
		url : '/loadScripts.do?name='+scriptName,
		type: 'GET',
		success : function(data){
			__prepareScript(data);
		},
		error : function(error){
			showScriptingMsg("Error occurred when connecting to the server to compile the script.");
			window.LOG_ERROR("Error occurred when connecting to the server to compile the script.");
			window.LOG_ERROR(error);
		}
	});

}

/**
* Convert the array sent from the server into a valid 
* recording queue. If successful, then run the recording queue.
*/
function __prepareScript(data){
	if(data === undefined){
		window.LOG_WARNING("Empty script sent from server.");
		return;
	}
	try{
		var parsedData = JSON.parse(data);
		if(parsedData.status === "success"){
			var tempQueue = parsedData.message;
			var scriptQueue = [];
			for(var i=0; i<tempQueue.length; i++){
				
				var buttonId = "_" + tempQueue[i].button;
				var type = tempQueue[i].eventType;
				var time = tempQueue[i].timeStamp;

				var btn = __BUTTON_MAP[buttonId];
				
				if(type === "click"){
					scriptQueue.push({
						button : btn,
						eventType : __KEY_DOWN,
						timeStamp : time
					});	

					scriptQueue.push({
						button : btn,
						eventType : __KEY_UP,
						timeStamp : 35
					});				
				}
				else if(type === 'hold'){
					scriptQueue.push({
						button : btn,
						eventType : __KEY_DOWN,
						timeStamp : time
					});	
				}
				else if(type === 'release'){
					scriptQueue.push({
						button : btn,
						eventType : __KEY_UP,
						timeStamp : time
					});	
				}
			}
			showScriptingMsg("Compilation successful.");
			__sendRecordingHelper(scriptQueue,0);
		}
		else{
			showScriptingMsg("Error when compiling script. <br />" + parsedData.message );
		}
	}
	catch(error){
		window.LOG_ERROR("Error when preparing the script. Unable to parse.");
		showScriptingMsg("Error when preparing the script. Unable to parse.");
	}

}



/**
* Show message on the message area.
*/
function showScriptingMsg(content){
	$("#script_message").html(content);
}