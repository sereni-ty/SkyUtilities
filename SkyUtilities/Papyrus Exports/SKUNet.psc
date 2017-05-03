scriptname SKUNet hidden

;
; Performs a HTTP POST request
;
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param 	string	Data to post. Make sure you encode the values if they contain illegal characters. See https://curl.haxx.se/libcurl/c/curl_easy_escape.html for more details
; @param	int		Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPPOSTRequest(string url, string data, int timeout) global native

;
; Performs a HTTP GET request.
;
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param	float	Timeout in ms
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPGETRequest(string url, int timeout) global native

;
; Returns buffered data (Currently only needed for HTTP GET/POST requests)
;
; -- Not yet implemented
;
; @param	int	Request ID
; @return	string	Request data
;
;string function GetBufferedData(int id) global native

;
; @param 	string	Unencoded string
; @return	string	URL encoded string
;
; -- Not yet implemented
;
;string function URLEncode(string data) global native

;
; Returns an error code if there has an error occurred while loading / storing data
;
; -- Not yet implemented
; 
; @param	int	Request ID
; @return	int	Error / Result code
;
;int function GetStatus(int id) global native

;
; Call this if you don't need any data from that request anymore.
;
; -- Not yet implemented
;
; @param	int	Request IDa
;
;bool function EndRequest(int id) global native
