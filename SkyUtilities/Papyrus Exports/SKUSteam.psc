scriptname SKUSteam hidden

; =========================================================
; FUNCTIONS (STATIC)
; =========================================================
;
;
; Get the logged on Steam User ID
;
; @return	string  Steam User ID or empty string on failure or if restricted
;
string function CurrentSteamUserProfileID() global native