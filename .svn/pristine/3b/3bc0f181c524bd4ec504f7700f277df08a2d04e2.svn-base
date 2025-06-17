//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "PCANBasicClass.h"

// Critical Section class for thread-safe menbers access
//
class clsCritical
{
private:
    CRITICAL_SECTION *m_objpCS;
    ULONG volatile m_dwOwnerThread;
    LONG volatile m_dwLocked;
    bool volatile m_bDoRecursive;

public:
    explicit clsCritical(CRITICAL_SECTION *cs, bool createUnlocked = false, bool lockRecursively = false);
    ~clsCritical();
    int GetRecursionCount();
    bool IsLocked();
    int Enter();
    int Leave();
};

/// Message Status structure used to show CAN Messages
/// in a ListView
//
class MessageStatus
{
private:
    TPCANMsgFD m_Msg;
    TPCANTimestampFD m_TimeStamp;
    TPCANTimestampFD m_oldTimeStamp;
    int m_iIndex;
    int m_Count;
    bool m_bShowPeriod;
    bool m_bWasChanged;

    AnsiString GetTypeString();
    AnsiString GetIdString();
    AnsiString GetDataString();
    AnsiString GetTimeString();
    void SetShowingPeriod(bool value);

public:
    MessageStatus(TPCANMsgFD canMsg,TPCANTimestampFD canTimestamp,int listIndex);
    void Update(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp);

    __property TPCANMsgFD CANMsg = {read = m_Msg};
    __property TPCANTimestampFD Timestamp = {read = m_TimeStamp};
    __property int Position = {read = m_iIndex};
    __property AnsiString TypeString = {read = GetTypeString};
    __property AnsiString IdString = {read = GetIdString};
    __property AnsiString DataString = {read = GetDataString};
    __property AnsiString TimeString = {read = GetTimeString};
    __property int Count = {read = m_Count};
    __property bool ShowingPeriod = {read = m_bShowPeriod, write = SetShowingPeriod};
    __property bool MarkedAsUpdated = {read = m_bWasChanged, write = m_bWasChanged};
};

typedef void (__closure *ProcMsgRead)();

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:    // IDE-managed Components
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox3;
    TGroupBox *GroupBox4;
    TGroupBox *GroupBox6;
    TLabel *Label1;
    TComboBox *cbbChannel;
    TButton *btnHwRefresh;
    TLabel *laBaudrate;
    TComboBox *cbbBaudrates;
    TLabel *laHwType;
    TComboBox *cbbHwType;
    TComboBox *cbbIO;
    TLabel *laIOPort;
    TLabel *laInterrupt;
    TComboBox *cbbInterrupt;
	TButton *btnConnect;
    TButton *btnRelease;
    TButton *btnParameterSet;
    TButton *btnParameterGet;
    TLabel *Label8;
    TComboBox *cbbParameter;
    TLabel *Label9;
    TRadioButton *rdbParamActive;
    TRadioButton *rdbParamInactive;
    TLabel *laDeviceOrDelay;
    TEdit *txtDeviceIdOrDelay;
    TCheckBox *chbShowPeriod;
    TButton *btnRead;
    TButton *btnMsgClear;
    TListView *lstMessages;
    TListBox *lbxInfo;
    TButton *btnGetVersions;
    TButton *btnInfoClear;
    TButton *btnReset;
    TButton *btnStatus;
    TTimer *tmrDisplay;
    TMemo *txtBitrate;
    TLabel *laBitrate;
    TCheckBox *chbCanFD;
	TGroupBox *grpPackData;
	TLabel *lbl0x410;
	TLabel *lbl0x421;
	TLabel *lbl0x422;
	TLabel *lbl0x423;
	TLabel *lbl0x425;
	TLabel *lbl0x430;
	TEdit *editSoh;
	TEdit *editVoltage;
	TEdit *editSoc;
	TEdit *editCurrent;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label10;
	TEdit *editHiCellVolt;
	TLabel *Label14;
	TEdit *editLoCellVolt;
	TLabel *Label16;
	TEdit *editAvgCellVolt;
	TLabel *Label17;
	TEdit *editHiCellTemp;
	TLabel *Label18;
	TEdit *editLoCellTemp;
	TLabel *Label19;
	TEdit *editAvgCellTemp;
	TLabel *Label20;
	TEdit *editChgLimit;
	TLabel *Label21;
	TEdit *editDisChgLimit;
	TLabel *Label22;
	TEdit *editEndVoltLimit;
	TLabel *Label23;
	TEdit *editState;
	TLabel *Label24;
	TEdit *editStatus;
	TLabel *Label25;
	TLabel *lblModules;
	TEdit *editTotalModules;
	TEdit *editFault;
	TLabel *Label27;
	TEdit *editActiveModules;
	TLabel *lblActiveModules;
	TLabel *Label30;
	TEdit *editIsolation;
	TGroupBox *grpState;
	TLabel *Label28;
	TCheckBox *chkSendState;
	TComboBox *cboState;
	TTimer *tmrState;
	TEdit *editInverter;
	TLabel *Label35;
	TGroupBox *grpPackParam;
	TLabel *Label36;
	TLabel *Label38;
	TComboBox *cboPackParam;
	TEdit *edtPackValue;
	TButton *btnPackParamGet;
	TButton *btnPackParamSet;
	TGroupBox *grpPackSelect;
	TComboBox *cboPackID;
	TLabel *Label37;
	TLabel *lblCANbase;
	TLabel *Label2;
	TComboBox *cboModuleId;
	TCheckBox *chkDMC;
	TGroupBox *grpModuleData;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label11;
	TLabel *Label12;
	TLabel *Label13;
	TLabel *Label15;
	TLabel *Label26;
	TLabel *Label29;
	TLabel *Label31;
	TLabel *Label32;
	TLabel *Label33;
	TLabel *Label34;
	TLabel *Label39;
	TLabel *Label40;
	TLabel *Label41;
	TLabel *Label42;
	TLabel *Label43;
	TLabel *Label44;
	TLabel *Label45;
	TLabel *Label46;
	TLabel *Label47;
	TLabel *Label48;
	TLabel *Label49;
	TLabel *Label50;
	TLabel *Label51;
	TEdit *editModuleSoh;
	TEdit *editModuleVoltage;
	TEdit *editModuleSoc;
	TEdit *editModuleCurrent;
	TEdit *editModuleHiCellVolt;
	TEdit *editModuleLoCellVolt;
	TEdit *Edit7;
	TEdit *editModuleHiCellTemp;
	TEdit *editModuleLoCellTemp;
	TEdit *editModuleAvgCellTemp;
	TEdit *editModuleChargeLimit;
	TEdit *editModuleDischargeLimit;
	TEdit *Edit13;
	TEdit *editModuleState;
	TEdit *editModuleStatus;
	TEdit *editModuleCellCount;
	TEdit *editModuleBalanceActive;
	TEdit *editModuleBalanceStatus;
	TEdit *editModuleIsolation;
	TEdit *editModuleEndVoltage;
	TEdit *editModuleAvgCellVolt;
	TEdit *editSelectedState;
	TEdit *editModuleFault0;
	TEdit *editModuleFault1;
	TEdit *editModuleFault2;
	TEdit *editModuleFault3;
	TEdit *editModuleFault4;
	TLabel *Label54;
	TLabel *Label52;
	TLabel *Label55;
	TLabel *Label56;
	TLabel *Label57;
	TLabel *Label58;
	TButton *btnSendState;
    void __fastcall btnHwRefreshClick(TObject *Sender);
    void __fastcall cbbBaudratesChange(TObject *Sender);
    void __fastcall cbbHwTypeChange(TObject *Sender);
    void __fastcall cbbParameterChange(TObject *Sender);
    void __fastcall cbbChannelChange(TObject *Sender);
    void __fastcall txtIdFromExit(TObject *Sender);
    void __fastcall chbFilterExtClick(TObject *Sender);
    void __fastcall txtIdFromKeyPress(TObject *Sender, char &Key);
	//void __fastcall txtIDExit(TObject *Sender);
	//void __fastcall chbExtendedClick(TObject *Sender);
    void __fastcall txtData0Exit(TObject *Sender);
	//void __fastcall nudLengthClick(TObject *Sender, TUDBtnType Button);
    void __fastcall btnConnectClick(TObject *Sender);
    void __fastcall btnReleaseClick(TObject *Sender);
    void __fastcall rdbTimerClick(TObject *Sender);
    void __fastcall btnFilterApplyClick(TObject *Sender);
    void __fastcall txtDeviceIdOrDelayKeyPress(TObject *Sender, char &Key);
    void __fastcall txtDeviceIdOrDelayExit(TObject *Sender);
	void __fastcall btnParameterSetClick(TObject *Sender);
	void __fastcall btnParameterGetClick(TObject *Sender);
    void __fastcall btnReadClick(TObject *Sender);
    void __fastcall btnMsgClearClick(TObject *Sender);
    //void __fastcall btnSendClick(TObject *Sender);
    void __fastcall btnGetVersionsClick(TObject *Sender);
    void __fastcall btnInfoClearClick(TObject *Sender);
    void __fastcall btnStatusClick(TObject *Sender);
	void __fastcall btnResetClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall btnFilterQueryClick(TObject *Sender);
    void __fastcall chbShowPeriodClick(TObject *Sender);
    void __fastcall tmrDisplayTimer(TObject *Sender);
    void __fastcall chbCanFDClick(TObject *Sender);
	//void __fastcall chbFDClick(TObject *Sender);
	//void __fastcall chbRemoteClick(TObject *Sender);
	void __fastcall tmrStateTimer(TObject *Sender);
	void __fastcall chkSendStateClick(TObject *Sender);
	void __fastcall btnPackParamSetClick(TObject *Sender);
	void __fastcall cboPackIDChange(TObject *Sender);
	void __fastcall chkDMCClick(TObject *Sender);
	void __fastcall btnRefreshClick(TObject *Sender);
	void __fastcall cboModuleIdChange(TObject *Sender);
	void __fastcall cboStateChange(TObject *Sender);
	void __fastcall btnSendStateClick(TObject *Sender);


private:    // User declarations
    // Variables to store the current PCANBasic instance
    //
    PCANBasicClass *m_objPCANBasic;
    // Saves the desired connection mode
    //
    bool m_IsFD;
    // Saves the handle of a PCAN hardware
    //
    TPCANHandle m_PcanHandle;

    // Saves the baudrate register for a conenction
    //
    TPCANBaudrate m_Baudrate;

    // Saves the type of a non-plug-and-play hardware
    //
    TPCANType m_HwType;

    // Variables to store the current reading mode
    // 0 : Timer Mode
    // 1 : Event Mode
    // 2 : Manual Mode
    //
    int m_ActiveReadingMode;

    // CAN messages array. Store the message status for its display
    //
    TList *m_LastMsgsList;

    // Handle to set Received-Event
    //
    HANDLE m_hEvent;

    // Handle to the thread to read using Received-Event method
    //
    HANDLE m_hThread;

    // Handles of non plug and play PCAN-Hardware
    //
    TPCANHandle m_NonPnPHandles[9];
    // Handle for a Critical Section
    //
    CRITICAL_SECTION *m_objpCS;

    void EnableDisableDataFields(int length);
    AnsiString FormatChannelName(TPCANHandle handle);
    AnsiString FormatChannelName(TPCANHandle handle, bool isFD);
    AnsiString GetTPCANHandleName(TPCANHandle handle);
    void FillComboBoxData();
    void ConfigureLogFile();
    void ConfigureTraceFile();
    void SetConnectionStatus(bool bConnected);
    void ReadingModeChanged();

    TPCANStatus ReadMessageFD();
    TPCANStatus ReadMessage();
    void ReadMessages();

	//TPCANStatus WriteFrame();
	//TPCANStatus WriteFrameFD();
    TPCANStatus WriteState();

    void ProcessMessage(TPCANMsgFD theMsg, TPCANTimestampFD itsTimeStamp);
    void ProcessMessage(TPCANMsg MyMsg, TPCANTimestamp itsTimeStamp);
    void InsertMsgEntry(TPCANMsgFD NewMsg, TPCANTimestampFD itsTimeStamp);
    void DisplayMessages();
    void IncludeTextMessage(AnsiString strMsg);
    bool GetFilterStatus(int* status);

	//void TransmitState(packState state);
	void ProcessState(TPCANMsgFD theMsg);
	void ProcessData1(TPCANMsgFD theMsg);
	void ProcessData2(TPCANMsgFD theMsg);
	void ProcessData3(TPCANMsgFD theMsg);
	void ProcessData4(TPCANMsgFD theMsg);
	void ProcessData5(TPCANMsgFD theMsg);
	void ProcessData8(TPCANMsgFD theMsg);
	void ProcessData9(TPCANMsgFD theMsg);
	void ProcessData10(TPCANMsgFD theMsg);
	void ProcessTimeRequest(void);

	void ProcessModuleState(TPCANMsgFD theMsg);
	void ProcessModulePower(TPCANMsgFD theMsg);
	void ProcessModuleCellVoltage(TPCANMsgFD theMsg);
	void ProcessModuleCellTemp(TPCANMsgFD theMsg);
	void ProcessModuleCellId(TPCANMsgFD theMsg);
	void ProcessModuleLimits(TPCANMsgFD theMsg);
	void ProcessModuleList(TPCANMsgFD theMsg);



	void clearData(void);

    void UpdateModuleDisplay(void);

    AnsiString GetFormatedError(TPCANStatus error);

    // Static Thread function to manage reading by event
    //
    static DWORD WINAPI CallCANReadThreadFunc(LPVOID lpParam);
    // Member Thread function to manage reading by event
    //
    DWORD CANReadThreadFunc();

    // Critical section Ini/deinit functions
    //
	void InitializeProtection();
	void FinalizeProtection();

public:     // User declarations
	__fastcall TForm1(TComponent* Owner);
    void InitializeControls(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
