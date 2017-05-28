#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <regex>

// TODO: if requests have failed..
// TODO: (nexus) adult content barrier
namespace SKU::Net::HTTP {

	void NexusModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request) // regular expressions partially working (adult only the exception), 28.05.2017
	{
		const std::regex
			adult_only_exp(
				"<[\t ]{0,}\\/[\t ]{0,}p><h2>Adult[\t ]{0,}\\-[\t ]{0,}only[\t ]{0,}content<[\t ]{0,}\\/[\t ]{0,}h2>"
			),

			mod_name_exp(
				"<span[\t ]{0,}class=\"header\\-name\">(.*?)<\\/span>"
			),

			mod_version_exp(
				"<p[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"file\\-version\">[\r\n\t ]{0,}"
				"<strong>(.*?)<[\t ]{0,}\\/[\t ]{0,}strong>"
			),

			mod_dates_exp(
				"<p[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"sub\\-header\">[\r\n\t ]{0,}"
				"<span[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"left\">Last updated at[\t ]{0,}(.*?)<[\t ]{0,}\\/[\t ]{0,}span>[\r\n\t ]{0,}"
				"<span[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"right\">Uploaded at[\t ]{0,}(.*?)<[\t ]{0,}\\/[\t ]{0,}span>[\r\n\t ]{0,}"
				"<[\t ]{0,}\\/[\t ]{0,}p>"
			),

			mod_downloads_exp(
				"<p[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"file\\-total\\-dls\">[\r\n\t ]{0,}"
				"<strong>(.*?)<[\t ]{0,}\\/[\t ]{0,}strong>"
			),

			mod_views_exp(
				"<p[\t ]{0,}class[\t ]{0,}=[\t ]{0,}\"file\\-total\\-views\">[\r\n\t ]{0,}"
				"<strong>(.*?)<[\t ]{0,}\\/[\t ]{0,}strong>"
			);

		std::smatch
			adult_only_match,
			mod_name_match,
			mod_version_match,
			mod_dates_match,
			mod_downloads_match,
			mod_views_match;

		RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

		std::string response = ctx->GetResponse();
		std::string fail_value;
		
		Plugin::Log(LOGL_VERBOSE, "NexusModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());

		PapyrusEvent::Args args = {
			std::make_any<int>(request->GetID())			
		};

		if (request->GetState() != Request::sOK)
		{
			fail_value = "_REQUEST_FAILED_";
		}
		else if (std::regex_search(response, adult_only_match, adult_only_exp))
		{
			fail_value = "ADULT-ONLY";
		}
		else
		{
			fail_value = "_PARSING_FAILED_";

			if (std::regex_search(response, mod_name_match, mod_name_exp) == false) // name
				goto fail;

			if (std::regex_search(response, mod_version_match, mod_version_exp) == false) // version
				goto fail;

			if (std::regex_search(response, mod_dates_match, mod_dates_exp) == false) // mod last updated, mod added
				goto fail;

			if (std::regex_search(response, mod_downloads_match, mod_downloads_exp) == false) // mod downloads
				goto fail;

			if (std::regex_search(response, mod_views_match, mod_views_exp) == false) // mod views
				goto fail;

			fail_value.clear();

			args.push_back(std::make_any<bool>(true));
			args.push_back(std::make_any<std::string>(mod_name_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_version_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_dates_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_dates_match[2].str()));
			args.push_back(std::make_any<std::string>(mod_downloads_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_views_match[1].str()));
		}

	fail:
		if (fail_value.size())
		{
			args.push_back(std::make_any<bool>(false));

			while (args.size() != 8)
			{
				args.push_back(std::make_any<std::string>(fail_value));
			}
		}

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
		std::string fail_value;

		PapyrusEvent::Args args = {
			std::make_any<int>(request->GetID()),
		};		

		Plugin::Log(LOGL_VERBOSE, "LLabModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());
		
		if (request->GetState() != Request::sOK)
		{
			fail_value = "_REQUEST_FAILED_";
		}
		else
		{
			fail_value = "_PARSING_FAILED_";

			if (std::regex_search(response, mod_name_match, mod_name_exp) == false) // name
				goto fail;

			if (std::regex_search(response, mod_version_match, mod_version_exp) == false) // version
				goto fail;

			if (std::regex_search(response, mod_last_updated_match, mod_last_updated_exp) == false) // mod last updated
				goto fail;

			if (std::regex_search(response, mod_added_match, mod_added_exp) == false) // mod added
				goto fail;

			if (std::regex_search(response, mod_downloads_match, mod_downloads_exp) == false) // mod downloads
				goto fail;

			if (std::regex_search(response, mod_views_match, mod_views_exp) == false) // mod views
				goto fail;

			args.push_back(std::make_any<bool>(true));
			args.push_back(std::make_any<std::string>(mod_name_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_version_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_last_updated_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_added_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_downloads_match[1].str()));
			args.push_back(std::make_any<std::string>(mod_views_match[1].str()));
		}

	fail:
		if (fail_value.size())
		{
			args.push_back(std::make_any<bool>(false));

			while (args.size() != 8)
			{
				args.push_back(std::make_any<std::string>(fail_value));
			}
		}

		PapyrusEventHandler::GetInstance()->Send(Net::Interface::GetEventString(Net::Interface::evModInfoRetrieval), std::move(args));
	}

}
