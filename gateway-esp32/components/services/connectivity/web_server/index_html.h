/**
 * @file index_html.h
 * @brief HTML content for the ESMU WiFi Provisioning page
 */

#pragma once

static const char* INDEX_HTML = 
"<!DOCTYPE html><html><head><title>ESMU WiFi Setup</title>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>body { font-family: Arial; margin: 20px; text-align: center; background-color: #f4f4f4; }"
"h2 { color: #333; } .container { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); display: inline-block; width: 300px; }"
"input { width: 90%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }"
"button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; width: 100%; font-size: 16px; }"
"button:hover { background: #0056b3; }</style></head><body>"
"<h2>ESMU WiFi Setup</h2>"
"<div class='container'>"
"<p>Enter your WiFi credentials to connect the ESMU to your network.</p>"
"<form action='/config' method='POST'>"
"<input type='text' name='ssid' placeholder='WiFi Name (SSID)' required>"
"<input type='password' name='pass' placeholder='Password'>"
"<button type='submit'>Connect</button>"
"</form></div></body></html>";
