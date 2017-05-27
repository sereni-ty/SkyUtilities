#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <curl/curl.h>

#include <regex>

namespace SKU::Net::HTTP {

	void NexusModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request)
	{
		Plugin::Log(LOGL_VERBOSE, "NexusModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());

		PapyrusEvent::Args args = {

		};



		PapyrusEventHandler::GetInstance()->Send(Net::Interface::GetEventString(Net::Interface::evModInfoRetrieval), std::move(args));
	}

	void LLabModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request) // regular expressions working, 27.05.2017
	{
		const std::regex 
			mod_name_exp( 
				"Download[\r\n\t ]{0,}"\
				"<[\t ]{0,}\\/[\t ]{0,}a>[\r\n\t ]{0,}"\
				"(.*?)[\r\n\t ]{0,}"\
				"<[\t ]{0,}\\/[\t ]{0,}h1>"
			),
			
			mod_version_exp(
				"<span[\t ]{0,}id=\"file_version\">(.*?)<\\/span>"
			),
			
			mod_last_updated_exp(
				"<strong[\t ]{0,}class[\t ]{0,}=[\t ]{0,}'title'>Last Updated[\t ]{0,}:[\t ]{0,}<[\t ]{0,}\\/[\t ]{0,}strong>[\r\n\t ]{0,}"
				"(.*?)[\r\n\t ]{0,}"
				"<[\t ]{0,}\\/[\t ]{0,}li>"
			),

			mod_added_exp(
				"<strong[\t ]{0,}class[\t ]{0,}=[\t ]{0,}'title'>Submitted:[\t ]{0,}<[\t ]{0,}\\/[\t ]{0,}strong>[\r\n\t ]{0,}"
				"(.*?)[\r\n\t ]{0,}"
				"<[\t ]{0,}\\/[\t ]{0,}li>"
			),
			
			mod_downloads_exp(
				"<strong[\t ]{0,}class[\t ]{0,}=[\t ]{0,}'title'>Downloads:[\t ]{0,}<[\t ]{0,}\\/[\t ]{0,}strong>[\r\n\t ]{0,}"
				"(.*?)[\r\n\t ]{0,}"
				"<[\t ]{0,}\\/[\t ]{0,}li>"
			),
			
			mod_views_exp(
				"<strong[\t ]{0,}class[\t ]{0,}=[\t ]{0,}'title'>Views:[\t ]{0,}<[\t ]{0,}\\/[\t ]{0,}strong>[\r\n\t ]{0,}"
				"(.*?)[\r\n\t ]{0,}"
				"<[\t ]{0,}\\/[\t ]{0,}li>"
			);

		std::smatch 
			mod_name_match,
			mod_version_match,
			mod_last_updated_match,
			mod_added_match,
			mod_downloads_match,
			mod_views_match;

		RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
		std::string response = ctx->GetResponse();

		PapyrusEvent::Args args = {
			std::make_any<int>(request->GetID()),
			std::make_any<bool>(request->GetState() != Request::sOK),
		};		

		Plugin::Log(LOGL_VERBOSE, "LLabModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());
		
		if (std::regex_search(response, mod_name_match, mod_name_exp)) // name
		{
			args.push_back(std::make_any<std::string>(mod_name_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>("")); 

		if (std::regex_search(response, mod_version_match, mod_version_exp)) // version
		{
			args.push_back(std::make_any<std::string>(mod_version_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>(""));

		if (std::regex_search(response, mod_last_updated_match, mod_last_updated_exp)) // mod last updated
		{
			args.push_back(std::make_any<std::string>(mod_last_updated_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>(""));

		if (std::regex_search(response, mod_added_match, mod_added_exp)) // mod added
		{
			args.push_back(std::make_any<std::string>(mod_added_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>(""));

		if (std::regex_search(response, mod_downloads_match, mod_downloads_exp)) // mod downloads
		{
			args.push_back(std::make_any<std::string>(mod_downloads_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>(""));

		if (std::regex_search(response, mod_views_match, mod_views_exp)) // mod views
		{
			args.push_back(std::make_any<std::string>(mod_views_match[1].str()));
		}
		else
			args.push_back(std::make_any<std::string>(""));
		
		PapyrusEventHandler::GetInstance()->Send(Net::Interface::GetEventString(Net::Interface::evModInfoRetrieval), std::move(args));
	}

}
