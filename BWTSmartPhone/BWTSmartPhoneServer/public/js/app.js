var assignments;
var words;
var rawData;


$(document).ready(function(){

  // Turn off Bootstrap's html-to-javascript features.
  $('body').off('.data-api');
  // Initialize modals:
  $('#helpModal').modal({show: false});

  /****************************
    Templates
  ****************************/

  // Handlebars template for a word table row.
  var wordRow = Handlebars.compile(
    "<tr>" +
      "<td class='tbl-word'>{{word}}</td>" +
      "<td class='tbl-def'>{{def}}</td>" +
      "<td class='tbl-edit'>" +
          "<button class='btn btn-mini btn-primary save-btn'>Save</button>" +
          "<button class='btn btn-mini btn-danger delete-btn'>Delete</button>" +
      "</td>" +
    "</tr>");

  // Handlebars template for an assignment row.
  var assnRow = Handlebars.compile(
    "<li class='{{#if active}}active {{/if}}assn-item' data-assn='{{assn}}'>"+
      "<a href='#'>{{assn}}</a>"+
    "</li>");


  /****************************
    Populators
  ****************************/

  // Takes an array of word items and injects them into the DOM
  function populateWordList(data){
    // Type checking
    if(typeof data != "object" || !(data instanceof Array)){
      console.error("Passed invalid data: ", data);
    }

    // For each item, add its position to its object and add it to the DOM.
    for(var i in data){
      $(".word-list").append(wordRow(data[i]));
    }
  }



  /****************************
    Listeners
  ****************************/
  $("#help-btn").click(function(){
    $("#helpModal").modal('show');
  });

  // Cause clicking on a table cell to convert it into a textbox.
  function rebindTable(){
    $(".tbl-word, .tbl-def, .save-btn, .delete-btn").unbind('click');
    $(".tbl-word, .tbl-def").click(function(){

      $that = $(this);

      // If this cell is already active, do nothing.
      if($(this).find(".temp-input").length !== 0){
        return;
      }

      // Create an input populated with the cell's data.
      var replacement = $("<input>")
        .attr("value", $(this).html())
        .attr("style", "width: 100%")
        .addClass("temp-input");

      // Insert it into the table and focus on it. 
      $(this).html(replacement);
      $(".temp-input").focus();

      $(".temp-input").blur(function(){

          $(this).after($(this).val());
          $(this).remove();
          validateRows();

      });

      $("temp-input").focus();
    });

    // Binds the save button command to the save button. 
    $(".save-btn").click(function(){
      // Get the appropriate information from the table.
      var thisWord = $(this).closest("tr").find(".tbl-word").html();
      var thisDef = $(this).closest("tr").find(".tbl-def").html();
      var currList = $("#assn-title").html();

      var reqURL = "/" + thisWord;

      // If the word isn't currently in the wordlist, request to create it.
      if(!words[thisWord]){
        // Add it to our local wordlist. 
        words[thisWord] = {word : thisWord, def : thisDef, assns : [currList]};
        $.post("/words", words[thisWord], function(data){
          if(data.result === "success"){
            $that.parent().addClass("success");
            setTimeout(function(){
              $that.parent().removeClass("success");
            }, 5000);
          }
          else{
            console.log(data);
            $that.parent().addClass("error");
          }
        });
        return;
      }

      // If the word isn't in the local assignment, add it.
      if($.inArray(currList, words[thisWord].assns) === -1)
        words[thisWord].assns.push(currList);

      // Create the word to be sent to the server.
      var payload = {};
          payload.word = thisWord;
          payload.def  = thisDef;
          payload.assn = words[thisWord].assns;

      // Make the request.
      $.ajax({
          url: '/words' + reqURL,
          type: 'PUT',
          data: payload,
          success: function(data) {
            if(data.result === "success"){
              $that.parent().addClass("success");
              assignments[payload.word] = payload;
              setTimeout(function(){
                $that.parent().removeClass("success");
              }, 5000);
            }
            else{
              console.log(data);
              $that.parent().addClass("error");
            }
          }
      });
    });

    // Binds the delete command to the delete button. 
    $(".delete-btn").click(function(){
      // Get info about the word.
      var thisWord = $(this).closest("tr").find(".tbl-word").html();
      var $that = $(this);
      var thisList = $("#assn-title").html();

      // Send a delete request to the appropriate url.
      $.ajax({
          url: '/words/' + thisWord,
          type: 'DELETE',
          success: function(result) {
            $that.closest('tr').remove();
            words[thisWord] = null;

            var deleteIndex;

            for(var i = 0; i < assignments[thisList].length; i++){
              if(assignments[thisList][i].word === thisWord){
                deleteIndex = i;
                break;
              }
            }

            assignments[thisList].splice(deleteIndex, 1);

          }
      });
    });
  }

  // Repopulate the word list whenever an assignment is picked. 
  function rebindAssignments(){
    $(".assn-item").unbind('click');
    $(".assn-item").click(function(){
      $(this).siblings().removeClass("active");
      $(this).addClass("active");
      renderWordList($(this).attr("data-assn"));
    });
  }

  $("#add-word").click(function(){
    $(".word-list").append(wordRow({
      word:"Click here to add a word...",
      def:"Click here to add a definition"
    }));
    rebindTable();
  });



  /****************************
    Validators
  ****************************/

  // Checks to see if any row has a word longer than 8 characters.  
  function validateRows(){
    var $words =$(".tbl-word");

    if($words.length <= 0)
      return;

    for(var i in $words){
      if($($words[i]).html().length > 8){
        $($words[i]).parent().addClass("error");
      }
      else{
        $($words[i]).parent().removeClass("error");
      }
    }
  }

  /****************************
    Data Parsing
  ****************************/


  function parseData(data){
    rawData = data;
    assignments = {};
    words = {};

    for(var i in data){
      words[data[i].word] = data[i];

      var _assns = data[i].assns;
      for(var assn in _assns){
        if(!assignments[_assns[assn]])
          assignments[_assns[assn]] = [];

        assignments[_assns[assn]].push({
          word: data[i].word,
          def: data[i].def
        });
      }
    }

    for(var list in assignments){
      assignments[list].sort(function(a,b){
        return a.word.localeCompare(b.word);
      });
    }

    // Get an arbitrary first assignment.
    var first;
    for(first in assignments) break;

    // Display said arbitrary assignment.
    renderWordList(first);


    // Find all assignment and create typeahead
    var typeaheadList = [];

    for(var list in assignments){
      typeaheadList.push(list);
      var payload = {assn : list};

      if(list === first)
        payload.active = true;
      else
        payload.active = false;

      // Add assignments to the assignment list
      $("#assn-list").append(assnRow(payload));
    }

    // Initialize typeahead
    $('.assignments').typeahead({
      'source' : typeaheadList
    });

    // Set active assignment title. 
    $("#assn-title").html(first);


    console.log(assignments);
  }

  // Given an assignment name, populates the table with the associated words.
  function renderWordList(assn){
    // Throw out bad queries.
    if(assignments[assn] === null){
      console.error("Undefined assignment " + assn);
      return;
    }

    // Update table title.
    $("#assn-title").html(assn);
    //Empty the table
    $(".word-list").html("");
    //And populate it with new data
    populateWordList(assignments[assn]);

    rebindTable();
  }


  $.get("/words", function(data){
    console.log(data);
    console.log(parseData(data));
    rebindAssignments();
  });


});
