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
        var message = e.payload;
        config.KEY_CONF_VIBES_EACH_HOUR = message.KEY_CONF_VIBES_EACH_HOUR;
        config.KEY_CONF_TIMEBAR_PATTERN = message.KEY_CONF_TIMEBAR_PATTERN;
        config.KEY_CONF_TIME_PATTERN    = message.KEY_CONF_TIME_PATTERN;
        console.log('appmessage: ' + JSON.stringify(config));
    }
);

Pebble.addEventListener(
    "showConfiguration",
    function(e) {
        // var url = "https://pebble_signal.srvdir.net " // for debug
        var url = "https://1c9dd430089b9ca482027abba2408b706433aefa.googledrive.com/host/0B0SsQ4mAFAiofldjQ203S1Z4WWUySVRjM1NvSEQ2UHA2eDNwSGJJY1VoRUFGNFB6YnJsS3M/index.html";
        url += "?vibes=" + config.KEY_CONF_VIBES_EACH_HOUR;
        url += "&timebar=" + config.KEY_CONF_TIMEBAR_PATTERN;
        url += "&time=" + config.KEY_CONF_TIME_PATTERN;
        
        console.log('showConfiguration: ' + url);
        Pebble.openURL(url);
    }
);

Pebble.addEventListener(
    "webviewclosed",
    function(e) {
        var response = JSON.parse(decodeURIComponent(e.response));
        console.log("webviewclosed: " + JSON.stringify(response));

        config.KEY_CONF_VIBES_EACH_HOUR = Number(response.vibes);
        config.KEY_CONF_TIMEBAR_PATTERN = Number(response.timebar);
        config.KEY_CONF_TIME_PATTERN = Number(response.time);

        Pebble.sendAppMessage(config);
    }
);