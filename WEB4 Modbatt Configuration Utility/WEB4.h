#ifndef WEB4_H
#define WEB4_H

// ===========================================================================
// Web4 Race Car Battery Management - Enhanced C++ RAD Studio SDK
// Using API Bridge Layer for Better Performance and Abstraction
// ===========================================================================

#include <System.Classes.hpp>
#include <System.SysUtils.hpp>
#include <System.DateUtils.hpp>
#include <System.JSON.hpp>
#include <REST.Client.hpp>
#include <REST.Types.hpp>
#include <REST.Response.Adapter.hpp>
#include <memory>
#include <vector>
#include <map>
#include <functional>

// Compatibility for older compilers
#ifndef nullptr
    #define nullptr NULL
#endif

// ===========================================================================
// ENHANCED DATA STRUCTURES
// ===========================================================================

class TComponentRegistration {
public:
    System::UnicodeString ComponentId;
    System::UnicodeString ComponentIdentity;
    System::UnicodeString LctId;
    System::UnicodeString Status;
    System::UnicodeString ComponentData;  // The original input data
    System::UnicodeString Context;        // The context (e.g., "battery_pack_demo")
    System::UnicodeString Creator;        // The creator (e.g., "alice")
    System::UnicodeString TxHash;         // The transaction hash
    System::TDateTime CreatedAt;

    TComponentRegistration();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TLctRelationship {
public:
    System::UnicodeString Creator;
    System::UnicodeString LctId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString Context;
    System::UnicodeString ProxyId;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;
    System::UnicodeString TxHash;
    
    // Cryptographic key halves for device communication
    System::UnicodeString DeviceKeyHalf;
    System::UnicodeString LctKeyHalf;

    TLctRelationship();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TTrustTensor {
public:
    System::UnicodeString TensorId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationalContext;
    double TrustScore;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;
    System::TDateTime CalculatedAt;
    System::TDateTime LastUpdated;
    int EvidenceCount;
    double LearningRate;
    System::UnicodeString TxHash;

    TTrustTensor();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TQueueRequest {
public:
    System::UnicodeString RequestId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationalContext;
    System::UnicodeString ProxyId;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;
    System::UnicodeString TxHash;

    TQueueRequest();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TQueueStatus {
public:
    System::UnicodeString ComponentId;
    System::UnicodeString Status;
    int PendingRequests;
    int ProcessedRequests;
    System::TDateTime LastProcessed;

    TQueueStatus();
    void FromJSON(System::Json::TJSONObject* json);
};

class TPairingAuthorization {
public:
    System::UnicodeString AuthorizationId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationalContext;
    System::UnicodeString AuthorizationRules;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;
    System::UnicodeString TxHash;

    TPairingAuthorization();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TAuthorizationCheck {
public:
    bool Authorized;
    System::UnicodeString AuthorizationId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationalContext;
    double TrustScore;
    System::UnicodeString VerificationStatus;
    System::TDateTime CheckedAt;

    TAuthorizationCheck();
    void FromJSON(System::Json::TJSONObject* json);
};

class TEnergyOperation {
public:
    System::UnicodeString OperationId;
    System::UnicodeString Creator;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationType;
    double Amount;
    System::UnicodeString Context;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;

    TEnergyOperation();
    void FromJSON(System::Json::TJSONObject* json);
    System::Json::TJSONObject* ToJSON() const;
};

class TEnergyBalance {
public:
    System::UnicodeString ComponentId;
    double AtpBalance;
    double AdpBalance;
    double TotalEnergy;
    double TrustWeightedBalance;
    System::TDateTime LastUpdated;

    TEnergyBalance();
    void FromJSON(System::Json::TJSONObject* json);
};

class TPairingChallenge {
public:
    System::UnicodeString ChallengeId;
    System::UnicodeString ComponentA;
    System::UnicodeString ComponentB;
    System::UnicodeString OperationalContext;
    System::UnicodeString Status;
    System::TDateTime CreatedAt;

    TPairingChallenge();
    void FromJSON(System::Json::TJSONObject* json);
};

// ===========================================================================
// ENHANCED CLIENT CONFIGURATION
// ===========================================================================

class TWeb4BridgeConfig {
public:
    System::UnicodeString ApiEndpoint;      // API Bridge endpoint
    System::UnicodeString RestPort;         // REST API port
    System::UnicodeString GrpcPort;         // gRPC port (future use)
    int RequestTimeoutSeconds;
    int MaxRetryAttempts;
    System::UnicodeString DefaultCreator;   // Default creator account
    bool EnableLogging;
    System::UnicodeString LogLevel;

    TWeb4BridgeConfig();
    void LoadFromFile(const System::UnicodeString& configPath);
    void SaveToFile(const System::UnicodeString& configPath);
    void LoadFromEnvironment();
};

// Forward declare the exception class
class EWeb4Exception : public System::Sysutils::Exception {
public:
    int ErrorCode;
    System::UnicodeString ErrorType;

    EWeb4Exception(const System::UnicodeString& msg, int code = 0, const System::UnicodeString& type = "");
};

// ===========================================================================
// ENHANCED WEB4 CLIENT WITH API BRIDGE INTEGRATION
// ===========================================================================

class TWeb4BridgeClient {
private:
    std::unique_ptr<Rest::Client::TRESTClient> FRESTClient;
    std::unique_ptr<Rest::Client::TRESTRequest> FRESTRequest;
    std::unique_ptr<Rest::Client::TRESTResponse> FRESTResponse;
    TWeb4BridgeConfig FConfig;
    bool FConnected;

    // Helper methods
    System::Json::TJSONObject* MakeRequest(
        const System::UnicodeString& method,
        const System::UnicodeString& endpoint,
        System::Json::TJSONObject* data = nullptr);

    System::Json::TJSONObject* MakeGetRequest(const System::UnicodeString& endpoint);
    System::Json::TJSONObject* MakePostRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data);
    System::Json::TJSONObject* MakePutRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data);
    System::Json::TJSONObject* MakeDeleteRequest(const System::UnicodeString& endpoint, System::Json::TJSONObject* data = nullptr);

    void LogRequest(const System::UnicodeString& method, const System::UnicodeString& endpoint);
    void LogResponse(int statusCode, const System::UnicodeString& response);
    System::UnicodeString BuildUrl(const System::UnicodeString& endpoint);

    // Error handling
    void HandleError(const System::UnicodeString& operation, const System::Sysutils::Exception& e);
    bool ValidateResponse(System::Json::TJSONObject* response);

public:
    explicit TWeb4BridgeClient(const TWeb4BridgeConfig& config = TWeb4BridgeConfig());
    ~TWeb4BridgeClient();

    // Connection management
    bool Connect();
    void Disconnect();
    bool IsConnected() const { return FConnected; }
    bool TestConnection();

    // Health check
    System::Json::TJSONObject* CheckHealth();
    System::Json::TJSONObject* CheckDetailedHealth();

    // ===================================================================
    // COMPONENT REGISTRY OPERATIONS
    // ===================================================================

    std::unique_ptr<TComponentRegistration> RegisterComponent(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentData,
        const System::UnicodeString& context = "");

    std::unique_ptr<TComponentRegistration> GetComponent(
        const System::UnicodeString& componentId);

    System::Json::TJSONObject* VerifyComponent(
        const System::UnicodeString& componentId,
        const System::UnicodeString& creator);

    // ===================================================================
    // LCT MANAGEMENT OPERATIONS
    // ===================================================================

    std::unique_ptr<TLctRelationship> CreateLct(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& context);

    std::unique_ptr<TLctRelationship> GetLct(
        const System::UnicodeString& lctId);

    // ===================================================================
    // TRUST TENSOR OPERATIONS
    // ===================================================================

    std::unique_ptr<TTrustTensor> CreateTrustTensor(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& context,
        double initialScore);

    std::unique_ptr<TTrustTensor> GetTrustTensor(
        const System::UnicodeString& tensorId);

    // ===================================================================
    // ENHANCED TRUST TENSOR OPERATIONS
    // ===================================================================

    std::unique_ptr<TTrustTensor> CalculateRelationshipTrust(
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationalContext);

    std::unique_ptr<TTrustTensor> GetRelationshipTensor(
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB);

    std::unique_ptr<TTrustTensor> UpdateTensorScore(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        double score,
        const System::UnicodeString& context = "");

    // ===================================================================
    // QUEUE MANAGEMENT OPERATIONS
    // ===================================================================

    std::unique_ptr<TQueueRequest> QueuePairingRequest(
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationalContext,
        const System::UnicodeString& proxyId = "");

    std::unique_ptr<TQueueStatus> GetQueueStatus(
        const System::UnicodeString& componentId);

    System::Json::TJSONObject* ProcessOfflineQueue(
        const System::UnicodeString& componentId);

    System::Json::TJSONObject* CancelQueueRequest(
        const System::UnicodeString& requestId,
        const System::UnicodeString& reason);

    std::vector<std::unique_ptr<TQueueRequest>> GetQueuedRequests(
        const System::UnicodeString& componentId);

    std::vector<std::unique_ptr<TQueueRequest>> ListProxyQueue(
        const System::UnicodeString& proxyId);

    // ===================================================================
    // AUTHORIZATION MANAGEMENT OPERATIONS
    // ===================================================================

    std::unique_ptr<TPairingAuthorization> CreatePairingAuthorization(
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationalContext,
        const System::UnicodeString& authorizationRules);

    std::vector<std::unique_ptr<TPairingAuthorization>> GetComponentAuthorizations(
        const System::UnicodeString& componentId);

    std::unique_ptr<TPairingAuthorization> UpdateAuthorization(
        const System::UnicodeString& authorizationId,
        const System::UnicodeString& operationalContext = "",
        const System::UnicodeString& authorizationRules = "");

    System::Json::TJSONObject* RevokeAuthorization(
        const System::UnicodeString& authorizationId,
        const System::UnicodeString& reason);

    std::unique_ptr<TAuthorizationCheck> CheckPairingAuthorization(
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationalContext);

    // ===================================================================
    // ENERGY OPERATIONS
    // ===================================================================

    std::unique_ptr<TEnergyOperation> CreateEnergyOperation(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationType,
        double amount,
        const System::UnicodeString& context);

    System::Json::TJSONObject* ExecuteEnergyTransfer(
        const System::UnicodeString& creator,
        const System::UnicodeString& operationId);

    std::unique_ptr<TEnergyBalance> GetEnergyBalance(
        const System::UnicodeString& componentId);

    // ===================================================================
    // PAIRING OPERATIONS
    // ===================================================================

    std::unique_ptr<TPairingChallenge> InitiatePairing(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& operationalContext);

    System::Json::TJSONObject* CompletePairing(
        const System::UnicodeString& creator,
        const System::UnicodeString& challengeId,
        const System::UnicodeString& response);

    std::unique_ptr<TPairingChallenge> GetPairingStatus(
        const System::UnicodeString& challengeId);

    // ===================================================================
    // BATTERY MANAGEMENT SPECIFIC WORKFLOWS
    // ===================================================================

    // High-level battery management operations
    struct TBatteryPairingResult {
        bool Success;
        System::UnicodeString PairingId;
        System::UnicodeString ErrorMessage;
        double InitialTrustScore;
    };

    TBatteryPairingResult EstablishBatteryPairing(
        const System::UnicodeString& batteryPackId,
        const System::UnicodeString& batteryModuleId,
        const System::UnicodeString& operationalContext = "battery_management");

    struct TEnergyTransferResult {
        bool Success;
        System::UnicodeString TransferId;
        double AmountTransferred;
        double FinalTrustScore;
        System::UnicodeString ErrorMessage;
    };

    TEnergyTransferResult ExecuteBatteryEnergyTransfer(
        const System::UnicodeString& sourceComponentId,
        const System::UnicodeString& targetComponentId,
        double energyAmount,
        const System::UnicodeString& transferType = "charge");

    struct TBatteryStatusSummary {
        System::UnicodeString ComponentId;
        double EnergyLevel;
        double TrustScore;
        int ActiveConnections;
        int PendingRequests;
        System::UnicodeString HealthStatus;
    };

    std::unique_ptr<TBatteryStatusSummary> GetBatteryStatus(
        const System::UnicodeString& componentId);

    // Batch operations for multiple battery components
    std::vector<TBatteryPairingResult> EstablishMultipleBatteryPairings(
        const std::vector<System::UnicodeString>& componentIds,
        const System::UnicodeString& operationalContext = "battery_management");

    std::vector<std::unique_ptr<TBatteryStatusSummary>> GetMultipleBatteryStatus(
        const std::vector<System::UnicodeString>& componentIds);

    // ===================================================================
    // DEMO AND TESTING WORKFLOWS
    // ===================================================================

    struct TDemoWorkflowResult {
        bool Success;
        System::UnicodeString ErrorMessage;
        std::vector<System::UnicodeString> CreatedComponentIds;
        std::vector<System::UnicodeString> CreatedLctIds;
        std::vector<System::UnicodeString> CreatedTensorIds;
        std::vector<System::UnicodeString> ExecutedOperationIds;
        System::Json::TJSONObject* DetailedResults;
    };

    TDemoWorkflowResult ExecuteCompleteBatteryDemo(
        const std::vector<System::UnicodeString>& batteryPackIds,
        const std::vector<System::UnicodeString>& batteryModuleIds,
        const System::UnicodeString& hostSystemId);

    // Testing utilities
    bool ValidateApiConnection();
    System::Json::TJSONObject* RunDiagnostics();
    System::Json::TJSONObject* GetSystemMetrics();
    
    // Temporary debugging method
    System::Json::TJSONObject* TestLctCreateEndpoint(
        const System::UnicodeString& creator,
        const System::UnicodeString& componentA,
        const System::UnicodeString& componentB,
        const System::UnicodeString& context);

    // Configuration access
    const TWeb4BridgeConfig& GetConfig() const { return FConfig; }
    void UpdateConfig(const TWeb4BridgeConfig& newConfig);
};

// ===========================================================================
// UTILITY FUNCTIONS AND HELPER CLASSES
// ===========================================================================

class TWeb4Utils {
public:
    static System::UnicodeString GenerateComponentId(const System::UnicodeString& prefix = "comp");
    static System::UnicodeString GenerateTimestamp();
    static System::UnicodeString FormatTrustScore(double score);
    static bool ValidateComponentId(const System::UnicodeString& componentId);
    static System::UnicodeString ExtractErrorMessage(System::Json::TJSONObject* response);
    static bool IsSuccessResponse(System::Json::TJSONObject* response);

    // JSON helpers
    static System::Json::TJSONObject* CreateRequestBody();
    static void AddStringField(System::Json::TJSONObject* json, const System::UnicodeString& key, const System::UnicodeString& value);
    static void AddNumberField(System::Json::TJSONObject* json, const System::UnicodeString& key, double value);
    static void AddBoolField(System::Json::TJSONObject* json, const System::UnicodeString& key, bool value);

    // String helpers
    static System::UnicodeString SafeJsonString(System::Json::TJSONValue* value);
    static double SafeJsonNumber(System::Json::TJSONValue* value, double defaultValue = 0.0);
    static bool SafeJsonBool(System::Json::TJSONValue* value, bool defaultValue = false);
    static __int64 SafeJsonInt64(System::Json::TJSONValue* value, __int64 defaultValue = 0);
};

#endif // WEB4_H
