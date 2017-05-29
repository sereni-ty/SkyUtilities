scriptname SKUNet hidden

; =========================================================
; EVENTS
; =========================================================
;
; OnHTTPRequestFinished:
; > event OnHTTPRequestFinished(int request_id, bool request_failed, int response_code, string response_body) 
; > endEvent
;
; ---------------------------------------------------------
;
; OnModInfoRetrieval:
;
; > event OnModInfoRetrieval(int request_id, bool request_failed, string mod_name, string mod_version, string mod_last_updated, string mod_added, string mod_downloads, string mod_views)
; > endEvent
;
; if request_failed is set to true:
;	mod_ parameter contains reason, which will be one of the following values:
;
;	_REQUEST_FAILED_: Request couldn't be processed properly. See the log for more details.
;	_PARSING_FAILED_: Source code of the website has changed and therefore the parsing process failed. Contact me if that happens for an update.
;	_ADULT-ONLY_: Nexus only. Happens if the mod is marked as adult only. There is no way around that barrier without accound details which I won't request of you. 
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
; Encodes string into an URL encoded one. For further information see: https://curl.haxx.se/libcurl/c/curl_easy_escape.html
;
; @param 	string	raw string
; @return	string	URL encoded string
;
;
string function URLEncode(string raw_str) global native


;
; Decodes an URL encoded string into a normal string. For further information see: https://curl.haxx.se/libcurl/c/curl_easy_unescape.html
;
; @param 	string	encoded string
; @return	string	URL decoded string
;
;
string function URLDecode(string encoded_str) global native

;
; Starts request to retrieve mod information
;
; @param	Form	Script instance on which the stated events will be registered and called.
; @param	string	Nexus mod ID ("www.nexusmods.com/skyrim/mods/<mod_id>". Example: "www.nexusmods.com/skyrim/mods/123456/", 123456 would be the id of the mod.)
; @return	int	Request id for event 'OnModInfoRetrieval'
;
int function GetNexusModInfo(Form form, string nexus_mod_id) global native

;
; Starts request to retrieve mod information
;
; @param	Form	Script instance on which the stated events will be registered and called.
; @param	string	LoversLab mod ID ("www.loverslab.com/files/file/<mod_id>-some-mods-name/". Example: ""www.loverslab.com/files/file/123456-some-mods-name/", 123456 would be the id of the mod.)
; @return	int	Request id for event 'OnModInfoRetrieval'
;
int function GetLLabModInfo(Form form, string ll_mod_id) global native


