scriptname SKUPluginMCM extends SKI_ConfigBase

; SLM MCM VERSION
; =========================================================
int SKU_MCM_Version = 1

int function GetVersion()
	return SKU_MCM_Version
endFunction

; GLOBALS
; =========================================================





; MAIN EVENTS 
; =========================================================
event OnConfigInit()
  ; MCM INIT
  ;
  ModName = "SkyUtilities"

  Pages = new string[1]
  Pages[0] = "$SKU_PAGE_TITLE"
endEvent

event OnVersionUpdate(int new_version)
	
endEvent

event OnConfigRegister()
	
endEvent

; EVENTS 
; =========================================================
event OnPageReset(string requested_page)
	if requested_page == "" \
	|| requested_page == "$SKU_PAGE_TITLE"

		DisplayPageTitle()

	endIf

endEvent

; PAGES
; =========================================================
function DisplayPageTitle()
  SetTitleText("SkyUtilities")
  AddHeaderOption("$SKU_PAGE_TITLE_HEADER")
endFunction


; OPTION STATES
; =========================================================

