scriptname SKUStringUtil hidden

; =========================================================
; FUNCTIONS (STATIC)
; =========================================================
;

;
; Searches the string for the given pattern
;
; @param	string	Regular Expression. See: http://www.cplusplus.com/reference/regex/ECMAScript/
; @param	string	String pattern is used for
; @return	string[] Found matches 
;
string[] function RegexSearch(string pattern, string str) global native

;
; Matches the string against the pattern
;
; @param	string	Regular Expression. See: http://www.cplusplus.com/reference/regex/ECMAScript/
; @param	string	String pattern is used for
; @return	string[] Found matches 
;
string[] function RegexMatch(string pattern, string str) global native

;
; Replaces the part of the string where the pattern matches
;
; @param	string	Regular Expression. See: http://www.cplusplus.com/reference/regex/ECMAScript/
; @param	string	Replacement if pattern was found in string
; @param	string	String pattern is used for
; @return	string[] Matches found
;
string function RegexReplace(string pattern, string replacement, string str) global native

