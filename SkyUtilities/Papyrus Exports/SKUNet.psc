scriptname SKUNet hidden

;
; Performs a HTTP POST request
;
; @param	Form	..
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param 	string	Data to post. Make sure you encode the values if they contain illegal characters. See https://curl.haxx.se/libcurl/c/curl_easy_escape.html for more details
; @param	int		Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPPOSTRequest(Form form, string url, string data, int timeout) global native

;
; Performs a HTTP GET request.
;
; @param	Form	..
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param	float	Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPGETRequest(Form form, string url, int timeout) global native

;
;
; @param 	string	Unencoded string
; @return	string	URL encoded string
;
; -- Not yet implemented
;
;string function URLEncode(string data) global native

;
; Returns an error status text 
;
; -- Not yet implemented
; 
; @param	int	Request ID
; @return	string 
;
;string function GetStatusString(int id) global native

