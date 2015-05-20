Pebble.addEventListener(
    "ready",
    function(e) {
        console.log("PebbleKit JS ready!");
        Pebble.sendAppMessage({"KEY_NOTIFY_REQ_CONF": 0});
    }
);

Pebble.addEventListener(
    'appmessage',
    function(e) {
        console.log('Received message: ' + JSON.stringify(e.payload));
    }
);

Pebble.addEventListener(
    "showConfiguration",
    function(e) {
        Pebble.openURL("http://pebble_signal.ngrok.io/config.html");
    }
);

Pebble.addEventListener(
    "webviewclosed",
    function(e) {
        var config = JSON.parse(decodeURIComponent(e.response));
        console.log("Configuration window returned: " + JSON.stringify(config));

        Pebble.sendAppMessage({"KEY_CONF_VIBES_EACH_HOUR": Number(config.vibes),
                               "KEY_CONF_TIMEBAR_PATTERN": Number(config.timebar),
                               "KEY_CONF_TIME_PATTERN": Number(config.time)});
    }
);