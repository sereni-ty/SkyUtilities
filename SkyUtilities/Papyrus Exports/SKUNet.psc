scriptname SKUNet hidden

;
; Performs a HTTP POST request
;
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param 	string	Data to post. Make sure you encode the values if they contain illegal characters. See https://curl.haxx.se/libcurl/c/curl_easy_escape.html for more details
; @param	float	Timeout in 'seconds.milliseconds'. Time how long the post may take tops.
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPPOSTRequest(string url, string data, int timeout) global native

;
; Performs a HTTP GET request.
;
; @param 	string	URL. See https://curl.haxx.se/libcurl/c/CURLOPT_URL.html for more details
; @param	float	Timeout in 'seconds.milliseconds'. Time how long the post may take tops.
; @return	int	Error code (CURL error is logged) or positive integer (Request ID)
;
int function HTTPGETRequest(string url, int timeout) global native

;
; Returns buffered data (Currently only needed for HTTP GET/POST requests)
;
; @param	int	Request ID
; @return	string	Request data
;
string function GetBufferedData(int id) global native

;
; @param 	string	Unencoded string
; @return	string	URL encoded string
;
string function URLEncode(string data) global native

;
; Returns an error code if there has an error occurred while loading / storing data
;
; -255: Fatal error. Might happen if a library internally fails, we're OOM or.. 
; -254: Internal error occured. Most probably something couldn't be initialized. This _really_ shouldn't happen
; -124: Invalid ID supplied
; -123: Timeout exceeded
;   -1: Request failed. Check logs for more details on the error code.
;
;	 1: Pending (Still working)
;    0: No error
; 
; @param	int	Request ID
; @return	int	Error / Result code
;
int function GetStatus(int id) global native

;
; Call this if you don't need any data from that request anymore. This function only succeeds if the request has ended. Make sure you call it if the state is not pending
;
; @param	int	Request IDa
;
bool function EndRequest(int id) global native
