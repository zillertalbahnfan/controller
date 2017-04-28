var CONTROLLER = {
  "url":"http://192.168.0.150",
  "speed_buttons": {
    "controller_start": {
      "button": "success",
      "action": "starting",
      "label": "Start",
      "url": "/loco/start"
    },
    "controller_coast":{
      "button": "warning",
      "action": "coasting",
      "label": "Coast",
      "url": "/loco/coast"
    },
    "controller_brake":{
      "button": "danger",
      "action": "braking",
      "label": "Brake",
      "url": "/loco/brake"
    }
  },
  "direction_buttons": {
    "jenbach":{
      "button": "info",
      "direction": "jenbach",
      "label": "Jenbach",
      "url": "/loco/jenbach"
    },
    "mayrhofen":{
      "button": "info",
      "direction": "mayrhofen",
      "label": "Mayrhofen",
      "url": "/loco/mayrhofen"
    }
  }
}
