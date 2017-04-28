function getValues(server, method, callback) {
  $.ajax({
      type: 'GET',
      url: server + method,
      statusCode: {
        200: function(data, method) {
          $(".debug").prepend("Loading " + method + "<br />" + JSON.stringify(data) + "<br /><br />");
          success_count += 1;
          $("#success_count").html(success_count);
          if (data["next"] == "refresh") {
            getStatus();
          }
          if (callback) {
            callback(data);
          }
        },
        423: function(data) {
          $(".debug").prepend("ERROR " + method + "<br />" + JSON.stringify(data["responseJSON"]) + "<br /><br />");
          $("#modal_title").html("Response from " + method);
          $("#modal_body").html(data["responseJSON"]["message"])
          $("#modal_dialog").modal("show");
          fail_count += 1;
          $("#fail_count").html(fail_count);
        }
      }
    });
}



// creates a set of speed and direction buttons

function setupButtons() {
  for (var button in CONTROLLER["speed_buttons"])
  {
      var thisbutton_set = CONTROLLER["speed_buttons"][button];
      thisbutton = "#" + button;
      $("#speed_buttons").append(
        "<button type=\"button\" class=\"btn btn-sm btn-pre-"+ thisbutton_set["button"] +" \" id=\""+ button +"\" url=\""+ thisbutton_set["url"] +"\">"+ thisbutton_set["label"] +"</button> ");
  }
  for (var button in CONTROLLER["direction_buttons"])
  {
      var thisbutton_set = CONTROLLER["direction_buttons"][button];
      thisbutton = "#" + button;
      $("#direction_buttons").append(
        "<button type=\"button\" class=\"btn btn-sm btn-pre-info\" id=\""+ button +"\" url=\""+ thisbutton_set["url"] +"\">"+ thisbutton_set["label"] +"</button> ");
  }
  bindClicks();
}


// binds clicks to the buttons when in automatic mode

function bindClicks() {
  $("#locomotive_list").change(function() {
    id = $("select#locomotive_list option:checked").attr("value");
    getValues(CONTROLLER["url"], "/loco/select?id="+ id, function (data) {
    });
  });

  for (var button in CONTROLLER["speed_buttons"])
  {
      thisbutton = "#" + button;
      $(thisbutton).click(function() {
        getValues(CONTROLLER["url"], $(this).attr("url"), false);
      });
  }

  for (var button in CONTROLLER["direction_buttons"])
  {
      thisbutton = "#" + button;
      $(thisbutton).click(function() {
        getValues(CONTROLLER["url"], $(this).attr("url"), false);
      });
  }

  $("#stop").click(function() {
    clearTimeout(poll_timeout);
    $("#stop").removeClass("btn-default")
    $("#refresh").addClass("btn-default")
  });

  $( "#refresh" ).click(function() {
    clearTimeout(poll_timeout);
    pollStatus();
    $("#stop").addClass("btn-default")
    $("#refresh").removeClass("btn-default")
  });

  getStatus();
}

// highlights the currently selected speed action

function paintSpeedButtons(current_action) {
  for (var button in CONTROLLER["speed_buttons"]) {
    var thisbutton = "#" + button;
    var active = "btn-" + CONTROLLER["speed_buttons"][button]["button"];
    var passive = "btn-pre-" + CONTROLLER["speed_buttons"][button]["button"];
    if (current_action == CONTROLLER["speed_buttons"][button]["action"]) {
      $(thisbutton).removeClass(passive);
      if(!$(thisbutton).hasClass(active)) {
        $(thisbutton).addClass(active);
      }
    }
    else {
      $(thisbutton).removeClass(active);
      if(!$(thisbutton).hasClass(passive)) {
        $(thisbutton).addClass(passive);
      }
    }
  }
}

// highlights the currently selected direction

function paintDirectionButtons(current_direction) {
  for (var button in CONTROLLER["direction_buttons"]) {
    var thisbutton = "#" + button;
    var active = "btn-" + CONTROLLER["direction_buttons"][button]["button"];
    var passive = "btn-pre-" + CONTROLLER["direction_buttons"][button]["button"];
    if (current_direction == CONTROLLER["direction_buttons"][button]["label"]) {
      $(thisbutton).removeClass(passive);
      if(!$(thisbutton).hasClass(active)) {
        $(thisbutton).addClass(active);
      }
    }
    else {
      $(thisbutton).removeClass(active);
      if(!$(thisbutton).hasClass(passive)) {
        $(thisbutton).addClass(passive);
      }
    }
  }
}

// gets the current status of the system.
// TODO run this every second

function getStatus() {
  var thisnow = new Date();
  getValues(CONTROLLER["url"], "/status", function (data) {
    if (data["current_mode"] == "MAN") {
        alert("manual")
    }
    else {
      paintSpeedButtons(data["current_action"]);
      console.log(data["current_action"])
      paintDirectionButtons(data["current_direction"]);
      var thisbutton = "#controller_mode";
      $(thisbutton).html("Automatic");
      $(thisbutton).removeClass("label-danger");
      $(thisbutton).removeClass("label-warning");
      if(!$(thisbutton).hasClass("label-success")) {
        $(thisbutton).addClass("label-success");
      }
    }
    $("#current_loco").html(data["current_loco"]);
    $("#current_direction").html(data["current_direction"]);
    $("#current_speed").html(data["current_speed"]);
    $("#current_action").html(data["current_action"]);
    $("#last_updated").html("Last updated: " + thisnow.toUTCString());
  });
}

function pollStatus(duration) {
  if (!duration) {
    duration = 1000;
  }
  poll_timeout = setTimeout(function() {
    getStatus();
    pollStatus(duration);
  }, duration);
}


// initial set up, gets the list of locos, sets up the buttons

function initializeValues() {
  getValues(CONTROLLER["url"], "/loco/list", function (data) {
    locomotive_list = data;
    if (locomotive_list["success"]) {
      for (i in locomotive_list["locomotives"]) {
        $("#locomotive_list").append("<option value=" + i + ">"+ locomotive_list["locomotives"][i] + "</option>");
      }
    }
  });
  pollStatus();
  setupButtons();
}
