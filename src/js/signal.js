var config = {
    "KEY_CONF_VIBES_EACH_HOUR": 0,
    "KEY_CONF_TIMEBAR_PATTERN": 0,
    "KEY_CONF_TIME_PATTERN": 0,
};

Pebble.addEventListener(
    "ready",
    function(e) {
        console.log("ready");
        Pebble.sendAppMessage({"KEY_NOTIFY_REQ_CONF": 0});
    }
);

Pebble.addEventListener(
    'appmessage',
    function(e) {
        config = JSON.stringify(e.payload);
        console.log('appmessage: ' + config);
    }
);

Pebble.addEventListener(
    "showConfiguration",
    function(e) {
        // var url = "http://pebble_signal.ngrok.io/config.html"
        var url = "https://googledrive.com/host//0B0SsQ4mAFAiofldjQ203S1Z4WWUySVRjM1NvSEQ2UHA2eDNwSGJJY1VoRUFGNFB6YnJsS3M";
        url += "?vibes=" + config.KEY_CONF_VIBES_EACH_HOUR;
        url += "&timebar=" + config.KEY_CONF_TIMEBAR_PATTERN;
        url += "&time=" + config.KEY_CONF_TIME_PATTERN;
        Pebble.openURL(url);
    }
);

Pebble.addEventListener(
    "webviewclosed",
    function(e) {
        config = JSON.parse(decodeURIComponent(e.response));
        console.log("webviewclosed: " + JSON.stringify(config));

        config.KEY_CONF_VIBES_EACH_HOUR = Number(config.vibes);
        config.KEY_CONF_TIMEBAR_PATTERN = Number(config.timebar);
        config.KEY_CONF_TIME_PATTERN = Number(config.time);

        Pebble.sendAppMessage(config);
    }
);