#!/bin/bash
#
# ngrok
# https://ngrok.com/
#
subdomain=pebble_signal
port=2015

ngrok authtoken 7mtRY4ZJDFWPuu4FFqmfg_6mHpCEzq2hTzmC4sg9Nbn
ngrok http -subdomain=$subdomain $port
