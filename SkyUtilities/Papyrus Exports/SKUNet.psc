scriptname SKUNet hidden

; =========================================================
; EVENTS
; =========================================================
;
; OnHTTPRequestFinished:
; > event OnHTTPRequestFinished(int request_id, bool request_failed, int response_code, string response_body) 
; > endEvent
;


; =========================================================
; FUNCTIONS (STATIC)
; =========================================================
;
;
; Performs a HTTP POST request. 
; Registers for event if Form argument was passed: OnRequestFinished.
;
; @param	Form	Script instance on which the stated events will be registered and called.
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param 	string	Data to post. Make sure you encode the values if they contain illegal characters. See https://curl.haxx.se/libcurl/c/curl_easy_escape.html for more details
; @param	int		Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPPOSTRequest(Form form, string url, string data, int timeout) global native

;
; Performs a HTTP GET request.
; Registers for event if Form argument was passed: OnRequestFinished.
;
; @param	Form	Script instance on which the stated events will be registered and called.
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param	float	Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPGETRequest(Form form, string url, int timeout) global native

;
; The function name pretty much says it all. For more information see: https://curl.haxx.se/libcurl/c/curl_easy_escape.html
;
; @param 	string	raw string
; @return	string	URL encoded string
;
;
string function URLEncode(string data) global native

; ======================
;  NOT IMPLEMENTED YET:
; ======================

;
; This function will check for a newer version of the mod identified by the corresponding id than the version supplied as argument
;
; @param	string	Nexus mod ID ("www.nexusmods.com/skyrim/mods/<mod_id>". Example: "www.nexusmods.com/skyrim/mods/123456/", 123456 would be the id of the mod.)
; @param	string	Installed version of the mod (for comparison)
; @return	bool	Returns true if an update has been found
;
;bool NexusUpdateAvailable(string nexus_mod_id, string installed_version_str) global native

;
; This function will check for a newer version of the mod identified by the corresponding id than the version supplied as argument
;
; @param	string	Nexus mod ID ("www.loverslab.com/files/file/<mod_id>-some-mods-name/". Example: ""www.loverslab.com/files/file/123456-some-mods-name/", 123456 would be the id of the mod.)
; @param	string	Installed version of the mod (for comparison)
; @return	bool	Returns true if an update has been found
;
;bool LoversLabUpdateAvailable(string ll_mod_id, string installed_version_str) global native


