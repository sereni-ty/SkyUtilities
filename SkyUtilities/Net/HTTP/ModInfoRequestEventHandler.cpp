#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <regex>
#include <cctype>
#include <fstream>

// TODO: if requests have failed..
// TODO: (nexus) adult content barrier
namespace SKU::Net::HTTP {
  inline void reformat_response(std::string &response)
  {
    char s_inb_qmark = false;
    bool s_tag = false;
    bool s_last_was_alpha = false;

    for (size_t pos = 0; pos < response.size(); pos++)
    {
      switch (response[pos])
      {
        case '<':
        {
          if (s_inb_qmark == false)
          {
            s_tag = true;
          }
        } break;

        case '>':
        {
          if (s_inb_qmark == false)
          {
            s_tag = false;
          }
        } break;

        case '\t':
        {
          response[pos] = ' ';
        } break;

        case '\'':
        case '"':
        {
          if (s_tag == true
            && pos > 0 && response[pos - 1] == '=')
          {
            s_inb_qmark = response[pos];

            if (s_inb_qmark == '\'')
            {
              response[pos] = '"';
            }
          }
          else if (s_tag == true
            && (pos + 1) < response.length() && response[pos + 1]
            && s_inb_qmark == response[pos])
          {
            s_inb_qmark = false;
            response[pos] = '"';
          }
        } break;
      }

      char before = response[pos];

      if (s_tag == true && s_inb_qmark == false && std::isspace(response[pos]) == true)
      {
        if (s_last_was_alpha == false
          || ((pos + 1) < response.length() && response[pos + 1] == '='))
        {
          response.erase(pos--);
        }
      }

      if (std::isalpha(before) == true)
      {
        s_last_was_alpha = true;
      }
    }
  }

  void NexusModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request) // regular expressions working, 29.05.2017
  {
    const std::regex
      adult_only_exp(
        "<h2>Adult-only content<\\/h2>"
      ),

      mod_name_exp(
        "<span[ ]?class=\"header\\-name\">(.*?)<\\/span>"
      ),

      mod_version_exp(
        "<p[ ]?class=\"file\\-version\">[\r\n]{0,}"
        "<strong>(.*?)<\\/strong>"
      ),

      mod_dates_exp(
        "<p[ ]?class=\"sub\\-header\">[\r\n]{0,}"
        "<span[ ]?class=\"left\">Last updated at[ ]?(.*?)<\\/span>[\r\n]{0,}"
        "<span[ ]?class=\"right\">Uploaded at[ ]?(.*?)<\\/span>[\r\n]{0,}"
        "<\\/p>"
      ),

      mod_downloads_exp(
        "<p[ ]?class=\"file\\-total\\-dls\">[\r\n]{0,}"
        "<strong>(.*?)<\\/strong>"
      ),

      mod_views_exp(
        "<p[ ]?class=\"file\\-total\\-views\">[\r\n]{0,}"
        "<strong>(.*?)<\\/strong>"
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

    reformat_response(response);

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
      fail_value = "_ADULT-ONLY_";
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
        "Download[\r\n]{0,}"\
        "<\\/a>[\r\n]{0,}"\
        "(.*?)[\r\n]{0,}"\
        "<\\/h1>"
      ),

      mod_version_exp(
        "<span[ ]?id=\"file_version\">(.*?)<\\/span>"
      ),

      mod_last_updated_exp(
        "<strong[ ]?class=\"title\">Last Updated[ ]?:[ ]?<\\/strong>[\r\n]{0,}"
        "(.*?)[\r\n]{0,}"
        "<\\/li>"
      ),

      mod_added_exp(
        "<strong[ ]?class=\"title\">Submitted:[ ]?<\\/strong>[\r\n]{0,}"
        "(.*?)[\r\n]{0,}"
        "<\\/li>"
      ),

      mod_downloads_exp(
        "<strong[ ]?class=\"title\">Downloads:[ ]?<\\/strong>[\r\n]{0,}"
        "(.*?)[\r\n]{0,}"
        "<\\/li>"
      ),

      mod_views_exp(
        "<strong[ ]?class=\"title\">Views:[ ]?<\\/strong>[\r\n]{0,}"
        "(.*?)[\r\n]{0,}"
        "<\\/li>"
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

    reformat_response(response);

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

      fail_value.clear();

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