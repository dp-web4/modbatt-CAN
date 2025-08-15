// ===========================================================================
// Web4 Race Car Demo - CORRECTED C++ RAD Studio SDK
// ===========================================================================
//
// Fixed implementation matching actual Web4 API endpoints and patterns
// Based on racecar-web chain with proper endpoint naming
// Platform: RAD Studio 12+ (Windows/macOS/Linux/Mobile)
// Dependencies: REST components, JSON, VCL/FMX
// ===========================================================================

#pragma hdrstop
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
#include "File2.h"

// ===========================================================================
// WEB4 CORRECTED CLIENT CONFIGURATION
// ===========================================================================

/// <summary>
/// Corrected configuration for Web4 client
/// </summary>
class TWeb4ClientConfig
{
public:
    String RestEndpoint;
    String GrpcEndpoint;
    String ChainId;
    String DemoAccount;
    int RequestTimeoutSeconds;
    int MaxRetryAttempts;

    TWeb4ClientConfig()
    {
        RestEndpoint = "http://localhost:1317";
        GrpcEndpoint = "localhost:9090";
        ChainId = "racecar-web";  // ✅ CORRECTED: Matches actual chain
        DemoAccount = "cosmos1k0kju6a63dmfz9dhjx0ralxp60fewtf29wwd9k";
        RequestTimeoutSeconds = 30;
        MaxRetryAttempts = 3;
    }
};

// ===========================================================================
// WEB4 CORRECTED C++ RAD STUDIO CLIENT
// ===========================================================================

class TWeb4Client
{
private:
    std::unique_ptr<TRESTClient> FRESTClient;
    std::unique_ptr<TRESTRequest> FRESTRequest;
    std::unique_ptr<TRESTResponse> FRESTResponse;
    TWeb4ClientConfig FConfig;

    // Helper method for making HTTP requests with proper error handling
    TJSONObject* MakeRequest(const String& method, const String& endpoint, TJSONObject* data = nullptr)
    {
        try
        {
            FRESTClient->BaseURL = FConfig.RestEndpoint;
            FRESTRequest->Resource = endpoint;
            FRESTRequest->Method = StringToRESTRequestMethod(method);

            // Clear previous parameters
            FRESTRequest->Params->Clear();

            if (data && (method == "POST" || method == "PUT"))
            {
                FRESTRequest->AddBody(data->ToJSON(), TRESTContentType::ctAPPLICATION_JSON);
            }

            FRESTRequest->Execute();

            // ✅ IMPROVED: Better error handling
            if (FRESTResponse->StatusCode >= 200 && FRESTResponse->StatusCode < 300)
            {
                if (FRESTResponse->Content.IsEmpty())
                {
                    // Return empty object for successful empty responses
                    return new TJSONObject();
                }
                return dynamic_cast<TJSONObject*>(TJSONObject::ParseJSONValue(FRESTResponse->Content));
            }
            else if (FRESTResponse->StatusCode == 404)
            {
                return nullptr; // Not found
            }
            else
            {
                throw Exception("HTTP Error: " + IntToStr(FRESTResponse->StatusCode) + " - " + FRESTResponse->StatusText + "\nResponse: " + FRESTResponse->Content);
            }
        }
        catch (const Exception& e)
        {
            throw Exception("Request failed: " + e.Message);
        }
    }

    TRESTRequestMethod StringToRESTRequestMethod(const String& method)
    {
        if (method == "GET") return TRESTRequestMethod::rmGET;
        if (method == "POST") return TRESTRequestMethod::rmPOST;
        if (method == "PUT") return TRESTRequestMethod::rmPUT;
        if (method == "DELETE") return TRESTRequestMethod::rmDELETE;
        return TRESTRequestMethod::rmGET;
    }

    // ✅ NEW: URL encoding helper
    String URLEncode(const String& value)
    {
        return TNetEncoding::URL->Encode(value);
    }

public:
    TWeb4Client(const TWeb4ClientConfig& config = TWeb4ClientConfig())
        : FConfig(config)
    {
        FRESTClient = std::make_unique<TRESTClient>(nullptr);
        FRESTRequest = std::make_unique<TRESTRequest>(nullptr);
        FRESTResponse = std::make_unique<TRESTResponse>(nullptr);

        FRESTRequest->Client = FRESTClient.get();
        FRESTRequest->Response = FRESTResponse.get();
        FRESTClient->ConnectTimeout = FConfig.RequestTimeoutSeconds * 1000;
        FRESTClient->ReadTimeout = FConfig.RequestTimeoutSeconds * 1000;
    }

    // =======================================================================
    // CORRECTED COMPONENT REGISTRY METHODS
    // =======================================================================

    /// <summary>
    /// Register a new component in the Web4 system
    /// </summary>
    TJSONObject* RegisterComponent(const String& componentId, const String& componentType, const String& manufacturerData = "")
    {
        auto message = new TJSONObject();
        // ✅ CORRECTED: Proper message type
        message->AddPair("@type", "/racecar-web.componentregistry.v1.MsgRegisterComponent");
        message->AddPair("creator", FConfig.DemoAccount);
        message->AddPair("component_id", componentId);
        message->AddPair("component_type", componentType);
        message->AddPair("manufacturer_data", manufacturerData);

        auto tx = new TJSONObject();
        auto body = new TJSONObject();
        auto messages = new TJSONArray();
        messages->AddElement(message);
        body->AddPair("messages", messages);
        tx->AddPair("body", body);

        auto request = new TJSONObject();
        request->AddPair("tx", tx);
        request->AddPair("mode", "BROADCAST_MODE_SYNC");

        return MakeRequest("POST", "/cosmos/tx/v1beta1/txs", request);
    }

    /// <summary>
    /// Retrieve component identity information
    /// </summary>
	std::unique_ptr<TComponentIdentity> GetComponent(const String& componentId)
    {
        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            auto response = MakeRequest("GET", "/racecar-web/componentregistry/v1/get_component/" + URLEncode(componentId));

            if (response && response->GetValue("component_identity"))
            {
                auto component = std::make_unique<TComponentIdentity>();
                component->FromJSON(dynamic_cast<TJSONObject*>(response->GetValue("component_identity")));
                return component;
            }
        }
        catch (const Exception& e)
        {
            // Log error but don't throw - component might not exist
        }

        return nullptr;
    }

    /// <summary>
    /// Check bidirectional pairing authorization
    /// </summary>
    std::unique_ptr<TPairingAuthorizationResult> CheckPairingAuthorization(const String& componentA, const String& componentB)
    {
        auto result = std::make_unique<TPairingAuthorizationResult>();

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern with path parameters
            String endpoint = "/racecar-web/componentregistry/v1/check_pairing_auth/" +
                            URLEncode(componentA) + "/" + URLEncode(componentB);
            auto response = MakeRequest("GET", endpoint);

            if (response)
            {
                result->FromJSON(response);
            }
        }
        catch (const Exception& e)
        {
            result->Reason = "Authorization check failed: " + e.Message;
        }

        return result;
    }

    /// <summary>
    /// List authorized partners for a component
    /// </summary>
    std::vector<String> ListAuthorizedPartners(const String& componentId)
    {
        std::vector<String> partners;

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/componentregistry/v1/list_authorized_partners/" + URLEncode(componentId);
            auto response = MakeRequest("GET", endpoint);

            if (response && response->GetValue("authorized_components"))
            {
                String authComponents = response->GetValue("authorized_components")->Value();
                // Parse comma-separated string into vector
                TStringList* list = new TStringList();
                try
                {
                    list->CommaText = authComponents;
                    for (int i = 0; i < list->Count; i++)
                    {
                        partners.push_back(list->Strings[i]);
                    }
                }
                __finally
                {
                    delete list;
                }
            }
        }
        catch (const Exception& e)
        {
            // Log error
        }

        return partners;
    }

    // =======================================================================
    // CORRECTED LCT MANAGER METHODS
    // =======================================================================

    /// <summary>
    /// Create a new LCT relationship between two components
    /// </summary>
    TJSONObject* CreateLctRelationship(const String& componentA, const String& componentB,
                                      const String& context = "race_car_operation", const String& proxyId = "")
    {
        auto message = new TJSONObject();
        // ✅ CORRECTED: Proper message type
        message->AddPair("@type", "/racecar-web.lctmanager.v1.MsgCreateLctRelationship");
        message->AddPair("creator", FConfig.DemoAccount);
        message->AddPair("component_a", componentA);
        message->AddPair("component_b", componentB);
        message->AddPair("context", context);
        message->AddPair("proxy_id", proxyId);

        auto tx = new TJSONObject();
        auto body = new TJSONObject();
        auto messages = new TJSONArray();
        messages->AddElement(message);
        body->AddPair("messages", messages);
        tx->AddPair("body", body);

        auto request = new TJSONObject();
        request->AddPair("tx", tx);
        request->AddPair("mode", "BROADCAST_MODE_SYNC");

        return MakeRequest("POST", "/cosmos/tx/v1beta1/txs", request);
    }

    /// <summary>
    /// Retrieve LCT relationship information
    /// </summary>
    std::unique_ptr<TLinkedContextToken> GetLctRelationship(const String& lctId)
    {
        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            auto response = MakeRequest("GET", "/racecar-web/lctmanager/v1/get_lct/" + URLEncode(lctId));

            if (response && response->GetValue("linked_context_token"))
            {
                auto lct = std::make_unique<TLinkedContextToken>();
                lct->FromJSON(dynamic_cast<TJSONObject*>(response->GetValue("linked_context_token")));
                return lct;
            }
        }
        catch (const Exception& e)
        {
            // Log error: Could not retrieve LCT
        }

        return nullptr;
    }

    /// <summary>
    /// Get all LCT relationships for a component
    /// </summary>
    std::vector<std::unique_ptr<TLinkedContextToken>> GetComponentRelationships(const String& componentId)
    {
        std::vector<std::unique_ptr<TLinkedContextToken>> relationships;

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/lctmanager/v1/get_component_relationships/" + URLEncode(componentId);
            auto response = MakeRequest("GET", endpoint);

            if (response && response->GetValue("component_relationships"))
            {
                String relationshipsData = response->GetValue("component_relationships")->Value();
                // Parse relationships data (format may vary)
                // Implementation depends on actual API response format
            }
        }
        catch (const Exception& e)
        {
            // Log error: Could not retrieve relationships
        }

        return relationships;
    }

    /// <summary>
    /// Validate LCT access permissions
    /// </summary>
    bool ValidateLctAccess(const String& lctId, const String& requestorId)
    {
        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/lctmanager/v1/validate_lct_access/" +
                            URLEncode(lctId) + "/" + URLEncode(requestorId);
            auto response = MakeRequest("GET", endpoint);

            if (response && response->GetValue("has_access"))
            {
                return response->GetValue("has_access")->Value() == "true";
            }
        }
        catch (const Exception& e)
        {
            // Log error
        }

        return false;
    }

    // =======================================================================
    // CORRECTED PAIRING METHODS
    // =======================================================================

    /// <summary>
    /// Initiate bidirectional pairing between two components
    /// </summary>
    TJSONObject* InitiateBidirectionalPairing(const String& componentA, const String& componentB,
                                             const String& operationalContext = "race_car_operation",
                                             const String& proxyId = "", bool forceImmediate = true)
    {
        auto message = new TJSONObject();
        // ✅ CORRECTED: Proper message type
        message->AddPair("@type", "/racecar-web.pairing.v1.MsgInitiateBidirectionalPairing");
        message->AddPair("creator", FConfig.DemoAccount);
        message->AddPair("component_a", componentA);
        message->AddPair("component_b", componentB);
        message->AddPair("operational_context", operationalContext);
        message->AddPair("proxy_id", proxyId);
        message->AddPair("force_immediate", forceImmediate ? "true" : "false");

        auto tx = new TJSONObject();
        auto body = new TJSONObject();
        auto messages = new TJSONArray();
        messages->AddElement(message);
        body->AddPair("messages", messages);
        tx->AddPair("body", body);

        auto request = new TJSONObject();
        request->AddPair("tx", tx);
        request->AddPair("mode", "BROADCAST_MODE_SYNC");

        return MakeRequest("POST", "/cosmos/tx/v1beta1/txs", request);
    }

    /// <summary>
    /// Validate bidirectional authorization
    /// </summary>
    std::unique_ptr<TPairingAuthorizationResult> ValidateBidirectionalAuth(const String& componentA, const String& componentB, const String& context = "")
    {
        auto result = std::make_unique<TPairingAuthorizationResult>();

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/pairing/v1/validate_bidirectional_auth/" +
                            URLEncode(componentA) + "/" + URLEncode(componentB);
            if (!context.IsEmpty())
            {
                endpoint += "?context=" + URLEncode(context);
            }

            auto response = MakeRequest("GET", endpoint);

            if (response)
            {
                result->FromJSON(response);
            }
        }
        catch (const Exception& e)
        {
            result->Reason = "Bidirectional auth validation failed: " + e.Message;
        }

        return result;
    }

    // =======================================================================
    // CORRECTED TRUST TENSOR METHODS
    // =======================================================================

    /// <summary>
    /// Create a trust tensor for an LCT relationship
    /// </summary>
    TJSONObject* CreateRelationshipTensor(const String& lctId, const String& tensorType = "T3",
                                         const String& context = "race_car_operation")
    {
        auto message = new TJSONObject();
        // ✅ CORRECTED: Proper message type
        message->AddPair("@type", "/racecar-web.trusttensor.v1.MsgCreateRelationshipTensor");
        message->AddPair("creator", FConfig.DemoAccount);
        message->AddPair("lct_id", lctId);
        message->AddPair("tensor_type", tensorType);
        message->AddPair("context", context);

        auto tx = new TJSONObject();
        auto body = new TJSONObject();
        auto messages = new TJSONArray();
        messages->AddElement(message);
        body->AddPair("messages", messages);
        tx->AddPair("body", body);

        auto request = new TJSONObject();
        request->AddPair("tx", tx);
        request->AddPair("mode", "BROADCAST_MODE_SYNC");

        return MakeRequest("POST", "/cosmos/tx/v1beta1/txs", request);
    }

    /// <summary>
    /// Calculate composite trust score for an LCT relationship
    /// </summary>
    std::unique_ptr<TTrustCalculationResult> CalculateRelationshipTrust(const String& lctId,
                                                                      const String& context = "race_car_operation")
    {
        auto result = std::make_unique<TTrustCalculationResult>();

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/trusttensor/v1/get_relationship_trust/" + URLEncode(lctId);
            if (!context.IsEmpty())
            {
                endpoint += "?context=" + URLEncode(context);
            }

            auto response = MakeRequest("GET", endpoint);

            if (response)
            {
                result->FromJSON(response);
            }
        }
        catch (const Exception& e)
        {
            result->TrustScore = 0.5;
            result->Factors = "error: " + e.Message;
        }

        return result;
    }

    // =======================================================================
    // CORRECTED ENERGY CYCLE METHODS
    // =======================================================================

    /// <summary>
    /// Create energy operation between LCT relationships
    /// </summary>
    TJSONObject* CreateEnergyOperation(const String& sourceLct, const String& targetLct, double energyAmount,
                                      const String& operationType = "discharge")
    {
        auto message = new TJSONObject();
        // ✅ CORRECTED: Proper message type
        message->AddPair("@type", "/racecar-web.energycycle.v1.MsgCreateRelationshipEnergyOperation");
        message->AddPair("creator", FConfig.DemoAccount);
        message->AddPair("source_lct", sourceLct);
        message->AddPair("target_lct", targetLct);
        message->AddPair("energy_amount", FloatToStrF(energyAmount, ffFixed, 15, 3));
        message->AddPair("operation_type", operationType);

        auto tx = new TJSONObject();
        auto body = new TJSONObject();
        auto messages = new TJSONArray();
        messages->AddElement(message);
        body->AddPair("messages", messages);
        tx->AddPair("body", body);

        auto request = new TJSONObject();
        request->AddPair("tx", tx);
        request->AddPair("mode", "BROADCAST_MODE_SYNC");

        return MakeRequest("POST", "/cosmos/tx/v1beta1/txs", request);
    }

    /// <summary>
    /// Get energy balance for an LCT relationship
    /// </summary>
    std::unique_ptr<TEnergyBalance> GetEnergyBalance(const String& lctId)
    {
        auto balance = std::make_unique<TEnergyBalance>();

        try
        {
            // ✅ CORRECTED: Proper API endpoint pattern
            String endpoint = "/racecar-web/energycycle/v1/get_relationship_energy_balance/" + URLEncode(lctId);
            auto response = MakeRequest("GET", endpoint);

            if (response)
            {
                balance->FromJSON(response);
            }
        }
        catch (const Exception& e)
        {
            // Log error: Could not retrieve energy balance
        }

        return balance;
    }

    // =======================================================================
    // CORRECTED UTILITY METHODS
    // =======================================================================

    /// <summary>
    /// Get basic Web4 blockchain information
    /// </summary>
    TJSONObject* GetBlockchainInfo()
    {
        return MakeRequest("GET", "/cosmos/base/tendermint/v1beta1/node_info");
    }

    /// <summary>
    /// Health check - test connectivity
    /// </summary>
    bool HealthCheck()
    {
        try
        {
            auto info = GetBlockchainInfo();
            return info != nullptr;
        }
        catch (...)
        {
            return false;
        }
    }

    /// <summary>
    /// Wait for transaction to be confirmed
    /// </summary>
    TJSONObject* WaitForTransaction(const String& txHash, int timeoutSeconds = 30)
    {
        TDateTime startTime = Now();
        double timeout = timeoutSeconds / 86400.0; // Convert to TDateTime fraction

        while ((Now() - startTime) < timeout)
        {
            try
            {
                auto result = MakeRequest("GET", "/cosmos/tx/v1beta1/txs/" + txHash);

                if (result && result->GetValue("tx_response"))
                {
                    auto txResponse = dynamic_cast<TJSONObject*>(result->GetValue("tx_response"));
                    if (txResponse && StrToIntDef(txResponse->GetValue("code")->Value(), -1) == 0)
                    {
                        return result;
                    }
                }
            }
            catch (...)
            {
                // Continue polling
            }

            Sleep(1000); // Wait 1 second
        }

        throw Exception("Transaction " + txHash + " not confirmed within " + IntToStr(timeoutSeconds) + " seconds");
    }
};
