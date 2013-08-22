//FROM 15237 F2011 - Cross platform mobile web apps
var App = function(){
  this.registerEvents();
};

App.prototype.registerEvents = function(){
  this.registerLogin();
};

// Wrapper for our POST request. 
App.prototype.ajaxFormJSON = function(json, url, onSuccess, onError){
  var data = new FormData();
  for (var key in json){
    data.append(key, json[key]);
  }

    $.ajax({
      url: url,
      data: data,
      cache: false,
      contentType: false,
      processData: false,
      type: 'POST',
      success: onSuccess,
      error: onError
    });
};

// Gets the username and password and attempts to authenticate the session. 
App.prototype.registerLogin = function(){
  $('#login').click(function(){
    var username = $('#loginUsername').val();
    var password = $('#loginPassword').val();

    var data = new FormData();
    data.append('username', username);
    data.append('password', password);

    this.ajaxFormJSON(
    {
      username: username,
      password: password
    },
    '/login',
    function success(data){
      if (data === 'success'){
        window.location = '/';
      }
      else {
        console.log(JSON.stringify(data));
      }
    },
    function error(xhr, status, err){
      if(err){
        $("input").addClass("error-custom").removeClass("disabled-custom");
      }
    });
  }.bind(this));
};


$(document).ready(function(){
  // Initialize the login app.
  new App();

  // Bind enter to the submit button. 
  $("#loginPassword").keyup(function(event){
    if(event.keyCode === 13){
    $("#login").click();
    $("input").addClass("disabled-custom");
    }
  });

  $("#loginUsername").focus();

});