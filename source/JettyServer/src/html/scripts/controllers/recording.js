/** @file recording.js
 *  @brief Handles the recorder plugin of the emulator.
 *
 * Allows the user to record a sequence of button presses and resend
 * the exact sequence later.
 *
 *  @author Zi Wang
 */

var __KEY_DOWN_STRING = "Key down";
var __KEY_UP_STRING  = "Key up";
var __KEY_HOLD_STRING  = "Key hold";
var __SAVED_RECORDING_QUEUE =[];

/**
* Register the onclick action of the toggle recording button
* and the send recording button
*/
function __registerButtons(){
  var toggleButton = $("#recording_toggle");
  var sendButton = $("#send_recording");

  toggleButton.on('click',function(e){
    window.recording = !window.recording;
    $("#recording_toggle").toggleClass("active");
    if(window.recording){
      prevTimeStamp = undefined;
      sendButton.attr('disabled', 'disabled');
    }
    else{
      sendButton.removeAttr('disabled');
      __saveRecording();
      __updateRecordingDropdown();
    }

  });

  sendButton.on('click',function(){
    sendRecording();
  });
}

/**
* Initialize the recording plugin. Called by load.js#main()
*/
function initializeRecording(){
    window.recording = false;
    window.recordingQueue = [];
    window.prevTimeStamp = undefined;
    __registerButtons();
    __updateRecordingDropdown();

}

/**
* Append the button press and its time stamp to the recording queue.
*/
function appendToRecordingQueue(button, eventType){
    if(button === undefined ||eventType === undefined){
      window.LOG_WARNING("Invalid key press. Can't add to recording queue.");
      return;
    }
    var delta;
    var curr = new Date();
    if(prevTimeStamp == undefined){
      delta = 0;
    }
    else{
      delta = curr.getTime() - prevTimeStamp;
    }
    prevTimeStamp = curr.getTime();

    recordingQueue.push({
      button : button,
      timeStamp : delta,
      eventType : eventType
    });
}

/**
* Send the recordings and clear the recording queue.
*/
function sendRecording(){

  	if(recording){
  		LOG_ERROR("Can't send saved recordings while another recording is going on.");
  		return;
  	}
    if(__SAVED_RECORDING_QUEUE === undefined){
      LOG_ERROR("Can't find saved recordings. Please try recording again.");
      __SAVED_RECORDING_QUEUE = [];
      return;
    }

  	var recordingIndex = $("#recording_select").val();
  	if(recordingIndex === undefined || 
          recordingIndex >= __SAVED_RECORDING_QUEUE.length){
  		LOG_ERROR("No such recording");
  		return;
  	}
  	var queue = __SAVED_RECORDING_QUEUE[recordingIndex].queue;

  	if(!__isValidRecordingQueue(queue)){
  		LOG_ERROR("Invalid recording: " + __SAVED_RECORDING_QUEUE[recordingIndex].name);
  		return;
  	}
  	__sendRecordingHelper(queue,0);

}

function __sendRecordingHelper(queue, index){

    if(index >= queue.length){
      LOG_ERROR("Invalid recording queue index.");
      return;
    }
    var eventType = queue[index].eventType;
    var button = queue[index].button;
    if(eventType === __KEY_DOWN_STRING){
      button.press_down();
    }
    else if(eventType === __KEY_UP_STRING){
      button.press_up();
    }
    else{
      LOG_ERROR("Invalid event type in recording queue. " + eventType);
    }

    //check the next button in the queue
    if(index+1 >= queue.length){
      LOG_INFO("Done sending recording queue. Last index is " + index);
      return;//done with the queue.
    }
    var delta = queue[index+1].timeStamp;

    //after delta, send the next button
    setTimeout(function(){
      __sendRecordingHelper(queue, index+1);
    }, delta);
}

/**
* Saved the last recording into __SAVED_RECORDING_QUEUE
*/
function __saveRecording(){
  if(recordingQueue === undefined || recordingQueue.length === 0){
  	window.LOG_WARNING("No action recorded. Can't save empty recording queue.");
  	recordingQueue = [];
  	return;
  }
  var name = $("#recording_name").val();
  if(name === undefined || name.trim().length === 0){
    name = "Recording " + (__SAVED_RECORDING_QUEUE.length+1);
  }
  __SAVED_RECORDING_QUEUE.push({
    queue : recordingQueue,
    name  : name.trim()
  });
  prevTimeStamp = undefined;
  recordingQueue = [];
  $("#recording_name").val("");
}

/**
* Update the html dropdown to show all saved recordings.
*/
function __updateRecordingDropdown(){
  var dropdown = $("#recording_select");
  var options = "";
  for(var i=0;i<__SAVED_RECORDING_QUEUE.length;i++){
    options += "<option value='" + i +"'>" + __SAVED_RECORDING_QUEUE[i].name +"</option>";
  }
  dropdown.html(options);
}

/**
* Check if the given recording queue is valid.
*/
function __isValidRecordingQueue(queue){
  if(queue === undefined ||
    (! queue instanceof Array) ||
    queue.length === 0){
    return false;
  }
  for(var i=0; i<queue.length; i++){
    var curr = queue[i];
    if(curr.button === undefined ||
      curr.eventType === undefined ||
      [__KEY_UP_STRING,__KEY_DOWN_STRING].indexOf(curr.eventType) === -1 ||
      curr.timeStamp === undefined ||
      curr.timeStamp < 0){
      return false;
    }
  }
  return true;

}





