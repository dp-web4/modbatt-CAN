#ifndef File2H
#define File2H

#include <System.hpp>
#include <System.Classes.hpp>
#include <System.SysUtils.hpp>
#include <REST.Client.hpp>
#include <REST.Response.Adapter.hpp>
#include <Data.Bind.Components.hpp>
#include <System.JSON.hpp>
#include <System.StrUtils.hpp>
#include <System.DateUtils.hpp>
#include <System.NetEncoding.hpp>
#include <memory>
#include <vector>
#include <map>
#include <functional>


class TWeb4ClientConfig
{
public:
    String RestEndpoint;
    String GrpcEndpoint;
    String ChainId;
    String DemoAccount;
    int RequestTimeoutSeconds;
    int MaxRetryAttempts;

	TWeb4ClientConfig();
};

class TWeb4Client
{
private:

	std::unique_ptr<TRESTClient> FRESTClient;
    std::unique_ptr<TRESTRequest> FRESTRequest;
	std::unique_ptr<TRESTResponse> FRESTResponse;
	TWeb4ClientConfig FConfig;

	TJSONObject* MakeRequest(const String& method, const String& endpoint, TJSONObject* data = nullptr);
	TRESTRequestMethod StringToRESTRequestMethod(const String& method);

public:
	TWeb4Client(const TWeb4ClientConfig& config = TWeb4ClientConfig());
	TJSONObject* RegisterComponent(const String& componentId, const String& componentType, const String& manufacturerData = "");



};




#endif
