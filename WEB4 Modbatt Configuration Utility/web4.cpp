// ===========================================================================
// Web4 Race Car Battery Management - Enhanced C++ RAD Studio SDK Implementation
// Using API Bridge Layer for Better Performance and Abstraction
// ===========================================================================
#pragma hdrstop

#include "WEB4.h"
#include <System.StrUtils.hpp>
#include <System.IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <Windows.hpp>
#include <algorithm>

// ===========================================================================
// HELPER FUNCTIONS AND CONSTANTS
// ===========================================================================

static const System::TDateTime CustomUnixDateDelta = System::Sysutils::EncodeDate(1970, 1, 1);
static const double CustomSecsPerDay = 86400.0;

__int64 MyDateTimeToUnix(System::TDateTime dt) {
    double delta = static_cast<double>(dt - CustomUnixDateDelta);
    return static_cast<__int64>(delta * CustomSecsPerDay);
}

System::TDateTime MyUnixToDateTime(__int64 seconds) {
    double delta = static_cast<double>(seconds) / CustomSecsPerDay;
    return CustomUnixDateDelta + delta;
}

// ===========================================================================
// EXCEPTION IMPLEMENTATION
// ===========================================================================

EWeb4Exception::EWeb4Exception(const System::UnicodeString& msg, int code, const System::UnicodeString& type)
    : System::Sysutils::Exception(msg), ErrorCode(code), ErrorType(type) {
}

// ===========================================================================
// DATA STRUCTURE IMPLEMENTATIONS
// ===========================================================================

// TComponentRegistration
TComponentRegistration::TComponentRegistration() : CreatedAt(System::Sysutils::Now()) {}

void TComponentRegistration::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    ComponentId = TWeb4Utils::SafeJsonString(json->GetValue("component_id"));
    ComponentIdentity = TWeb4Utils::SafeJsonString(json->GetValue("component_identity"));
    LctId = TWeb4Utils::SafeJsonString(json->GetValue("lct_id"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    ComponentData = TWeb4Utils::SafeJsonString(json->GetValue("component_data"));
    Context = TWeb4Utils::SafeJsonString(json->GetValue("context"));
    Creator = TWeb4Utils::SafeJsonString(json->GetValue("creator"));
    TxHash = TWeb4Utils::SafeJsonString(json->GetValue("txhash"));
}

System::Json::TJSONObject* TComponentRegistration::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "component_id", ComponentId);
    TWeb4Utils::AddStringField(json, "component_identity", ComponentIdentity);
    TWeb4Utils::AddStringField(json, "lct_id", LctId);
    TWeb4Utils::AddStringField(json, "status", Status);
    TWeb4Utils::AddStringField(json, "component_data", ComponentData);
    TWeb4Utils::AddStringField(json, "context", Context);
    TWeb4Utils::AddStringField(json, "creator", Creator);
    TWeb4Utils::AddStringField(json, "txhash", TxHash);
    return json;
}

// TLctRelationship
TLctRelationship::TLctRelationship() : CreatedAt(System::Sysutils::Now()) {}

void TLctRelationship::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    Creator = TWeb4Utils::SafeJsonString(json->GetValue("creator"));
    LctId = TWeb4Utils::SafeJsonString(json->GetValue("lct_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    Context = TWeb4Utils::SafeJsonString(json->GetValue("context"));
    ProxyId = TWeb4Utils::SafeJsonString(json->GetValue("proxy_id"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    TxHash = TWeb4Utils::SafeJsonString(json->GetValue("txhash"));
    
    // Extract cryptographic key halves
    DeviceKeyHalf = TWeb4Utils::SafeJsonString(json->GetValue("device_key_half"));
    LctKeyHalf = TWeb4Utils::SafeJsonString(json->GetValue("lct_key_half"));
}

System::Json::TJSONObject* TLctRelationship::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "creator", Creator);
    TWeb4Utils::AddStringField(json, "lct_id", LctId);
    TWeb4Utils::AddStringField(json, "component_a", ComponentA);
    TWeb4Utils::AddStringField(json, "component_b", ComponentB);
    TWeb4Utils::AddStringField(json, "context", Context);
    TWeb4Utils::AddStringField(json, "proxy_id", ProxyId);
    TWeb4Utils::AddStringField(json, "status", Status);
    TWeb4Utils::AddStringField(json, "txhash", TxHash);
    
    // Include cryptographic key halves
    TWeb4Utils::AddStringField(json, "device_key_half", DeviceKeyHalf);
    TWeb4Utils::AddStringField(json, "lct_key_half", LctKeyHalf);
    
    return json;
}

// TTrustTensor
TTrustTensor::TTrustTensor() :
    TrustScore(0.5),
    CreatedAt(System::Sysutils::Now()),
    CalculatedAt(System::Sysutils::Now()),
    LastUpdated(System::Sysutils::Now()),
    EvidenceCount(0),
    LearningRate(0.1) {}

void TTrustTensor::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    TensorId = TWeb4Utils::SafeJsonString(json->GetValue("tensor_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationalContext = TWeb4Utils::SafeJsonString(json->GetValue("operational_context"));
    TrustScore = TWeb4Utils::SafeJsonNumber(json->GetValue("trust_score"), 0.5);
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    TxHash = TWeb4Utils::SafeJsonString(json->GetValue("txhash"));
    EvidenceCount = static_cast<int>(TWeb4Utils::SafeJsonInt64(json->GetValue("evidence_count"), 0));
    LearningRate = TWeb4Utils::SafeJsonNumber(json->GetValue("learning_rate"), 0.1);

    __int64 createdTimestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("created_at"));
    if (createdTimestamp > 0) CreatedAt = MyUnixToDateTime(createdTimestamp);

    __int64 calculatedTimestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("calculated_at"));
    if (calculatedTimestamp > 0) CalculatedAt = MyUnixToDateTime(calculatedTimestamp);

    __int64 lastUpdatedTimestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("last_updated"));
    if (lastUpdatedTimestamp > 0) LastUpdated = MyUnixToDateTime(lastUpdatedTimestamp);
}

System::Json::TJSONObject* TTrustTensor::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "tensor_id", TensorId);
    TWeb4Utils::AddStringField(json, "component_a", ComponentA);
    TWeb4Utils::AddStringField(json, "component_b", ComponentB);
    TWeb4Utils::AddStringField(json, "operational_context", OperationalContext);
    TWeb4Utils::AddNumberField(json, "trust_score", TrustScore);
    TWeb4Utils::AddStringField(json, "status", Status);
    TWeb4Utils::AddStringField(json, "txhash", TxHash);
    TWeb4Utils::AddNumberField(json, "evidence_count", static_cast<double>(EvidenceCount));
    TWeb4Utils::AddNumberField(json, "learning_rate", LearningRate);
    return json;
}

// TQueueRequest
TQueueRequest::TQueueRequest() : CreatedAt(System::Sysutils::Now()) {}

void TQueueRequest::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    RequestId = TWeb4Utils::SafeJsonString(json->GetValue("request_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationalContext = TWeb4Utils::SafeJsonString(json->GetValue("operational_context"));
    ProxyId = TWeb4Utils::SafeJsonString(json->GetValue("proxy_id"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    TxHash = TWeb4Utils::SafeJsonString(json->GetValue("txhash"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("created_at"));
    if (timestamp > 0) CreatedAt = MyUnixToDateTime(timestamp);
}

System::Json::TJSONObject* TQueueRequest::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "request_id", RequestId);
    TWeb4Utils::AddStringField(json, "component_a", ComponentA);
    TWeb4Utils::AddStringField(json, "component_b", ComponentB);
    TWeb4Utils::AddStringField(json, "operational_context", OperationalContext);
    TWeb4Utils::AddStringField(json, "proxy_id", ProxyId);
    TWeb4Utils::AddStringField(json, "status", Status);
    return json;
}

// TQueueStatus
TQueueStatus::TQueueStatus() :
    PendingRequests(0),
    ProcessedRequests(0),
    LastProcessed(System::Sysutils::Now()) {}

void TQueueStatus::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    ComponentId = TWeb4Utils::SafeJsonString(json->GetValue("component_id"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    PendingRequests = static_cast<int>(TWeb4Utils::SafeJsonNumber(json->GetValue("pending_requests")));
    ProcessedRequests = static_cast<int>(TWeb4Utils::SafeJsonNumber(json->GetValue("processed_requests")));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("last_processed"));
    if (timestamp > 0) LastProcessed = MyUnixToDateTime(timestamp);
}

// TPairingAuthorization
TPairingAuthorization::TPairingAuthorization() : CreatedAt(System::Sysutils::Now()) {}

void TPairingAuthorization::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    AuthorizationId = TWeb4Utils::SafeJsonString(json->GetValue("authorization_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationalContext = TWeb4Utils::SafeJsonString(json->GetValue("operational_context"));
    AuthorizationRules = TWeb4Utils::SafeJsonString(json->GetValue("authorization_rules"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));
    TxHash = TWeb4Utils::SafeJsonString(json->GetValue("txhash"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("created_at"));
    if (timestamp > 0) CreatedAt = MyUnixToDateTime(timestamp);
}

System::Json::TJSONObject* TPairingAuthorization::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "authorization_id", AuthorizationId);
    TWeb4Utils::AddStringField(json, "component_a", ComponentA);
    TWeb4Utils::AddStringField(json, "component_b", ComponentB);
    TWeb4Utils::AddStringField(json, "operational_context", OperationalContext);
    TWeb4Utils::AddStringField(json, "authorization_rules", AuthorizationRules);
    TWeb4Utils::AddStringField(json, "status", Status);
    return json;
}

// TAuthorizationCheck
TAuthorizationCheck::TAuthorizationCheck() :
    Authorized(false),
    TrustScore(0.5),
    CheckedAt(System::Sysutils::Now()) {}

void TAuthorizationCheck::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    Authorized = TWeb4Utils::SafeJsonBool(json->GetValue("authorized"));
    AuthorizationId = TWeb4Utils::SafeJsonString(json->GetValue("authorization_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationalContext = TWeb4Utils::SafeJsonString(json->GetValue("operational_context"));
    TrustScore = TWeb4Utils::SafeJsonNumber(json->GetValue("trust_score"), 0.5);
    VerificationStatus = TWeb4Utils::SafeJsonString(json->GetValue("verification_status"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("checked_at"));
    if (timestamp > 0) CheckedAt = MyUnixToDateTime(timestamp);
}

// TEnergyOperation
TEnergyOperation::TEnergyOperation() :
    Amount(0.0),
    CreatedAt(System::Sysutils::Now()) {}

void TEnergyOperation::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    OperationId = TWeb4Utils::SafeJsonString(json->GetValue("operation_id"));
    Creator = TWeb4Utils::SafeJsonString(json->GetValue("creator"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationType = TWeb4Utils::SafeJsonString(json->GetValue("operation_type"));
    Amount = TWeb4Utils::SafeJsonNumber(json->GetValue("amount"));
    Context = TWeb4Utils::SafeJsonString(json->GetValue("context"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("created_at"));
    if (timestamp > 0) CreatedAt = MyUnixToDateTime(timestamp);
}

System::Json::TJSONObject* TEnergyOperation::ToJSON() const {
    System::Json::TJSONObject* json = new System::Json::TJSONObject();
    TWeb4Utils::AddStringField(json, "operation_id", OperationId);
    TWeb4Utils::AddStringField(json, "creator", Creator);
    TWeb4Utils::AddStringField(json, "component_a", ComponentA);
    TWeb4Utils::AddStringField(json, "component_b", ComponentB);
    TWeb4Utils::AddStringField(json, "operation_type", OperationType);
    TWeb4Utils::AddNumberField(json, "amount", Amount);
    TWeb4Utils::AddStringField(json, "context", Context);
    TWeb4Utils::AddStringField(json, "status", Status);
    return json;
}

// TEnergyBalance
TEnergyBalance::TEnergyBalance() :
    AtpBalance(0.0),
    AdpBalance(0.0),
    TotalEnergy(0.0),
    TrustWeightedBalance(0.0),
    LastUpdated(System::Sysutils::Now()) {}

void TEnergyBalance::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    ComponentId = TWeb4Utils::SafeJsonString(json->GetValue("component_id"));
    AtpBalance = TWeb4Utils::SafeJsonNumber(json->GetValue("atp_balance"));
    AdpBalance = TWeb4Utils::SafeJsonNumber(json->GetValue("adp_balance"));
    TotalEnergy = TWeb4Utils::SafeJsonNumber(json->GetValue("total_energy"));
    TrustWeightedBalance = TWeb4Utils::SafeJsonNumber(json->GetValue("trust_weighted_balance"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("last_updated"));
    if (timestamp > 0) LastUpdated = MyUnixToDateTime(timestamp);
}

// TPairingChallenge
TPairingChallenge::TPairingChallenge() : CreatedAt(System::Sysutils::Now()) {}

void TPairingChallenge::FromJSON(System::Json::TJSONObject* json) {
    if (!json) return;
    ChallengeId = TWeb4Utils::SafeJsonString(json->GetValue("challenge_id"));
    ComponentA = TWeb4Utils::SafeJsonString(json->GetValue("component_a"));
    ComponentB = TWeb4Utils::SafeJsonString(json->GetValue("component_b"));
    OperationalContext = TWeb4Utils::SafeJsonString(json->GetValue("operational_context"));
    Status = TWeb4Utils::SafeJsonString(json->GetValue("status"));

    __int64 timestamp = TWeb4Utils::SafeJsonInt64(json->GetValue("created_at"));
    if (timestamp > 0) CreatedAt = MyUnixToDateTime(timestamp);
}

// ===========================================================================
// CONFIGURATION IMPLEMENTATION
// ===========================================================================

TWeb4BridgeConfig::TWeb4BridgeConfig() {
    ApiEndpoint = "http://localhost";
    RestPort = "8082";
    GrpcPort = "9090";
    RequestTimeoutSeconds = 30;
    MaxRetryAttempts = 3;
    DefaultCreator = "alice";  // Default demo account
    EnableLogging = true;
    LogLevel = "info";
}

void TWeb4BridgeConfig::LoadFromFile(const System::UnicodeString& configPath) {
    if (!System::Ioutils::TFile::Exists(configPath)) return;

    try {
        std::unique_ptr<System::Inifiles::TIniFile> ini(new System::Inifiles::TIniFile(configPath));

        ApiEndpoint = ini->ReadString("server", "endpoint", ApiEndpoint);
        RestPort = ini->ReadString("server", "rest_port", RestPort);
        GrpcPort = ini->ReadString("server", "grpc_port", GrpcPort);
        RequestTimeoutSeconds = ini->ReadInteger("server", "timeout", RequestTimeoutSeconds);
        MaxRetryAttempts = ini->ReadInteger("server", "max_retries", MaxRetryAttempts);
        DefaultCreator = ini->ReadString("client", "default_creator", DefaultCreator);
        EnableLogging = ini->ReadBool("logging", "enabled", EnableLogging);
        LogLevel = ini->ReadString("logging", "level", LogLevel);
    }
    catch (const System::Sysutils::Exception& e) {
        // Log error but continue with defaults
    }
}

void TWeb4BridgeConfig::SaveToFile(const System::UnicodeString& configPath) {
    try {
        std::unique_ptr<System::Inifiles::TIniFile> ini(new System::Inifiles::TIniFile(configPath));

        ini->WriteString("server", "endpoint", ApiEndpoint);
        ini->WriteString("server", "rest_port", RestPort);
        ini->WriteString("server", "grpc_port", GrpcPort);
        ini->WriteInteger("server", "timeout", RequestTimeoutSeconds);
        ini->WriteInteger("server", "max_retries", MaxRetryAttempts);
        ini->WriteString("client", "default_creator", DefaultCreator);
        ini->WriteBool("logging", "enabled", EnableLogging);
        ini->WriteString("logging", "level", LogLevel);
    }
    catch (const System::Sysutils::Exception& e) {
        throw EWeb4Exception("Failed to save configuration: " + e.Message);
    }
}

void TWeb4BridgeConfig::LoadFromEnvironment() {
    System::UnicodeString envValue;

    envValue = System::Sysutils::GetEnvironmentVariable("API_BRIDGE_ENDPOINT");
    if (!envValue.IsEmpty()) ApiEndpoint = envValue;

    envValue = System::Sysutils::GetEnvironmentVariable("API_BRIDGE_PORT");
    if (!envValue.IsEmpty()) RestPort = envValue;

    envValue = System::Sysutils::GetEnvironmentVariable("API_BRIDGE_GRPC_PORT");
    if (!envValue.IsEmpty()) GrpcPort = envValue;

    envValue = System::Sysutils::GetEnvironmentVariable("API_BRIDGE_TIMEOUT");
    if (!envValue.IsEmpty()) RequestTimeoutSeconds = System::Sysutils::StrToIntDef(envValue, RequestTimeoutSeconds);

    envValue = System::Sysutils::GetEnvironmentVariable("WEB4_DEFAULT_CREATOR");
    if (!envValue.IsEmpty()) DefaultCreator = envValue;

    envValue = System::Sysutils::GetEnvironmentVariable("API_BRIDGE_LOG_LEVEL");
    if (!envValue.IsEmpty()) LogLevel = envValue;
}

     // ===========================================================================
// MAIN CLIENT IMPLEMENTATION
// ===========================================================================

TWeb4BridgeClient::TWeb4BridgeClient(const TWeb4BridgeConfig& config) :
    FConfig(config),
    FConnected(false) {

    FRESTClient.reset(new Rest::Client::TRESTClient(nullptr));
    FRESTRequest.reset(new Rest::Client::TRESTRequest(nullptr));
    FRESTResponse.reset(new Rest::Client::TRESTResponse(nullptr));

    FRESTRequest->Client = FRESTClient.get();
    FRESTRequest->Response = FRESTResponse.get();

    FRESTClient->ConnectTimeout = FConfig.RequestTimeoutSeconds * 1000;
    FRESTClient->ReadTimeout = FConfig.RequestTimeoutSeconds * 1000;
}

TWeb4BridgeClient::~TWeb4BridgeClient() {
    Disconnect();
}

bool TWeb4BridgeClient::Connect() {
    try {
        FRESTClient->BaseURL = BuildUrl("");

        // Test connection with health check
        System::Json::TJSONObject* healthResponse = CheckHealth();
        if (healthResponse) {
            FConnected = true;
            delete healthResponse;
            return true;
        }

        FConnected = false;
        return false;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("Connect", e);
        FConnected = false;
        return false;
    }
}

void TWeb4BridgeClient::Disconnect() {
try {
        if (FConnected) {
            Disconnect();
        }
        // Smart pointers will clean up automatically
        FRESTResponse.reset();
        FRESTRequest.reset();
        FRESTClient.reset();
    }
    catch (...) {
        // Don't throw in destructor
    }
}

bool TWeb4BridgeClient::TestConnection() {
    try {
        System::Json::TJSONObject* response = CheckHealth();
        if (response) {
            delete response;
            return true;
        }
    }
    catch (...) {
        // Connection failed
    }
    return false;
}

System::Json::TJSONObject* TWeb4BridgeClient::CheckHealth() {
    return MakeGetRequest("/health");
}

System::Json::TJSONObject* TWeb4BridgeClient::CheckDetailedHealth() {
    return MakeGetRequest("/health/detailed");
}

// ===========================================================================
// HTTP REQUEST METHODS
// ===========================================================================

System::Json::TJSONObject* TWeb4BridgeClient::MakeRequest(
    const System::UnicodeString& method,
    const System::UnicodeString& endpoint,
    System::Json::TJSONObject* data) {

    try {
        LogRequest(method, endpoint);

        FRESTClient->BaseURL = BuildUrl("");
        FRESTRequest->Resource = endpoint;

        // Set HTTP method
        if (method.Compare("GET") == 0) FRESTRequest->Method = Rest::Types::rmGET;
        else if (method.Compare("POST") == 0) FRESTRequest->Method = Rest::Types::rmPOST;
        else if (method.Compare("PUT") == 0) FRESTRequest->Method = Rest::Types::rmPUT;
        else if (method.Compare("DELETE") == 0) FRESTRequest->Method = Rest::Types::rmDELETE;

        // Clear previous parameters and body
        FRESTRequest->Params->Clear();
        FRESTRequest->ClearBody();

        // Add request body for POST/PUT
        if (data && (method.Compare("POST") == 0 || method.Compare("PUT") == 0)) {
            // FIXED: Proper JSON string handling with correct content type
            System::UnicodeString jsonString = data->ToJSON();
            
            // Try manually setting content type and using AddBody
            FRESTRequest->AddParameter("Content-Type", "application/json", Rest::Types::pkHTTPHEADER);
            FRESTRequest->AddBody(jsonString, "application/json");
        }

        // Execute request
        FRESTRequest->Execute();

        LogResponse(FRESTResponse->StatusCode, FRESTResponse->Content);

        // Check response status
        if (FRESTResponse->StatusCode >= 200 && FRESTResponse->StatusCode < 300) {
            if (!FRESTResponse->Content.IsEmpty()) {
                // FIXED: Proper JSON parsing with error handling
                try {
                    System::Json::TJSONValue* jsonValue =
                        System::Json::TJSONObject::ParseJSONValue(FRESTResponse->Content);
                    
                    System::Json::TJSONObject* jsonObject =
                        dynamic_cast<System::Json::TJSONObject*>(jsonValue);

                    if (jsonObject) {
                        // Create a clean copy to avoid memory corruption
                        try {
                            System::UnicodeString jsonText = jsonObject->ToJSON();
                            delete jsonObject; // Clean up original
                            
                            // Parse the string back into a new clean object
                            System::Json::TJSONValue* newValue = System::Json::TJSONObject::ParseJSONValue(jsonText);
                            System::Json::TJSONObject* newObject = dynamic_cast<System::Json::TJSONObject*>(newValue);
                            
                            return newObject; // Return clean copy or nullptr if failed
                        }
                        catch (...) {
                            return nullptr;
                        }
                    } else {
                        // If cast failed, clean up and return null
                        if (jsonValue) delete jsonValue;
                        return nullptr;
                    }
                } catch (...) {
                    // JSON parsing failed
                    return nullptr;
                }
            }
            return new System::Json::TJSONObject(); // Empty success response
        } else {
            System::UnicodeString errorMsg = "HTTP " +
                System::Sysutils::IntToStr(FRESTResponse->StatusCode) +
                ": " + FRESTResponse->StatusText;
            throw EWeb4Exception(errorMsg, FRESTResponse->StatusCode, "HTTP_ERROR");
        }
    }
    catch (const EWeb4Exception&) {
        throw; // Re-throw Web4 exceptions
    }
    catch (const System::Sysutils::Exception& e) {
        throw EWeb4Exception("Request failed: " + e.Message, 0, "REQUEST_ERROR");
    }
}

System::Json::TJSONObject* TWeb4BridgeClient::MakeGetRequest(const System::UnicodeString& endpoint) {
	return MakeRequest("GET", endpoint);
}

System::Json::TJSONObject* TWeb4BridgeClient::MakePostRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data) {
    return MakeRequest("POST", endpoint, data);
}

System::Json::TJSONObject* TWeb4BridgeClient::MakePutRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data) {
    return MakeRequest("PUT", endpoint, data);
}

System::Json::TJSONObject* TWeb4BridgeClient::MakeDeleteRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data) {
    return MakeRequest("DELETE", endpoint, data);
}

// ===========================================================================
// COMPONENT REGISTRY OPERATIONS
// ===========================================================================

std::unique_ptr<TComponentRegistration> TWeb4BridgeClient::RegisterComponent(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentData,
    const System::UnicodeString& context) {

    // FIXED: Use smart pointer for automatic cleanup
    std::unique_ptr<System::Json::TJSONObject> requestData(TWeb4Utils::CreateRequestBody());

    TWeb4Utils::AddStringField(requestData.get(), "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData.get(), "component_data", componentData);
    if (!context.IsEmpty()) {
        TWeb4Utils::AddStringField(requestData.get(), "context", context);
    }

    try {
        // FIXED: Pass raw pointer, but manage cleanup properly
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/components/register", requestData.get());

        if (response && ValidateResponse(response)) {
            std::unique_ptr<TComponentRegistration> registration(new TComponentRegistration());
            registration->FromJSON(response);
            delete response; // Clean up response
            return registration;
        } else if (response) {
            // Log validation failure details
            if (FConfig.EnableLogging) {
                System::UnicodeString errorMsg = System::Sysutils::FormatDateTime("yyyy-mm-dd hh:nn:ss", System::Sysutils::Now()) +
                    " [VALIDATION_FAILED] Response validation failed for: " + response->ToJSON();
                #ifdef _DEBUG
                OutputDebugStringA(System::Utf8string(errorMsg).c_str());
                #endif
            }
        }

        // Clean up response if validation failed
        if (response) delete response;
        return nullptr;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("RegisterComponent", e);
        return nullptr;
    }
}

std::unique_ptr<TComponentRegistration> TWeb4BridgeClient::GetComponent(
	const System::UnicodeString& componentId) {

    try {
        System::UnicodeString endpoint = "/api/v1/components/" + componentId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TComponentRegistration> registration(new TComponentRegistration());
            registration->FromJSON(response);
            delete response;
            return registration;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetComponent", e);
    }

    return nullptr;
}

System::Json::TJSONObject* TWeb4BridgeClient::VerifyComponent(
    const System::UnicodeString& componentId,
    const System::UnicodeString& creator) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);

    try {
        System::UnicodeString endpoint = "/api/v1/components/" + componentId + "/verify";
        System::Json::TJSONObject* response = MakePostRequest(endpoint, requestData);
        delete requestData;
        return response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("VerifyComponent", e);
        return nullptr;
    }
}

// ===========================================================================
// LCT MANAGEMENT OPERATIONS
// ===========================================================================

std::unique_ptr<TLctRelationship> TWeb4BridgeClient::CreateLct(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& context) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "context", context);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/lct/create", requestData);

        // Debug logging for CreateLct response
        if (FConfig.EnableLogging) {
            System::UnicodeString logMsg = System::Sysutils::FormatDateTime("yyyy-mm-dd hh:nn:ss", System::Sysutils::Now()) +
                " [CreateLct] Response received";
            #ifdef _DEBUG
            OutputDebugStringA(System::Utf8string(logMsg).c_str());
            #endif
            
            if (response) {
                System::UnicodeString responseMsg = " [CreateLct] Response JSON: " + response->ToJSON();
                #ifdef _DEBUG
                OutputDebugStringA(System::Utf8string(responseMsg).c_str());
                #endif
                
                System::Json::TJSONValue* statusValue = response->GetValue("status");
                if (statusValue) {
                    System::UnicodeString statusMsg = " [CreateLct] Status field: " + statusValue->Value();
                    #ifdef _DEBUG
                    OutputDebugStringA(System::Utf8string(statusMsg).c_str());
                    #endif
                }
            } else {
                #ifdef _DEBUG
                OutputDebugStringA(" [CreateLct] Response is NULL");
                #endif
            }
        }

        if (ValidateResponse(response)) {
            std::unique_ptr<TLctRelationship> lct(new TLctRelationship());
            lct->FromJSON(response);
            delete response;
            delete requestData;
            return lct;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CreateLct", e);
    }

    delete requestData;
    return nullptr;
}

std::unique_ptr<TLctRelationship> TWeb4BridgeClient::GetLct(const System::UnicodeString& lctId) {
    try {
        System::UnicodeString endpoint = "/api/v1/lct/" + lctId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TLctRelationship> lct(new TLctRelationship());
            lct->FromJSON(response);
            delete response;
            return lct;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetLct", e);
    }

    return nullptr;
}

// ===========================================================================
// TRUST TENSOR OPERATIONS
// ===========================================================================

std::unique_ptr<TTrustTensor> TWeb4BridgeClient::CreateTrustTensor(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& context,
    double initialScore) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "context", context);
    TWeb4Utils::AddNumberField(requestData, "initial_score", initialScore);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/trust/tensor", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TTrustTensor> tensor(new TTrustTensor());
            tensor->FromJSON(response);
            delete response;
            delete requestData;
            return tensor;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CreateTrustTensor", e);
    }

    delete requestData;
    return nullptr;
}

std::unique_ptr<TTrustTensor> TWeb4BridgeClient::GetTrustTensor(const System::UnicodeString& tensorId) {
    try {
        System::UnicodeString endpoint = "/api/v1/trust/tensor/" + tensorId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TTrustTensor> tensor(new TTrustTensor());
            tensor->FromJSON(response);
            delete response;
            return tensor;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetTrustTensor", e);
    }

    return nullptr;
}

std::unique_ptr<TTrustTensor> TWeb4BridgeClient::CalculateRelationshipTrust(
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationalContext) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "operational_context", operationalContext);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/trust-enhanced/calculate", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TTrustTensor> tensor(new TTrustTensor());
            tensor->FromJSON(response);
            delete response;
            delete requestData;
            return tensor;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CalculateRelationshipTrust", e);
    }

    delete requestData;
    return nullptr;
}

std::unique_ptr<TTrustTensor> TWeb4BridgeClient::GetRelationshipTensor(
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB) {

	try {
		System::UnicodeString endpoint = System::UnicodeString("/api/v1/trust-enhanced/relationship?component_a=") +
			componentA + System::UnicodeString("&component_b=") + componentB;
		System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TTrustTensor> tensor(new TTrustTensor());
            tensor->FromJSON(response);
            delete response;
            return tensor;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetRelationshipTensor", e);
    }

    return nullptr;
}

std::unique_ptr<TTrustTensor> TWeb4BridgeClient::UpdateTensorScore(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    double score,
    const System::UnicodeString& context) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddNumberField(requestData, "score", score);
    if (!context.IsEmpty()) {
        TWeb4Utils::AddStringField(requestData, "context", context);
    }

    try {
        System::Json::TJSONObject* response = MakePutRequest("/api/v1/trust-enhanced/score", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TTrustTensor> tensor(new TTrustTensor());
            tensor->FromJSON(response);
            delete response;
            delete requestData;
            return tensor;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("UpdateTensorScore", e);
    }

    delete requestData;
    return nullptr;
}

// ===========================================================================
// QUEUE MANAGEMENT OPERATIONS
// ===========================================================================

std::unique_ptr<TQueueRequest> TWeb4BridgeClient::QueuePairingRequest(
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationalContext,
    const System::UnicodeString& proxyId) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "operational_context", operationalContext);
    if (!proxyId.IsEmpty()) {
        TWeb4Utils::AddStringField(requestData, "proxy_id", proxyId);
    }

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/queue/pairing-request", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TQueueRequest> queueRequest(new TQueueRequest());
            queueRequest->FromJSON(response);
            delete response;
            delete requestData;
            return queueRequest;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("QueuePairingRequest", e);
    }

    delete requestData;
    return nullptr;
}

std::unique_ptr<TQueueStatus> TWeb4BridgeClient::GetQueueStatus(const System::UnicodeString& componentId) {
    try {
        System::UnicodeString endpoint = "/api/v1/queue/status/" + componentId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TQueueStatus> status(new TQueueStatus());
            status->FromJSON(response);
            delete response;
            return status;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetQueueStatus", e);
    }

    return nullptr;
}

System::Json::TJSONObject* TWeb4BridgeClient::ProcessOfflineQueue(const System::UnicodeString& componentId) {
    try {
        System::UnicodeString endpoint = "/api/v1/queue/process-offline/" + componentId;
        return MakePostRequest(endpoint, nullptr);
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("ProcessOfflineQueue", e);
        return nullptr;
    }
}

System::Json::TJSONObject* TWeb4BridgeClient::CancelQueueRequest(
    const System::UnicodeString& requestId,
    const System::UnicodeString& reason) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "reason", reason);

    try {
        System::UnicodeString endpoint = "/api/v1/queue/cancel/" + requestId;
        System::Json::TJSONObject* response = MakeDeleteRequest(endpoint, requestData);
        delete requestData;
        return response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CancelQueueRequest", e);
        return nullptr;
    }
}

std::vector<std::unique_ptr<TQueueRequest>> TWeb4BridgeClient::GetQueuedRequests(const System::UnicodeString& componentId) {
    std::vector<std::unique_ptr<TQueueRequest>> requests;

    try {
        System::UnicodeString endpoint = "/api/v1/queue/requests/" + componentId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response) && response->GetValue("requests")) {
            System::Json::TJSONArray* requestsArray = dynamic_cast<System::Json::TJSONArray*>(response->GetValue("requests"));
            if (requestsArray) {
                for (int i = 0; i < requestsArray->Count; i++) {
                    System::Json::TJSONObject* requestData = dynamic_cast<System::Json::TJSONObject*>(requestsArray->Items[i]);
                    if (requestData) {
                        std::unique_ptr<TQueueRequest> queueRequest(new TQueueRequest());
                        queueRequest->FromJSON(requestData);
                        requests.push_back(std::move(queueRequest));
                    }
                }
            }
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetQueuedRequests", e);
    }

    return requests;
}

std::vector<std::unique_ptr<TQueueRequest>> TWeb4BridgeClient::ListProxyQueue(const System::UnicodeString& proxyId) {
    std::vector<std::unique_ptr<TQueueRequest>> requests;

    try {
        System::UnicodeString endpoint = "/api/v1/queue/proxy/" + proxyId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response) && response->GetValue("requests")) {
            System::Json::TJSONArray* requestsArray = dynamic_cast<System::Json::TJSONArray*>(response->GetValue("requests"));
            if (requestsArray) {
                for (int i = 0; i < requestsArray->Count; i++) {
                    System::Json::TJSONObject* requestData = dynamic_cast<System::Json::TJSONObject*>(requestsArray->Items[i]);
                    if (requestData) {
                        std::unique_ptr<TQueueRequest> queueRequest(new TQueueRequest());
                        queueRequest->FromJSON(requestData);
                        requests.push_back(std::move(queueRequest));
                    }
                }
            }
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("ListProxyQueue", e);
    }

    return requests;
}

   // ===========================================================================
// AUTHORIZATION MANAGEMENT OPERATIONS
// ===========================================================================

std::unique_ptr<TPairingAuthorization> TWeb4BridgeClient::CreatePairingAuthorization(
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationalContext,
    const System::UnicodeString& authorizationRules) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "operational_context", operationalContext);
    TWeb4Utils::AddStringField(requestData, "authorization_rules", authorizationRules);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/authorization/pairing", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TPairingAuthorization> authorization(new TPairingAuthorization());
            authorization->FromJSON(response);
            delete response;
            delete requestData;
            return authorization;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CreatePairingAuthorization", e);
    }

    delete requestData;
    return nullptr;
}

std::vector<std::unique_ptr<TPairingAuthorization>> TWeb4BridgeClient::GetComponentAuthorizations(const System::UnicodeString& componentId) {
    std::vector<std::unique_ptr<TPairingAuthorization>> authorizations;

    try {
        System::UnicodeString endpoint = "/api/v1/authorization/component/" + componentId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response) && response->GetValue("authorizations")) {
            System::Json::TJSONArray* authArray = dynamic_cast<System::Json::TJSONArray*>(response->GetValue("authorizations"));
            if (authArray) {
                for (int i = 0; i < authArray->Count; i++) {
                    System::Json::TJSONObject* authData = dynamic_cast<System::Json::TJSONObject*>(authArray->Items[i]);
                    if (authData) {
                        std::unique_ptr<TPairingAuthorization> authorization(new TPairingAuthorization());
                        authorization->FromJSON(authData);
                        authorizations.push_back(std::move(authorization));
                    }
                }
            }
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetComponentAuthorizations", e);
    }

    return authorizations;
}

std::unique_ptr<TPairingAuthorization> TWeb4BridgeClient::UpdateAuthorization(
    const System::UnicodeString& authorizationId,
    const System::UnicodeString& operationalContext,
    const System::UnicodeString& authorizationRules) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    if (!operationalContext.IsEmpty()) {
        TWeb4Utils::AddStringField(requestData, "operational_context", operationalContext);
    }
    if (!authorizationRules.IsEmpty()) {
        TWeb4Utils::AddStringField(requestData, "authorization_rules", authorizationRules);
    }

    try {
        System::UnicodeString endpoint = "/api/v1/authorization/" + authorizationId;
        System::Json::TJSONObject* response = MakePutRequest(endpoint, requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TPairingAuthorization> authorization(new TPairingAuthorization());
            authorization->FromJSON(response);
            delete response;
            delete requestData;
            return authorization;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("UpdateAuthorization", e);
    }

    delete requestData;
    return nullptr;
}

System::Json::TJSONObject* TWeb4BridgeClient::RevokeAuthorization(
    const System::UnicodeString& authorizationId,
    const System::UnicodeString& reason) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "reason", reason);

    try {
        System::UnicodeString endpoint = "/api/v1/authorization/" + authorizationId;
        System::Json::TJSONObject* response = MakeDeleteRequest(endpoint, requestData);
        delete requestData;
        return response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("RevokeAuthorization", e);
        return nullptr;
    }
}

std::unique_ptr<TAuthorizationCheck> TWeb4BridgeClient::CheckPairingAuthorization(
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationalContext) {

    try {
		System::UnicodeString endpoint = System::UnicodeString("/api/v1/authorization/check") +
			System::UnicodeString("?component_a=") + componentA +
			System::UnicodeString("&component_b=") + componentB +
			System::UnicodeString("&operational_context=") + operationalContext;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TAuthorizationCheck> check(new TAuthorizationCheck());
            check->FromJSON(response);
            delete response;
            return check;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("CheckPairingAuthorization", e);
    }

    return nullptr;
}

// ===========================================================================
// ENERGY OPERATIONS
// ===========================================================================

std::unique_ptr<TEnergyOperation> TWeb4BridgeClient::CreateEnergyOperation(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationType,
    double amount,
    const System::UnicodeString& context) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "operation_type", operationType);
    TWeb4Utils::AddNumberField(requestData, "amount", amount);
    TWeb4Utils::AddStringField(requestData, "context", context);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/energy/operation", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TEnergyOperation> operation(new TEnergyOperation());
            operation->FromJSON(response);
            delete response;
            delete requestData;
            return operation;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CreateEnergyOperation", e);
    }

    delete requestData;
    return nullptr;
}

System::Json::TJSONObject* TWeb4BridgeClient::ExecuteEnergyTransfer(
    const System::UnicodeString& creator,
    const System::UnicodeString& operationId) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "operation_id", operationId);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/energy/transfer", requestData);
        delete requestData;
        return response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("ExecuteEnergyTransfer", e);
        return nullptr;
    }
}

std::unique_ptr<TEnergyBalance> TWeb4BridgeClient::GetEnergyBalance(const System::UnicodeString& componentId) {
    try {
        System::UnicodeString endpoint = "/api/v1/energy/balance/" + componentId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TEnergyBalance> balance(new TEnergyBalance());
            balance->FromJSON(response);
            delete response;
            return balance;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetEnergyBalance", e);
    }

    return nullptr;
}

// ===========================================================================
// PAIRING OPERATIONS
// ===========================================================================

std::unique_ptr<TPairingChallenge> TWeb4BridgeClient::InitiatePairing(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& operationalContext) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "operational_context", operationalContext);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/pairing/initiate", requestData);

        if (ValidateResponse(response)) {
            std::unique_ptr<TPairingChallenge> challenge(new TPairingChallenge());
            challenge->FromJSON(response);
            delete response;
            delete requestData;
            return challenge;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("InitiatePairing", e);
    }

    delete requestData;
    return nullptr;
}

System::Json::TJSONObject* TWeb4BridgeClient::CompletePairing(
    const System::UnicodeString& creator,
    const System::UnicodeString& challengeId,
    const System::UnicodeString& response) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "challenge_id", challengeId);
    TWeb4Utils::AddStringField(requestData, "response", response);

    try {
        System::Json::TJSONObject* responseObj = MakePostRequest("/api/v1/pairing/complete", requestData);
        delete requestData;
        return responseObj;
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        HandleError("CompletePairing", e);
        return nullptr;
    }
}

std::unique_ptr<TPairingChallenge> TWeb4BridgeClient::GetPairingStatus(const System::UnicodeString& challengeId) {
    try {
        System::UnicodeString endpoint = "/api/v1/pairing/status/" + challengeId;
        System::Json::TJSONObject* response = MakeGetRequest(endpoint);

        if (ValidateResponse(response)) {
            std::unique_ptr<TPairingChallenge> challenge(new TPairingChallenge());
            challenge->FromJSON(response);
            delete response;
            return challenge;
        }

        delete response;
    }
    catch (const System::Sysutils::Exception& e) {
        HandleError("GetPairingStatus", e);
    }

    return nullptr;
}

// ===========================================================================
// BATTERY MANAGEMENT SPECIFIC WORKFLOWS
// ===========================================================================

TWeb4BridgeClient::TBatteryPairingResult TWeb4BridgeClient::EstablishBatteryPairing(
    const System::UnicodeString& batteryPackId,
    const System::UnicodeString& batteryModuleId,
    const System::UnicodeString& operationalContext) {

    TBatteryPairingResult result;
    result.Success = false;
    result.InitialTrustScore = 0.5;

    try {
        // Step 1: Check if pairing is authorized
        std::unique_ptr<TAuthorizationCheck> authCheck = CheckPairingAuthorization(
            batteryPackId, batteryModuleId, operationalContext);

        if (!authCheck || !authCheck->Authorized) {
            result.ErrorMessage = "Pairing not authorized";
            return result;
        }

        // Step 2: Initiate pairing
        std::unique_ptr<TPairingChallenge> challenge = InitiatePairing(
            FConfig.DefaultCreator, batteryPackId, batteryModuleId, operationalContext);

        if (!challenge) {
            result.ErrorMessage = "Failed to initiate pairing";
            return result;
        }

        // Step 3: Complete pairing (simplified for demo)
        System::Json::TJSONObject* completionResult = CompletePairing(
            FConfig.DefaultCreator, challenge->ChallengeId, "demo_response");

        if (!completionResult) {
            result.ErrorMessage = "Failed to complete pairing";
            return result;
        }

        // Step 4: Calculate initial trust
        std::unique_ptr<TTrustTensor> trustCalc = CalculateRelationshipTrust(
            batteryPackId, batteryModuleId, operationalContext);

        if (trustCalc) {
            result.InitialTrustScore = trustCalc->TrustScore;
        }

        result.Success = true;
        result.PairingId = challenge->ChallengeId;

        delete completionResult;
    }
    catch (const System::Sysutils::Exception& e) {
        result.ErrorMessage = "Pairing failed: " + e.Message;
    }

    return result;
}

TWeb4BridgeClient::TEnergyTransferResult TWeb4BridgeClient::ExecuteBatteryEnergyTransfer(
    const System::UnicodeString& sourceComponentId,
    const System::UnicodeString& targetComponentId,
    double energyAmount,
    const System::UnicodeString& transferType) {

    TEnergyTransferResult result;
    result.Success = false;
    result.AmountTransferred = 0.0;
    result.FinalTrustScore = 0.5;

    try {
        // Step 1: Create energy operation
        std::unique_ptr<TEnergyOperation> operation = CreateEnergyOperation(
            FConfig.DefaultCreator, sourceComponentId, targetComponentId,
            transferType, energyAmount, "battery_management");

        if (!operation) {
            result.ErrorMessage = "Failed to create energy operation";
            return result;
        }

        // Step 2: Execute transfer
        System::Json::TJSONObject* transferResult = ExecuteEnergyTransfer(
            FConfig.DefaultCreator, operation->OperationId);

        if (!transferResult) {
            result.ErrorMessage = "Failed to execute energy transfer";
            return result;
        }

        // Step 3: Update trust based on successful transfer
        std::unique_ptr<TTrustTensor> updatedTrust = UpdateTensorScore(
            FConfig.DefaultCreator, sourceComponentId, targetComponentId,
            0.8, "successful_energy_transfer");

        if (updatedTrust) {
            result.FinalTrustScore = updatedTrust->TrustScore;
        }

        result.Success = true;
        result.TransferId = operation->OperationId;
        result.AmountTransferred = energyAmount;

        delete transferResult;
    }
    catch (const System::Sysutils::Exception& e) {
        result.ErrorMessage = "Energy transfer failed: " + e.Message;
    }

    return result;
}

std::unique_ptr<TWeb4BridgeClient::TBatteryStatusSummary> TWeb4BridgeClient::GetBatteryStatus(const System::UnicodeString& componentId) {
    std::unique_ptr<TBatteryStatusSummary> summary(new TBatteryStatusSummary());
    summary->ComponentId = componentId;
    summary->EnergyLevel = 0.0;
    summary->TrustScore = 0.5;
    summary->ActiveConnections = 0;
    summary->PendingRequests = 0;
    summary->HealthStatus = "unknown";

    try {
        // Get energy balance
        std::unique_ptr<TEnergyBalance> balance = GetEnergyBalance(componentId);
        if (balance) {
            summary->EnergyLevel = balance->TotalEnergy;
        }

        // Get queue status
        std::unique_ptr<TQueueStatus> queueStatus = GetQueueStatus(componentId);
        if (queueStatus) {
            summary->PendingRequests = queueStatus->PendingRequests;
            summary->HealthStatus = queueStatus->Status;
        }

        // Get component authorizations to count active connections
        std::vector<std::unique_ptr<TPairingAuthorization>> authorizations = GetComponentAuthorizations(componentId);
        summary->ActiveConnections = static_cast<int>(authorizations.size());

        // Calculate average trust score (simplified)
        if (!authorizations.empty()) {
            double totalTrust = 0.0;
            int trustCount = 0;

            for (const auto& auth : authorizations) {
                // For each authorization, try to get relationship trust
                std::unique_ptr<TTrustTensor> trust = GetRelationshipTensor(
                    auth->ComponentA == componentId ? auth->ComponentA : auth->ComponentB,
                    auth->ComponentA == componentId ? auth->ComponentB : auth->ComponentA);
                if (trust) {
                    totalTrust += trust->TrustScore;
                    trustCount++;
                }
            }

            if (trustCount > 0) {
                summary->TrustScore = totalTrust / trustCount;
            }
        }
    }
    catch (const System::Sysutils::Exception& e) {
        summary->HealthStatus = "error: " + e.Message;
    }

    return summary;
}

// Batch operations for multiple battery components
std::vector<TWeb4BridgeClient::TBatteryPairingResult> TWeb4BridgeClient::EstablishMultipleBatteryPairings(
    const std::vector<System::UnicodeString>& componentIds,
    const System::UnicodeString& operationalContext) {

    std::vector<TBatteryPairingResult> results;

    // Pair each component with every other component
    for (size_t i = 0; i < componentIds.size(); i++) {
        for (size_t j = i + 1; j < componentIds.size(); j++) {
            TBatteryPairingResult result = EstablishBatteryPairing(
                componentIds[i], componentIds[j], operationalContext);
            results.push_back(result);
        }
    }

    return results;
}

std::vector<std::unique_ptr<TWeb4BridgeClient::TBatteryStatusSummary>> TWeb4BridgeClient::GetMultipleBatteryStatus(
    const std::vector<System::UnicodeString>& componentIds) {

    std::vector<std::unique_ptr<TBatteryStatusSummary>> statuses;

    for (const auto& componentId : componentIds) {
        std::unique_ptr<TBatteryStatusSummary> status = GetBatteryStatus(componentId);
        if (status) {
            statuses.push_back(std::move(status));
        }
    }

    return statuses;
}

// ===========================================================================
// UTILITY AND HELPER METHODS
// ===========================================================================

void TWeb4BridgeClient::LogRequest(const System::UnicodeString& method, const System::UnicodeString& endpoint) {
    if (FConfig.EnableLogging) {
        System::UnicodeString logMsg = System::Sysutils::FormatDateTime("yyyy-mm-dd hh:nn:ss", System::Sysutils::Now()) +
            " [" + method + "] " + endpoint;
        // In a real implementation, you might want to use a proper logging framework
        // For now, we can use OutputDebugString for debugging
        #ifdef _DEBUG
        OutputDebugStringA(System::Utf8string(logMsg).c_str());
        #endif
    }
}

void TWeb4BridgeClient::LogResponse(int statusCode, const System::UnicodeString& response) {
    if (FConfig.EnableLogging) {
        System::UnicodeString logMsg = System::Sysutils::FormatDateTime("yyyy-mm-dd hh:nn:ss", System::Sysutils::Now()) +
            " [RESPONSE] " + System::Sysutils::IntToStr(statusCode) + " - " +
            (response.Length() > 100 ? response.SubString(1, 100) + "..." : response);
        #ifdef _DEBUG
        OutputDebugStringA(System::Utf8string(logMsg).c_str());
        #endif
    }
}

System::UnicodeString TWeb4BridgeClient::BuildUrl(const System::UnicodeString& endpoint) {
    return FConfig.ApiEndpoint + ":" + FConfig.RestPort + endpoint;
}

void TWeb4BridgeClient::HandleError(const System::UnicodeString& operation, const System::Sysutils::Exception& e) {
    System::UnicodeString errorMsg = "Operation '" + operation + "' failed: " + e.Message;
    if (FConfig.EnableLogging) {
        #ifdef _DEBUG
        OutputDebugStringA(System::Utf8string(errorMsg).c_str());
        #endif
    }
    // In a production environment, you might want to log to file or event log
}

bool TWeb4BridgeClient::ValidateResponse(System::Json::TJSONObject* response) {
    return response != nullptr && TWeb4Utils::IsSuccessResponse(response);
}

void TWeb4BridgeClient::UpdateConfig(const TWeb4BridgeConfig& newConfig) {
    FConfig = newConfig;
    if (FConnected) {
        Disconnect();
        Connect();
    }
}

bool TWeb4BridgeClient::ValidateApiConnection() {
    return TestConnection();
}

System::Json::TJSONObject* TWeb4BridgeClient::GetSystemMetrics() {
    // This would typically call a metrics endpoint on the API bridge
    // For now, return basic client metrics
    System::Json::TJSONObject* metrics = TWeb4Utils::CreateRequestBody();

    TWeb4Utils::AddBoolField(metrics, "connected", FConnected);
    TWeb4Utils::AddStringField(metrics, "endpoint", BuildUrl(""));
    TWeb4Utils::AddNumberField(metrics, "timeout_seconds", FConfig.RequestTimeoutSeconds);
    TWeb4Utils::AddStringField(metrics, "default_creator", FConfig.DefaultCreator);
    TWeb4Utils::AddStringField(metrics, "timestamp", TWeb4Utils::GenerateTimestamp());

    return metrics;
}

// Temporary debugging method to test LCT endpoint directly
System::Json::TJSONObject* TWeb4BridgeClient::TestLctCreateEndpoint(
    const System::UnicodeString& creator,
    const System::UnicodeString& componentA,
    const System::UnicodeString& componentB,
    const System::UnicodeString& context) {

    System::Json::TJSONObject* requestData = TWeb4Utils::CreateRequestBody();
    TWeb4Utils::AddStringField(requestData, "creator", creator.IsEmpty() ? FConfig.DefaultCreator : creator);
    TWeb4Utils::AddStringField(requestData, "component_a", componentA);
    TWeb4Utils::AddStringField(requestData, "component_b", componentB);
    TWeb4Utils::AddStringField(requestData, "context", context);

    try {
        System::Json::TJSONObject* response = MakePostRequest("/api/v1/lct/create", requestData);
        delete requestData;
        return response; // Return the raw response for debugging
    }
    catch (const System::Sysutils::Exception& e) {
        delete requestData;
        // Return error information as JSON
        System::Json::TJSONObject* errorResponse = TWeb4Utils::CreateRequestBody();
        TWeb4Utils::AddStringField(errorResponse, "debug_error", e.Message);
        return errorResponse;
    }
}







// ===========================================================================
// DEMO AND TESTING WORKFLOWS
// ===========================================================================

TWeb4BridgeClient::TDemoWorkflowResult TWeb4BridgeClient::ExecuteCompleteBatteryDemo(
    const std::vector<System::UnicodeString>& batteryPackIds,
    const std::vector<System::UnicodeString>& batteryModuleIds,
    const System::UnicodeString& hostSystemId) {

    TDemoWorkflowResult result;
    result.Success = false;
    result.DetailedResults = TWeb4Utils::CreateRequestBody();

    System::Json::TJSONArray* registrationResults = new System::Json::TJSONArray();
    System::Json::TJSONArray* pairingResults = new System::Json::TJSONArray();
    System::Json::TJSONArray* trustResults = new System::Json::TJSONArray();
    System::Json::TJSONArray* energyResults = new System::Json::TJSONArray();

    try {
        // Step 1: Register all components
        TWeb4Utils::AddStringField(result.DetailedResults, "step", "registering_components");

        // Register battery packs
        for (const auto& packId : batteryPackIds) {
            std::unique_ptr<TComponentRegistration> registration = RegisterComponent(
                FConfig.DefaultCreator, packId, "battery_pack_demo");

            if (registration) {
                result.CreatedComponentIds.push_back(registration->ComponentId);
                System::Json::TJSONObject* regResult = registration->ToJSON();
                registrationResults->AddElement(regResult);
            } else {
                System::Json::TJSONObject* errorResult = TWeb4Utils::CreateRequestBody();
                TWeb4Utils::AddStringField(errorResult, "component_id", packId);
                TWeb4Utils::AddStringField(errorResult, "error", "registration_failed");
                registrationResults->AddElement(errorResult);
            }
        }

        // Register battery modules
        for (const auto& moduleId : batteryModuleIds) {
            std::unique_ptr<TComponentRegistration> registration = RegisterComponent(
                FConfig.DefaultCreator, moduleId, "battery_module_demo");

            if (registration) {
                result.CreatedComponentIds.push_back(registration->ComponentId);
                System::Json::TJSONObject* regResult = registration->ToJSON();
                registrationResults->AddElement(regResult);
            } else {
                System::Json::TJSONObject* errorResult = TWeb4Utils::CreateRequestBody();
                TWeb4Utils::AddStringField(errorResult, "component_id", moduleId);
                TWeb4Utils::AddStringField(errorResult, "error", "registration_failed");
                registrationResults->AddElement(errorResult);
            }
        }

        // Register host system
        if (!hostSystemId.IsEmpty()) {
            std::unique_ptr<TComponentRegistration> registration = RegisterComponent(
                FConfig.DefaultCreator, hostSystemId, "host_system_demo");

            if (registration) {
                result.CreatedComponentIds.push_back(registration->ComponentId);
                System::Json::TJSONObject* regResult = registration->ToJSON();
                registrationResults->AddElement(regResult);
            }
        }

        // Step 2: Create LCT relationships and establish pairings
        TWeb4Utils::AddStringField(result.DetailedResults, "step", "establishing_pairings");

        // Pair modules with packs
        for (const auto& moduleId : batteryModuleIds) {
            for (const auto& packId : batteryPackIds) {
                std::unique_ptr<TLctRelationship> lct = CreateLct(
                    FConfig.DefaultCreator, moduleId, packId, "battery_management_demo");

                if (lct) {
                    result.CreatedLctIds.push_back(lct->LctId);

                    // Establish pairing
                    TBatteryPairingResult pairingResult = EstablishBatteryPairing(
                        moduleId, packId, "battery_management_demo");

                    System::Json::TJSONObject* pairResult = TWeb4Utils::CreateRequestBody();
                    TWeb4Utils::AddStringField(pairResult, "module_id", moduleId);
                    TWeb4Utils::AddStringField(pairResult, "pack_id", packId);
                    TWeb4Utils::AddStringField(pairResult, "lct_id", lct->LctId);
                    TWeb4Utils::AddBoolField(pairResult, "pairing_success", pairingResult.Success);
                    TWeb4Utils::AddNumberField(pairResult, "initial_trust", pairingResult.InitialTrustScore);
                    if (!pairingResult.ErrorMessage.IsEmpty()) {
                        TWeb4Utils::AddStringField(pairResult, "error", pairingResult.ErrorMessage);
                    }
                    pairingResults->AddElement(pairResult);
                }
            }
        }

        // Pair packs with host system
        if (!hostSystemId.IsEmpty()) {
            for (const auto& packId : batteryPackIds) {
                std::unique_ptr<TLctRelationship> lct = CreateLct(
                    FConfig.DefaultCreator, packId, hostSystemId, "battery_management_demo");

                if (lct) {
                    result.CreatedLctIds.push_back(lct->LctId);

                    TBatteryPairingResult pairingResult = EstablishBatteryPairing(
                        packId, hostSystemId, "battery_management_demo");

                    System::Json::TJSONObject* pairResult = TWeb4Utils::CreateRequestBody();
                    TWeb4Utils::AddStringField(pairResult, "pack_id", packId);
                    TWeb4Utils::AddStringField(pairResult, "host_id", hostSystemId);
                    TWeb4Utils::AddStringField(pairResult, "lct_id", lct->LctId);
                    TWeb4Utils::AddBoolField(pairResult, "pairing_success", pairingResult.Success);
                    TWeb4Utils::AddNumberField(pairResult, "initial_trust", pairingResult.InitialTrustScore);
                    pairingResults->AddElement(pairResult);
                }
            }
        }

        // Step 3: Create and update trust tensors
        TWeb4Utils::AddStringField(result.DetailedResults, "step", "managing_trust");

        // Create trust tensors for some relationships
		size_t maxIterations = (result.CreatedLctIds.size() < 3) ? result.CreatedLctIds.size() : 3;
		for (size_t i = 0; i < maxIterations; i++) {
			const System::UnicodeString& lctId = result.CreatedLctIds[i];

            // Create trust tensor using component relationships
            if (i < batteryModuleIds.size() && i < batteryPackIds.size()) {
                std::unique_ptr<TTrustTensor> tensor = CreateTrustTensor(
                    FConfig.DefaultCreator, batteryModuleIds[i], batteryPackIds[i],
                    "battery_management_demo", 0.7);

                if (tensor) {
                    result.CreatedTensorIds.push_back(tensor->TensorId);

                    System::Json::TJSONObject* trustResult = tensor->ToJSON();
                    trustResults->AddElement(trustResult);

                    // Update trust score based on demo scenario
                    std::unique_ptr<TTrustTensor> updatedTensor = UpdateTensorScore(
                        FConfig.DefaultCreator, batteryModuleIds[i], batteryPackIds[i],
                        0.85, "successful_demo_operation");

                    if (updatedTensor) {
                        System::Json::TJSONObject* updateResult = TWeb4Utils::CreateRequestBody();
                        TWeb4Utils::AddStringField(updateResult, "tensor_id", updatedTensor->TensorId);
                        TWeb4Utils::AddNumberField(updateResult, "updated_score", updatedTensor->TrustScore);
                        TWeb4Utils::AddStringField(updateResult, "operation", "score_update");
                        trustResults->AddElement(updateResult);
                    }
                }
            }
        }

        // Step 4: Execute energy operations
        TWeb4Utils::AddStringField(result.DetailedResults, "step", "energy_operations");

        // Create energy operations between components
		size_t maxModules = (batteryModuleIds.size() < 2) ? batteryModuleIds.size() : 2;
		for (size_t i = 0; i < maxModules && i < batteryPackIds.size(); i++) {

			TEnergyTransferResult energyResult = ExecuteBatteryEnergyTransfer(
                batteryModuleIds[i], batteryPackIds[i], 25.5, "charge");

            if (energyResult.Success) {
                result.ExecutedOperationIds.push_back(energyResult.TransferId);
            }

            System::Json::TJSONObject* energyOp = TWeb4Utils::CreateRequestBody();
            TWeb4Utils::AddStringField(energyOp, "source", batteryModuleIds[i]);
            TWeb4Utils::AddStringField(energyOp, "target", batteryPackIds[i]);
            TWeb4Utils::AddNumberField(energyOp, "amount", 25.5);
            TWeb4Utils::AddBoolField(energyOp, "success", energyResult.Success);
            TWeb4Utils::AddNumberField(energyOp, "final_trust", energyResult.FinalTrustScore);
            if (!energyResult.ErrorMessage.IsEmpty()) {
                TWeb4Utils::AddStringField(energyOp, "error", energyResult.ErrorMessage);
            }
            energyResults->AddElement(energyOp);
        }

        // Step 5: Generate final summary
        result.DetailedResults->AddPair("component_registrations", registrationResults);
        result.DetailedResults->AddPair("pairing_results", pairingResults);
        result.DetailedResults->AddPair("trust_results", trustResults);
        result.DetailedResults->AddPair("energy_results", energyResults);

        TWeb4Utils::AddNumberField(result.DetailedResults, "total_components", static_cast<double>(result.CreatedComponentIds.size()));
        TWeb4Utils::AddNumberField(result.DetailedResults, "total_lcts", static_cast<double>(result.CreatedLctIds.size()));
        TWeb4Utils::AddNumberField(result.DetailedResults, "total_tensors", static_cast<double>(result.CreatedTensorIds.size()));
        TWeb4Utils::AddNumberField(result.DetailedResults, "total_operations", static_cast<double>(result.ExecutedOperationIds.size()));
        TWeb4Utils::AddStringField(result.DetailedResults, "demo_status", "completed");
        TWeb4Utils::AddStringField(result.DetailedResults, "completion_time", TWeb4Utils::GenerateTimestamp());

        result.Success = true;

    }
    catch (const System::Sysutils::Exception& e) {
        result.ErrorMessage = "Demo workflow failed: " + e.Message;
        TWeb4Utils::AddStringField(result.DetailedResults, "error", result.ErrorMessage);
        TWeb4Utils::AddStringField(result.DetailedResults, "demo_status", "failed");

        // Clean up any partial results arrays if they weren't added yet
        if (!result.DetailedResults->GetValue("component_registrations")) {
            result.DetailedResults->AddPair("component_registrations", registrationResults);
        }
        if (!result.DetailedResults->GetValue("pairing_results")) {
            result.DetailedResults->AddPair("pairing_results", pairingResults);
        }
        if (!result.DetailedResults->GetValue("trust_results")) {
            result.DetailedResults->AddPair("trust_results", trustResults);
        }
        if (!result.DetailedResults->GetValue("energy_results")) {
            result.DetailedResults->AddPair("energy_results", energyResults);
        }
    }

    return result;
}


// ===========================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// ===========================================================================

System::UnicodeString TWeb4Utils::GenerateComponentId(const System::UnicodeString& prefix) {
    return prefix + "_" + System::Sysutils::IntToStr(MyDateTimeToUnix(System::Sysutils::Now()));
}

System::UnicodeString TWeb4Utils::GenerateTimestamp() {
    return System::Sysutils::IntToStr(MyDateTimeToUnix(System::Sysutils::Now()));
}

System::UnicodeString TWeb4Utils::FormatTrustScore(double score) {
    return System::Sysutils::FloatToStrF(score, System::Sysutils::ffFixed, 15, 3);
}

bool TWeb4Utils::ValidateComponentId(const System::UnicodeString& componentId) {
    return !componentId.IsEmpty() && componentId.Length() > 3;
}

System::UnicodeString TWeb4Utils::ExtractErrorMessage(System::Json::TJSONObject* response) {
    if (!response) return "Unknown error";

    System::Json::TJSONValue* errorValue = response->GetValue("error");
    if (errorValue) {
        return errorValue->Value();
    }

    System::Json::TJSONValue* messageValue = response->GetValue("message");
    if (messageValue) {
        return messageValue->Value();
    }

    return "Operation failed";
}

bool TWeb4Utils::IsSuccessResponse(System::Json::TJSONObject* response) {
    if (!response) return false;

    System::Json::TJSONValue* statusValue = response->GetValue("status");
    if (statusValue) {
        System::UnicodeString status = statusValue->Value();
        // Accept Web4 API success status values
        return System::Sysutils::SameText(status, "registered") ||  // Component registration
               System::Sysutils::SameText(status, "active");        // LCT creation
    }

    // If no status field, assume success if no error field
    return !response->GetValue("error");
}

System::Json::TJSONObject* TWeb4Utils::CreateRequestBody() {
    return new System::Json::TJSONObject();
}

void TWeb4Utils::AddStringField(System::Json::TJSONObject* json,
                               const System::UnicodeString& key,
                               const System::UnicodeString& value) {
    if (json) {
        // FIXED: Always add the field, even if empty (API might expect it)
        json->AddPair(key, new System::Json::TJSONString(value));
    }
}

void TWeb4Utils::AddNumberField(System::Json::TJSONObject* json, const System::UnicodeString& key, double value) {
    if (json) {
        json->AddPair(key, new System::Json::TJSONNumber(value));
    }
}

void TWeb4Utils::AddBoolField(System::Json::TJSONObject* json, const System::UnicodeString& key, bool value) {
    if (json) {
        if (value) {
            json->AddPair(key, new System::Json::TJSONTrue());
        } else {
            json->AddPair(key, new System::Json::TJSONFalse());
        }
    }
}

System::UnicodeString TWeb4Utils::SafeJsonString(System::Json::TJSONValue* value) {
    return value ? value->Value() : "";
}

double TWeb4Utils::SafeJsonNumber(System::Json::TJSONValue* value, double defaultValue) {
    if (!value) return defaultValue;
    try {
        return System::Sysutils::StrToFloatDef(value->Value(), defaultValue);
    }
    catch (...) {
        return defaultValue;
    }
}

bool TWeb4Utils::SafeJsonBool(System::Json::TJSONValue* value, bool defaultValue) {
    if (!value) return defaultValue;
    return System::Sysutils::SameText(value->Value(), "true");
}

__int64 TWeb4Utils::SafeJsonInt64(System::Json::TJSONValue* value, __int64 defaultValue) {
    if (!value) return defaultValue;
    try {
        return System::Sysutils::StrToInt64Def(value->Value(), defaultValue);
    }
    catch (...) {
        return defaultValue;
    }
}

// ===========================================================================
// END OF WEB4_ENHANCED.CPP IMPLEMENTATION
// ===========================================================================

