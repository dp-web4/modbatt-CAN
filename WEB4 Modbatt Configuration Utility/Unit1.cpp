//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "can_id_bms_vcu.h"
#include "../../../Pack-Controller-EEPROM/protocols/can_frm_vcu.h"
#include "bms.h"
//#include "WEB4.h"
#include <REST.Client.hpp>
#include <REST.Types.hpp>
#include <REST.Response.Adapter.hpp>
#include <algorithm>  // For std::min


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
uint32_t locked = 0;
uint8_t packID = 0;

batteryModule module[MAX_MODULES_PER_PACK];
batteryPack pack;  // Global pack controller instance




/// <summary>
/// Convert a CAN DLC value into the actual data length of the CAN/CAN-FD frame.
/// </summary>
/// <param name="dlc">A value between 0 and 15 (CAN and FD DLC range)</param>
/// <param name="isSTD">A value indicating if the msg is a standard CAN (FD Flag not checked)</param>
/// <returns>The length represented by the DLC</returns>
static int GetLengthFromDLC(int dlc, bool isSTD)
{
    if (dlc <= 8)
        return dlc;

    if (isSTD)
        return 8;

    switch (dlc)
    {
        case 9: return 12;
        case 10: return 16;
        case 11: return 20;
        case 12: return 24;
        case 13: return 32;
        case 14: return 48;
        case 15: return 64;
        default: return dlc;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Critical Section class
//
clsCritical::clsCritical(CRITICAL_SECTION *cs, bool createUnlocked, bool lockRecursively)
{
    m_objpCS = cs;
    m_dwLocked = -1;
    m_bDoRecursive = lockRecursively;
    m_dwOwnerThread = GetCurrentThreadId();

    if (!createUnlocked)
        Enter();
}

clsCritical::~clsCritical()
{
    int iFail = (int)0x80000000;

    while(m_dwLocked >= 0)
        if (Leave() == iFail)
            break;
}

int clsCritical::Enter()
{
    if (m_dwOwnerThread != GetCurrentThreadId())
		throw "class clsCritical: Thread cross-over error. ";

    try
    {
        if (m_bDoRecursive || (m_dwLocked == -1))
        {
            EnterCriticalSection(m_objpCS);
            InterlockedIncrement(&m_dwLocked);
        }
        return m_dwLocked;
    }
    catch(...)
    {
        return 0x80000000;
    }
}

int clsCritical::Leave()
{
    if (m_dwOwnerThread != GetCurrentThreadId())
        throw "class clsCritical: Thread cross-over error. ";

    try
    {
        if (m_dwLocked >= 0)
        {
            LeaveCriticalSection(m_objpCS);
            InterlockedDecrement(&m_dwLocked);
            return m_dwLocked;
        }
        return -1;
    }
    catch(...)
    {
        return 0x80000000;
    }
}

bool clsCritical::IsLocked()
{
	return (m_dwLocked > -1);
}

int clsCritical::GetRecursionCount()
{
    return m_dwLocked;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// MessageStatus class
//
MessageStatus::MessageStatus(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp, int listIndex)
{
    m_Msg = canMsg;
    m_TimeStamp = canTimestamp;
    m_oldTimeStamp = canTimestamp;
    m_iIndex = listIndex;
    m_Count = 1;
    m_bShowPeriod = true;
    m_bWasChanged = false;
}

void MessageStatus::Update(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp)
{
    m_Msg = canMsg;
    m_oldTimeStamp = m_TimeStamp;
    m_TimeStamp = canTimestamp;
    m_bWasChanged = true;
    m_Count += 1;
}

AnsiString MessageStatus::GetTypeString()
{
	AnsiString strTemp;
	bool isEcho = (m_Msg.MSGTYPE & PCAN_MESSAGE_ECHO) != 0;

	// Add the new ListView Item with the type of the message
	//
	if ((m_Msg.MSGTYPE & PCAN_MESSAGE_STATUS) != 0)
        return "STATUS";

    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME) != 0)
        return "ERROR";

    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0)
        strTemp = "EXT";
    else
		strTemp = "STD";

    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_RTR) == PCAN_MESSAGE_RTR)
		strTemp += isEcho ? ("/RTR [ ECHO ]") : ("/RTR");
	else
		if (m_Msg.MSGTYPE > PCAN_MESSAGE_EXTENDED)
		{
			if(isEcho)
				strTemp += (" [ ECHO");
			else
				strTemp += (" [ ");
			if (m_Msg.MSGTYPE & PCAN_MESSAGE_FD)
                strTemp += (" FD");
            if (m_Msg.MSGTYPE & PCAN_MESSAGE_BRS)
				strTemp += (" BRS");
            if (m_Msg.MSGTYPE & PCAN_MESSAGE_ESI)
                strTemp += (" ESI");
            strTemp += (" ]");
        }

    return strTemp;
}

AnsiString MessageStatus::GetIdString()
{
    // We format the ID of the message and show it
    //
	if ((m_Msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0)
		return IntToHex((int)m_Msg.ID,8) + "h";
	else
		return IntToHex((int)m_Msg.ID,3) + "h";
}

AnsiString MessageStatus::GetDataString()
{
    AnsiString strTemp, strTemp2;

	strTemp = "";
    strTemp2 = "";

    if ((m_Msg.MSGTYPE & PCAN_MESSAGE_RTR) == PCAN_MESSAGE_RTR)
        return "Remote Request";
    else
        for(int i=0; i < GetLengthFromDLC(m_Msg.DLC, !(m_Msg.MSGTYPE & PCAN_MESSAGE_FD)); i++)
        {
            strTemp = AnsiString::Format("%s %02X", ARRAYOFCONST((strTemp2, m_Msg.DATA[i])));
            strTemp2 = strTemp;
        }

    return strTemp2;
}

AnsiString MessageStatus::GetTimeString()
{
    double fTime;

    fTime = (m_TimeStamp / 1000.0);
    if (m_bShowPeriod)
        fTime -= (m_oldTimeStamp / 1000.0);

    // Use sprintf to avoid the double-to-TVarRec ambiguity
    AnsiString result;
    result.sprintf("%.1f", fTime);
    return result;
}

void MessageStatus::SetShowingPeriod(bool value)
{
    if (m_bShowPeriod ^ value)
    {
        m_bShowPeriod = value;
		m_bWasChanged = true;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    // Initialize all
	//
	uint8_t index;
	for(index = 0;index<MAX_MODULES_PER_PACK;index++){
		memset(&module[index],0,sizeof(module[index]));
	}

	InitializeControls();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    // Release Hardware if needed
    //
    if (btnRelease->Enabled)
        btnRelease->Click();

    // Close the event
    CloseHandle(m_hEvent);

    //Free Ressources
    //
    delete m_objPCANBasic;

    // (Protected environment)
    //
	{
		clsCritical locker(m_objpCS);

        while(m_LastMsgsList->Count)
        {
            // Explicit cast to correct pointer type before deletion
            delete static_cast<MessageStatus*>(m_LastMsgsList->First());
			//delete m_LastMsgsList->First();
            m_LastMsgsList->Delete(0);
        }
        delete m_LastMsgsList;
    }

    // Uninitialize the Critical Section
    //
	FinalizeProtection();

	// Clean up Web4 connection
	if (FWeb4Client) {
		FWeb4Client->Disconnect();
		FWeb4Client.reset();
	}


}
//---------------------------------------------------------------------------

void TForm1::InitializeControls(void)
{
    // Initialize the Critical Section
	//
	InitializeProtection();

    // Creates an array with all possible non plug-and-play PCAN-Channels
	//

	m_NonPnPHandles[0] = PCAN_ISABUS1;
	m_NonPnPHandles[1] = PCAN_ISABUS2;
	m_NonPnPHandles[2] = PCAN_ISABUS3;
	m_NonPnPHandles[3] = PCAN_ISABUS4;
	m_NonPnPHandles[4] = PCAN_ISABUS5;
	m_NonPnPHandles[5] = PCAN_ISABUS6;
	m_NonPnPHandles[6] = PCAN_ISABUS7;
	m_NonPnPHandles[7] = PCAN_ISABUS8;
	m_NonPnPHandles[8] = PCAN_DNGBUS1;


	// We set the variable for the current
    // PCAN Basic Class instance to use it
    //
    m_objPCANBasic = new PCANBasicClass();

	// As defautl, normal CAN hardware is used
    //
    m_IsFD = false;

    // Create a list to store the displayed messages
    //
    m_LastMsgsList = new TList();

    // Create Event to use Received-event
    //
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, "");

    // Set the Read-thread variable to null
    //
    m_hThread = NULL;

    // We set the variable to know which reading mode is
    // currently selected (Event by default)
	//
	m_ActiveReadingMode = 1;

    // Prepares the PCAN-Basic's debug-Log file
	//
    FillComboBoxData();

    // Init log parameters
    ConfigureLogFile();

    // Set default connection status
    //
	SetConnectionStatus(false);

	//web 4 testing
	grpCanData->Visible = false;
	grpWeb4->Visible = true;
		  // Initialize WEb4 default values
	edtBatteryModuleId->Text = "MODBATT-MOD-RC001-001";
	edtBatteryPackId->Text = "MODBATT-PACK-RC001-A";
	edtHostSystemId->Text = "MODBATT-HOST-RC001";

// Configure Web4 settings
	FConfig.ApiEndpoint = "http://localhost";
	FConfig.RestPort = "8082";
    FConfig.RequestTimeoutSeconds = 30;
    FConfig.DefaultCreator = "alice";
	//FConfig.EnableLogging = chkEnableLogging->Checked;
	//FConfig.LogLevel = "info";



	TPCANMsgFD NewMsg;
	TPCANTimestampFD timeStamp;
	MessageStatus *msgStsCurrentMsg;
	TListItem *CurrentItem;
	NewMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

	NewMsg.DLC = 8;

}
//---------------------------------------------------------------------------
void TForm1::InitializeProtection()
{
	m_objpCS = new CRITICAL_SECTION();
	InitializeCriticalSection(m_objpCS);
}

//---------------------------------------------------------------------------
void TForm1::FinalizeProtection()
{
	try
	{
		DeleteCriticalSection(m_objpCS);
		delete m_objpCS;
		m_objpCS = NULL;
    }
	catch(...)
	{
		throw;
	}
}
//---------------------------------------------------------------------------

void TForm1::FillComboBoxData()
{
    // Channels will be check
    //
    btnHwRefresh->Click();

    // FD Bitrate:
    //      Arbitration: 1 Mbit/sec
    //      Data: 2 Mbit/sec
    //
    txtBitrate->Text =   "f_clock_mhz=20, nom_brp=5, nom_tseg1=2, nom_tseg2=1, nom_sjw=1, data_brp=2, data_tseg1=3, data_tseg2=1, data_sjw=1";

    // TPCANBaudrate
    //
    cbbBaudrates->ItemIndex =2; // 500 K
    cbbBaudratesChange(cbbBaudrates);

    // Hardware Type for no plugAndplay hardware
    //
    cbbHwType->ItemIndex = 0;
    cbbHwTypeChange(cbbHwType);

    // Interrupt for no plugAndplay hardware
    //
    cbbInterrupt->ItemIndex = 0;

    // IO Port for no plugAndplay hardware
    //
    cbbIO->ItemIndex = 0;

    // Parameters for GetValue and SetValue function calls
	//
    cbbParameter->ItemIndex = 0;
    cbbParameterChange(cbbParameter);
}
//---------------------------------------------------------------------------

AnsiString TForm1::FormatChannelName(TPCANHandle handle, bool isFD)
{
    AnsiString strName;
    BYTE byChannel;

    // Gets the owner device and channel for a
    // PCAN-Basic handle
    //
    if (handle < 0x100)
        byChannel = (BYTE)(handle) & 0xF;
    else
        byChannel = (BYTE)(handle) & 0xFF;

    // Constructs the PCAN-Basic Channel name and return it
    //
    return Format(isFD ? "%s:FD %d (%Xh)" : "%s %d (%Xh)", ARRAYOFCONST((GetTPCANHandleName(handle), byChannel, handle)));
}
//---------------------------------------------------------------------------

AnsiString TForm1::FormatChannelName(TPCANHandle handle)
{
    return FormatChannelName(handle, false);
}

//---------------------------------------------------------------------------
AnsiString TForm1::GetTPCANHandleName(TPCANHandle handle)
{
    AnsiString result = "PCAN_NONE";

    switch(handle)
    {
        case PCAN_ISABUS1:
        case PCAN_ISABUS2:
        case PCAN_ISABUS3:
        case PCAN_ISABUS4:
        case PCAN_ISABUS5:
        case PCAN_ISABUS6:
		case PCAN_ISABUS7:
        case PCAN_ISABUS8:
			result = "PCAN_ISA";
            break;

        case PCAN_DNGBUS1:
            result = "PCAN_DNG";
            break;

        case PCAN_PCIBUS1:
        case PCAN_PCIBUS2:
        case PCAN_PCIBUS3:
        case PCAN_PCIBUS4:
        case PCAN_PCIBUS5:
        case PCAN_PCIBUS6:
        case PCAN_PCIBUS7:
        case PCAN_PCIBUS8:
        case PCAN_PCIBUS9:
        case PCAN_PCIBUS10:
        case PCAN_PCIBUS11:
        case PCAN_PCIBUS12:
		case PCAN_PCIBUS13:
        case PCAN_PCIBUS14:
        case PCAN_PCIBUS15:
        case PCAN_PCIBUS16:
            result = "PCAN_PCI";
            break;

        case PCAN_USBBUS1:
        case PCAN_USBBUS2:
        case PCAN_USBBUS3:
        case PCAN_USBBUS4:
        case PCAN_USBBUS5:
        case PCAN_USBBUS6:
        case PCAN_USBBUS7:
        case PCAN_USBBUS8:
        case PCAN_USBBUS9:
        case PCAN_USBBUS10:
        case PCAN_USBBUS11:
        case PCAN_USBBUS12:
        case PCAN_USBBUS13:
        case PCAN_USBBUS14:
        case PCAN_USBBUS15:
        case PCAN_USBBUS16:
            result = "PCAN_USB";
            break;

        case PCAN_PCCBUS1:
        case PCAN_PCCBUS2:
            result = "PCAN_PCC";
            break;

        case PCAN_LANBUS1:
        case PCAN_LANBUS2:
        case PCAN_LANBUS3:
        case PCAN_LANBUS4:
        case PCAN_LANBUS5:
        case PCAN_LANBUS6:
        case PCAN_LANBUS7:
        case PCAN_LANBUS8:
        case PCAN_LANBUS9:
        case PCAN_LANBUS10:
        case PCAN_LANBUS11:
		case PCAN_LANBUS12:
        case PCAN_LANBUS13:
        case PCAN_LANBUS14:
        case PCAN_LANBUS15:
        case PCAN_LANBUS16:
            result = "PCAN_LAN";
            break;
    }
    return result;
}
//---------------------------------------------------------------------------

void TForm1::ConfigureLogFile()
{
    int iBuffer;

    // Sets the mask to catch all events
    //
    iBuffer = LOG_FUNCTION_ALL;

    // Configures the log file.
    // NOTE: The Log capability is to be used with the NONEBUS Handle. Other handle than this will
	// cause the function fail.
    //
    m_objPCANBasic->SetValue(PCAN_NONEBUS, PCAN_LOG_CONFIGURE, (void*)&iBuffer, sizeof(iBuffer));
}
//---------------------------------------------------------------------------

void TForm1::ConfigureTraceFile()
{
    int iBuffer;
    TPCANStatus stsResult;

    // Configure the maximum size of a trace file to 5 megabytes
    //
    iBuffer = 5;
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_SIZE, (void*)&iBuffer, sizeof(iBuffer));
    if (stsResult != PCAN_ERROR_OK)
        IncludeTextMessage(GetFormatedError(stsResult));

    // Configure the way how trace files are created:
    // * Standard name is used
    // * Existing file is ovewritten,
    // * Only one file is created.
    // * Recording stopts when the file size reaches 5 megabytes.
    //
    iBuffer = TRACE_FILE_SINGLE | TRACE_FILE_OVERWRITE;
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_CONFIGURE, (void*)&iBuffer, sizeof(iBuffer));
    if (stsResult != PCAN_ERROR_OK)
        IncludeTextMessage(GetFormatedError(stsResult));
}
//---------------------------------------------------------------------------

void TForm1::SetConnectionStatus(bool bConnected)
{
    // Buttons
    //
	btnConnect->Enabled = !bConnected;
	//btnRead->Enabled = (bConnected && rdbManual->Checked);
	btnRelease->Enabled = bConnected;
	chkSendState->Enabled = bConnected;
	cboState->Enabled = bConnected;
	editInverter->Enabled = bConnected;
	//btnFilterApply->Enabled = bConnected;
	//btnFilterQuery->Enabled = bConnected;
    btnGetVersions->Enabled = bConnected;
    btnHwRefresh->Enabled = !bConnected;
    btnStatus->Enabled = bConnected;
    btnReset->Enabled = bConnected;
	tmrState->Enabled = bConnected;
	btnSendState->Enabled = bConnected;
    cboPackParam->Enabled = bConnected;
	btnPackParamSet->Enabled  = bConnected;
	btnPackParamGet->Enabled  = bConnected;
	edtPackValue->Enabled     = bConnected;
	cboPackID->Enabled          = bConnected;
	chkDMC->Enabled             = bConnected;


	// ComboBoxs
    //
    cbbBaudrates->Enabled = !bConnected;
    cbbChannel->Enabled = !bConnected;
    cbbHwType->Enabled = !bConnected;
    cbbIO->Enabled = !bConnected;
    cbbInterrupt->Enabled = !bConnected;

    // Check-Buttons
    //
    chbCanFD->Enabled = !bConnected;

    // Hardware configuration and read mode
    //
    if (!bConnected)
        cbbChannelChange(cbbChannel);
    else
        ReadingModeChanged();

    // Display messages in grid
    //
	tmrDisplay->Enabled = bConnected;

	if(!bConnected) {
		editState->Text 				= "0";
		editStatus->Text 				= "0";
		editSoh->Text 					= "0";
		editTotalModules->Text 			= "0";
		editFault->Text 				= "0";
		editActiveModules->Text 		= "0";
		editVoltage->Text 				= "0";
		editCurrent->Text 				= "0";
		editHiCellVolt->Text 		   	= "0";
		editLoCellVolt->Text 			= "0";
		editAvgCellVolt->Text 			= "0";
		editSoc->Text 					= "0";
		editHiCellTemp->Text 			= "0";
		editLoCellTemp->Text 			= "0";
		editAvgCellTemp->Text 			= "0";
		editChgLimit->Text 				= "0";
		editDisChgLimit->Text 			= "0";
		editEndVoltLimit->Text 			= "0";
		editIsolation->Text				= "0";

		//Module Data
		editModuleState->Text 			= "0";
		editModuleStatus->Text 			= "0";
		editModuleSoc->Text 			= "0";
		editModuleSoh->Text 			= "0";
		editModuleBalanceActive->Text 	= "0";
		editModuleBalanceStatus->Text 	= "0";
		editModuleCellCount->Text       = "0";
		editModuleFault0->Text 			= "0";
		editModuleFault1->Text 			= "0";
		editModuleFault2->Text 			= "0";
		editModuleFault3->Text 			= "0";
		editModuleFault4->Text 			= "0";
		editModuleVoltage->Text			= "0";
		editModuleCurrent->Text 		= "0";
		editModuleHiCellVolt->Text 		= "0";
		editModuleLoCellVolt->Text 		= "0";
		editModuleAvgCellVolt->Text 	= "0";
		editModuleHiCellTemp->Text 		= "0";
		editModuleLoCellTemp->Text 		= "0";
		editModuleAvgCellTemp->Text 	= "0";
		editModuleChargeLimit->Text 	= "0";
		editModuleDischargeLimit->Text	= "0";
		editModuleEndVoltage->Text 		= "0";
		editModuleIsolation->Text 		= "0";

	}
}
//---------------------------------------------------------------------------

AnsiString TForm1::GetFormatedError(TPCANStatus error)
{
    TPCANStatus status;
    char buffer[256];
    AnsiString result;

    memset(buffer,'\0',256);
	// Gets the text using the GetErrorText API function
    // If the function success, the translated error is returned. If it fails,
    // a text describing the current error is returned.
    //
    status = m_objPCANBasic->GetErrorText(error, 0, buffer);
    if (status != PCAN_ERROR_OK)
        result = Format("An error ocurred. Error-code's text (%Xh) couldn't be retrieved", ARRAYOFCONST((error)));
    else
        result = buffer;
    return result;
}
//---------------------------------------------------------------------------

void TForm1::IncludeTextMessage(AnsiString strMsg)
{
    // Inserts a message in the Info box and focuses it
    //
    lbxInfo->Items->Add(strMsg);
    lbxInfo->ItemIndex = lbxInfo->Items->Count-1;
}
//---------------------------------------------------------------------------

bool TForm1::GetFilterStatus(int* status)
{
    TPCANStatus stsResult;

    // Tries to get the status of the filter for the current connected hardware
    //
    stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_MESSAGE_FILTER, (void*)status, sizeof(int));

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
    {
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------

void TForm1::InsertMsgEntry(TPCANMsgFD NewMsg, TPCANTimestampFD timeStamp)
{
	MessageStatus *msgStsCurrentMsg;
	TListItem *CurrentItem;

	// (Protected environment)
	//
	{
		if(locked == 0)
		{
			//clsCritical locker(m_objpCS);
			 locked = 1;
			// We add this status in the last message list
			//
			msgStsCurrentMsg = new MessageStatus(NewMsg, timeStamp, lstMessages->Items->Count);
			msgStsCurrentMsg->ShowingPeriod = chbShowPeriod->Checked;
			m_LastMsgsList->Add(msgStsCurrentMsg);

			// Add the new ListView Item with the Type of the message
			//
			CurrentItem = lstMessages->Items->Add();
			CurrentItem->Caption = msgStsCurrentMsg->TypeString;

			// We set the ID of the message
			//
			CurrentItem->SubItems->Add(msgStsCurrentMsg->IdString);
			// We set the length of the Message
			//
			CurrentItem->SubItems->Add(IntToStr(GetLengthFromDLC(NewMsg.DLC, !(NewMsg.MSGTYPE & PCAN_MESSAGE_FD))));
			// we set the message count message (this is the First, so count is 1)
			//
			CurrentItem->SubItems->Add(IntToStr(msgStsCurrentMsg->Count));
			// Add timestamp information
			//
			CurrentItem->SubItems->Add(msgStsCurrentMsg->TimeString);
			// We set the data of the message.
			//
			CurrentItem->SubItems->Add(msgStsCurrentMsg->DataString);
			locked=0;
		   }
	}
}
//---------------------------------------------------------------------------

void TForm1::DisplayMessages()
{
    TListItem *CurrentItem;
    MessageStatus *msgStatus;

    // We search if a message (Same ID and Type) is
    // already received or if this is a new message
    // (in a protected environment)
	//
	if(locked ==0)
	{
		locked =1;
		//clsCritical locker(m_objpCS);
        for(int i=0; i < m_LastMsgsList->Count; i++)
        {
            msgStatus = (MessageStatus*)m_LastMsgsList->Items[i];

            if (msgStatus->MarkedAsUpdated)
            {
                msgStatus->MarkedAsUpdated = false;
                CurrentItem = lstMessages->Items->Item[msgStatus->Position];

				CurrentItem->SubItems->Strings[1] = IntToStr(GetLengthFromDLC(msgStatus->CANMsg.DLC, !(msgStatus->CANMsg.MSGTYPE & PCAN_MESSAGE_FD)));
                CurrentItem->SubItems->Strings[2] = IntToStr(msgStatus->Count);
                CurrentItem->SubItems->Strings[3] = msgStatus->TimeString;
                CurrentItem->SubItems->Strings[4] = msgStatus->DataString;
            }
		}
		locked =0;
    }
}
//---------------------------------------------------------------------------

TPCANStatus TForm1::ReadMessageFD()
{
    TPCANMsgFD CANMsg;
	TPCANTimestampFD CANTimeStamp;
    TPCANStatus stsResult;

    // We execute the "Read" function of the PCANBasic
    //
    stsResult = m_objPCANBasic->ReadFD(m_PcanHandle, &CANMsg, &CANTimeStamp);
    if (stsResult != PCAN_ERROR_QRCVEMPTY)
        // We process the received message
        //
        ProcessMessage(CANMsg, CANTimeStamp);

	return stsResult;
}
//---------------------------------------------------------------------------

TPCANStatus TForm1::ReadMessage()
{
    TPCANMsg CANMsg;
    TPCANTimestamp CANTimeStamp;
    TPCANStatus stsResult;

    // We execute the "Read" function of the PCANBasic
    //
    stsResult = m_objPCANBasic->Read(m_PcanHandle, &CANMsg, &CANTimeStamp);
    if (stsResult != PCAN_ERROR_QRCVEMPTY)
        // We process the received message
        //
        ProcessMessage(CANMsg, CANTimeStamp);

    return stsResult;
}
//---------------------------------------------------------------------------

void TForm1::ReadMessages()
{
    TPCANStatus stsResult;

    // We read at least one time the queue looking for messages.
    // If a message is found, we look again trying to find more.
    // If the queue is empty or an error occurr, we get out from
    // the dowhile statement.
    //
    do
    {
        stsResult = m_IsFD ? ReadMessageFD() : ReadMessage();
        if (stsResult == PCAN_ERROR_ILLOPERATION)
            break;
    } while (btnRelease->Enabled && (!(stsResult & PCAN_ERROR_QRCVEMPTY)));
}
//---------------------------------------------------------------------------

void TForm1::ProcessMessage(TPCANMsgFD theMsg, TPCANTimestampFD itsTimeStamp)
{
	MessageStatus *msg;


	// OK SO HERE WE ARE GOING TO PROCESS THE DATA AND SHOW IT ON THE FORM
	// THEN WE WILL COME BACK IN AND UPDATE THE MESSAGE LIST
	if(theMsg.ID == ID_BMS_DATA_1 + (packID * 0x100)){
		  ProcessData1(theMsg);
	}else if(theMsg.ID == ID_BMS_DATA_2 + (packID * 0x100)){
		  ProcessData2(theMsg);
	} else if (theMsg.ID == ID_BMS_DATA_3 + (packID * 0x100)){
		  ProcessData3(theMsg);
	}else if(theMsg.ID == ID_BMS_DATA_5 + (packID * 0x100)){
		  ProcessData5(theMsg);
	}else if(theMsg.ID == ID_BMS_DATA_8 + (packID * 0x100)){
		  ProcessData8(theMsg);
	}else if(theMsg.ID == ID_BMS_DATA_9 + (packID * 0x100)){
		  ProcessData9(theMsg);
	}else if(theMsg.ID == ID_BMS_DATA_10 + (packID * 0x100)){
		  ProcessData10(theMsg);
	}else if(theMsg.ID == ID_BMS_STATE + (packID * 0x100)){
		  ProcessState(theMsg);
	}else if(theMsg.ID == ID_BMS_TIME_REQUEST  + (packID * 0x100)){
		  ProcessTimeRequest();
	}else if(theMsg.ID == ID_MODULE_STATE  + (packID * 0x100)){
		  ProcessModuleState(theMsg);
	}else if(theMsg.ID == ID_MODULE_POWER  + (packID * 0x100)){
		  ProcessModulePower(theMsg);
	}else if(theMsg.ID == ID_MODULE_CELL_VOLTAGE  + (packID * 0x100)){
		  ProcessModuleCellVoltage(theMsg);
	}else if(theMsg.ID == ID_MODULE_CELL_TEMP  + (packID * 0x100)){
		  ProcessModuleCellTemp(theMsg);
	}else if(theMsg.ID == ID_MODULE_CELL_ID  + (packID * 0x100)){
		  ProcessModuleCellId(theMsg);
	}else if(theMsg.ID == ID_MODULE_LIMITS  + (packID * 0x100)){
		  ProcessModuleLimits(theMsg);
	}else if(theMsg.ID == ID_MODULE_LIST  + (packID * 0x100)){
		  ProcessModuleList(theMsg);
	}



	// We search if a message (Same ID and Type) is
		// already received or if this is a new message
		//
	{

		//clsCritical locker(m_objpCS);

		for(int i=0; i < m_LastMsgsList->Count; i++)
		{
			msg = (MessageStatus*)m_LastMsgsList->Items[i];
			if ((msg->CANMsg.ID == theMsg.ID) && (msg->CANMsg.MSGTYPE == theMsg.MSGTYPE))
			{
				// Modify the message and exit
				//
				msg->Update(theMsg, itsTimeStamp);
				return;
			}
		}
		// Message not found. It will created
		//
		InsertMsgEntry(theMsg, itsTimeStamp);

	   /*
	MessageStatus *msgStsCurrentMsg;
	TListItem *CurrentItem;
	//NewMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

	//NewMsg.DLC = 8;




	msgStsCurrentMsg = new MessageStatus(theMsg, itsTimeStamp, lstMessages->Items->Count);
	msgStsCurrentMsg->ShowingPeriod = chbShowPeriod->Checked;
	m_LastMsgsList->Add(msgStsCurrentMsg);



		// Add the new ListView Item with the Type of the message
		//
		CurrentItem = lstMessages->Items->Add();
		CurrentItem->Caption = msgStsCurrentMsg->TypeString;

		// We set the ID of the message
		//
		CurrentItem->SubItems->Add(msgStsCurrentMsg->IdString);
		// We set the length of the Message
		//
		CurrentItem->SubItems->Add(IntToStr(GetLengthFromDLC(theMsg.DLC, !(theMsg.MSGTYPE & PCAN_MESSAGE_FD))));
		// we set the message count message (this is the First, so count is 1)
		//
		CurrentItem->SubItems->Add(IntToStr(msgStsCurrentMsg->Count));
		// Add timestamp information
		//
		CurrentItem->SubItems->Add(msgStsCurrentMsg->TimeString);
		// We set the data of the message.
		//
		CurrentItem->SubItems->Add(msgStsCurrentMsg->DataString);



	//msg = (MessageStatus*)m_LastMsgsList->Items[(m_LastMsgsList->Count)-1];
	//msg->Update(theMsg, itsTimeStamp);
      	*/
	}



}
//---------------------------------------------------------------------------

void TForm1::ProcessMessage(TPCANMsg theMsg, TPCANTimestamp itsTimeStamp)
{
	TPCANMsgFD newMsg;
	TPCANTimestampFD newTimestamp;

	newMsg = TPCANMsgFD();
	newMsg.ID = theMsg.ID;
	newMsg.DLC = theMsg.LEN;
	for (int i = 0; i < ((theMsg.LEN > 8) ? 8 : theMsg.LEN); i++)
		newMsg.DATA[i] = theMsg.DATA[i];
	newMsg.MSGTYPE = theMsg.MSGTYPE;

	newTimestamp = itsTimeStamp.micros + (1000UI64 * itsTimeStamp.millis) + (0x100000000UI64 * 1000UI64 * itsTimeStamp.millis_overflow);
	ProcessMessage(newMsg, newTimestamp);

}
//---------------------------------------------------------------------------

void TForm1::ReadingModeChanged()
{
    if (!btnRelease->Enabled)
        return;

    switch(m_ActiveReadingMode)
    {
        case 0:     // If active reading mode is By Timer
            // Terminate Read Thread if it exists
            //
            if (m_hThread != NULL)
            {
                TerminateThread(m_hThread, -1000);
                m_hThread = NULL;
            }
            // We start to read
            //
			//tmrRead->Enabled = true;
            break;
        case 1:     // If active reading mode is By Event
            // We stop to read from the CAN queue
            //
			//tmrRead->Enabled = false;

            // Create Reading Thread ....
            //
            m_hThread = CreateThread(NULL, NULL, TForm1::CallCANReadThreadFunc, (LPVOID)this, NULL, NULL);

            if (m_hThread == NULL)
                ::MessageBox(NULL,"Create CANRead-Thread failed","Error!",MB_ICONERROR);
            break;
        default:    // If active reading mode is Manual
            // Terminate Read Thread if it exists
            //
            if (m_hThread != NULL)
            {
                TerminateThread(m_hThread, -1000);
                m_hThread = NULL;
            }
            // We enable the button for read
            //
			//tmrRead->Enabled = false;
			//btnRead->Enabled = (btnRelease->Enabled && rdbManual->Checked);
            break;
    }
}
//---------------------------------------------------------------------------

DWORD WINAPI TForm1::CallCANReadThreadFunc(LPVOID lpParam)
{
    // Cast lpParam argument to PCANBasicExampleDlg*
    //
    TForm1* dialog = (TForm1*)lpParam;

    // Call TForm1 Thread member function
    //
	return dialog->CANReadThreadFunc();
}
//---------------------------------------------------------------------------

DWORD TForm1::CANReadThreadFunc()
{
    TPCANStatus stsResult;
    DWORD result, dwTemp;

    // Sets the handle of the Receive-Event.
    //
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_EVENT ,&m_hEvent, sizeof(m_hEvent));

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
    {
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
        return 1;
    }

    // While this mode is selected
    //
	while(1)//rdbEvent->Checked)
	{
		//Wait for CAN Data...
		result = WaitForSingleObject(m_hEvent, INFINITE);

		if (result == WAIT_OBJECT_0)
			ReadMessages();
	}

    // Resets the Event-handle configuration
	//
	dwTemp = 0;
    m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_EVENT ,&dwTemp, sizeof(dwTemp));

    return 0;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

void __fastcall TForm1::btnHwRefreshClick(TObject *Sender)
{
    TPCANChannelInformation *info;
    TPCANStatus stsResult;
    DWORD iChannelsCount;
    bool bIsFD;

    // Clears the Channel combioBox and fill it again with
    // the PCAN-Basic handles for no-Plug&Play hardware and
    // the detected Plug&Play hardware
    //
    cbbChannel->Items->Clear();

    // Includes all no-Plug&Play Handles
    for (int i = 0; i < (int)(sizeof(m_NonPnPHandles) /sizeof(TPCANHandle)) ; i++)
        cbbChannel->Items->Add(FormatChannelName(m_NonPnPHandles[i]));

	stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS_COUNT, (void*)&iChannelsCount, sizeof(iChannelsCount));
    if (stsResult == PCAN_ERROR_OK)
    {
        info = new TPCANChannelInformation[iChannelsCount];
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS, (void*)info, iChannelsCount * sizeof(TPCANChannelInformation));
        if (stsResult == PCAN_ERROR_OK)
            // Include only connectable channels
            //
            for (int i=0; i < (int)iChannelsCount; i++)
                if (info[i].channel_condition & PCAN_CHANNEL_AVAILABLE)
                {
                    bIsFD = info[i].device_features & FEATURE_FD_CAPABLE;
                    cbbChannel->Items->Add(FormatChannelName(info[i].channel_handle, bIsFD));
                }
        delete [] info;
    }

    cbbChannel->ItemIndex = cbbChannel->Items->Count - 1;
    cbbChannelChange(cbbChannel);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnConnectClick(TObject *Sender)
{
	TPCANStatus stsResult;
    int selectedIO;
    int selectedInterrupt;

    // Parse IO and Interrupt
    //
    selectedIO = StrToInt("0x"+cbbIO->Text);
	selectedInterrupt = StrToInt("0x"+cbbInterrupt->Text);

    // Connects a selected PCAN-Basic channel
    //
	if (m_IsFD)
		stsResult = m_objPCANBasic->InitializeFD(m_PcanHandle, AnsiString(txtBitrate->Text).c_str());
    else
        stsResult = m_objPCANBasic->Initialize(m_PcanHandle, m_Baudrate, m_HwType, selectedIO, selectedInterrupt);

    if (stsResult != PCAN_ERROR_OK)
        if (stsResult != PCAN_ERROR_CAUTION)
            ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
        else
        {
            IncludeTextMessage("******************************************************");
            IncludeTextMessage("The bitrate being used is different than the given one");
            IncludeTextMessage("******************************************************");
            stsResult = PCAN_ERROR_OK;
        }
    else
        // Prepares the PCAN-Basic's PCAN-Trace file
        //
        ConfigureTraceFile();

    // Sets the connection status of the main-form
    //
	SetConnectionStatus(stsResult == PCAN_ERROR_OK);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnReleaseClick(TObject *Sender)
{
	// Terminate Read Thread if it exists
	//
    if (m_hThread != NULL)
    {
        TerminateThread(m_hThread, 1000);
        m_hThread = NULL;
	}

    // We stop to read from the CAN queue
    //
	//tmrRead->Enabled = false;

    // Releases a current connected PCAN-Basic channel
    //
    m_objPCANBasic->Uninitialize(m_PcanHandle);

    // Sets the connection status of the main-form
    //
	SetConnectionStatus(false);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnFilterApplyClick(TObject *Sender)
{
	/*
	int iBuffer;
    AnsiString info;
    TPCANStatus stsResult;

    // Gets the current status of the message filter
    //
	if (!GetFilterStatus(&iBuffer))
        return;

    // Configures the message filter for a custom range of messages
    //
if (rdbFilterCustom->Checked)
	{
		// Sets the custom filter
		//
		stsResult = m_objPCANBasic->FilterMessages(m_PcanHandle, StrToInt("0x"+txtIdFrom->Text), StrToInt("0x"+txtIdTo->Text), chbFilterExt->Checked ? PCAN_MODE_EXTENDED : PCAN_MODE_STANDARD);
		// If success, an information message is written, if it is not, an error message is shown
		//
		if (stsResult == PCAN_ERROR_OK)
		{
			info = Format("The filter was customized. IDs from 0x%s to 0x%s will be received", ARRAYOFCONST((txtIdFrom->Text,txtIdTo->Text)));
			IncludeTextMessage(info);
		}
		else
			::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);

		return;
	}

    // The filter will be full opened or complete closed
    //
    if (rdbFilterClose->Checked)
        iBuffer = PCAN_FILTER_CLOSE;
    else
        iBuffer = PCAN_FILTER_OPEN;

    // The filter is configured
    //
    stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_MESSAGE_FILTER, (void*)&iBuffer, sizeof(int));

    // If success, an information message is written, if it is not, an error message is shown
	//
    if (stsResult == PCAN_ERROR_OK)
    {
        info = Format("The filter was successfully %s", ARRAYOFCONST((rdbFilterClose->Checked ? "closed." : "opened.")));
        IncludeTextMessage(info);
    }
    else
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
			  */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnFilterQueryClick(TObject *Sender)
{
    int iBuffer;

    // Queries the current status of the message filter
    //
    if (GetFilterStatus(&iBuffer))
    {
		switch(iBuffer)
		{
			// The filter is closed
			//
		case PCAN_FILTER_CLOSE:
            IncludeTextMessage("The Status of the filter is: closed.");
            break;
            // The filter is fully opened
			//
        case PCAN_FILTER_OPEN:
            IncludeTextMessage("The Status of the filter is: full opened.");
            break;
            // The filter is customized
            //
        case PCAN_FILTER_CUSTOM:
            IncludeTextMessage("The Status of the filter is: customized.");
            break;
            // The status of the filter is undefined. (Should never happen)
            //
        default:
            IncludeTextMessage("The Status of the filter is: Invalid.");
            break;
        }
	}
}

void __fastcall TForm1::btnParameterSetClick(TObject *Sender)
{
	TPCANStatus stsResult;
	unsigned int iBuffer;
	AnsiString info;
	TCHAR szDirectory[MAX_PATH] = "";
	bool bActivate;

    bActivate = rdbParamActive->Checked;

    // Sets a PCAN-Basic parameter value
    //
    switch (cbbParameter->ItemIndex)
    {
        // The device identifier of a channel will be set
        //
	case 0:
		iBuffer = StrToInt("0x" + txtDeviceIdOrDelay->Text);
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_DEVICE_ID, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
            IncludeTextMessage("The desired Device-ID was successfully configured");
        break;

        // The 5 Volt Power feature of a channel will be set
        //
    case 1:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_5VOLTS_POWER, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The USB/PC-Card 5 power was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for automatic reset on BUS-OFF will be set
        //
    case 2:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_BUSOFF_AUTORESET, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The automatic-reset on BUS-OFF was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
		break;

        // The CAN option "Listen Only" will be set
        //
    case 3:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_LISTEN_ONLY, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The CAN option \"Listen Only\" was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for logging debug-information will be set
        //
    case 4:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(PCAN_NONEBUS, PCAN_LOG_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The feature for logging debug information was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
            ::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);
            info = Format("Log file folder: %s" , ARRAYOFCONST((szDirectory)));
            IncludeTextMessage(info);
        }
        break;

        // The channel option "Receive Status" will be set
        //
    case 5:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_RECEIVE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
			info = Format("The channel option \"Receive Status\" was set to %s", ARRAYOFCONST((bActivate ? "ON" : "OFF")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for tracing will be set
        //
    case 7:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_TRACE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The feature for tracing data was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for identifying an USB Channel will be set
        //
    case 8:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_CHANNEL_IDENTIFYING, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The procedure for channel identification was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
		break;

        // The feature for using an already configured speed will be set
        //
    case 10:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_BITRATE_ADAPTING, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The feature for bit rate adaptation was successfully %s", ARRAYOFCONST((bActivate ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow Status Frames" will be set
        //
    case 17:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_STATUS_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of Status frames was successfully %s", ARRAYOFCONST((bActivate ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow RTR Frames" will be set
        //
    case 18:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_RTR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of RTR frames was successfully %s", ARRAYOFCONST((bActivate ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
        break;

        // The option "Allow Error Frames" will be set
        //
    case 19:
        iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_ERROR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of Error frames was successfully %s", ARRAYOFCONST((bActivate ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
        break;

        // The option "Interframes Delay" will be set
        //
    case 20:
        iBuffer = StrToInt(txtDeviceIdOrDelay->Text);
        stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_INTERFRAME_DELAY, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
            IncludeTextMessage("The delay between transmitting frames was successfully set");
		break;

		// The option "Allow Echo Frames" will be set
		//
	case 21:
		iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
		stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ALLOW_ECHO_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
		if (stsResult == PCAN_ERROR_OK)
		{
			info = Format("The reception of Echo frames was successfully %s", ARRAYOFCONST((bActivate ? "enabled" : "disabled")));
			IncludeTextMessage(info);
		}
		break;

		// The option "Hard Reset Status" will be set
		//
	case 22:
		iBuffer = bActivate ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
		stsResult = m_objPCANBasic->SetValue(m_PcanHandle, PCAN_HARD_RESET_STATUS, (void*)&iBuffer, sizeof(iBuffer));
		if (stsResult == PCAN_ERROR_OK)
		{
			info = Format("The activation of a hard reset within the method PCANBasic.Reset was successfully %s", ARRAYOFCONST((bActivate ? "enabled" : "disabled")));
			IncludeTextMessage(info);
		}
		break;

        // The current parameter is invalid
        //
    default:
        stsResult = PCAN_ERROR_UNKNOWN;
        ::MessageBox(NULL, "Wrong parameter code.", "Error!",MB_ICONERROR);
        return;
    }

    // If the function fail, an error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!", MB_ICONERROR);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnParameterGetClick(TObject *Sender)
{
	TPCANStatus stsResult;
	unsigned int iBuffer;
	char strBuffer[256];
	AnsiString  info;

	// Sets a PCAN-Basic parameter value
	//
	switch (cbbParameter->ItemIndex)
	{
		// The device identifier of a channel will be get
		//
	case 0:
		stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_DEVICE_ID, (void*)&iBuffer, sizeof(iBuffer));
		if (stsResult == PCAN_ERROR_OK)
		{
			info = Format("The configured Device-ID is 0x%s", ARRAYOFCONST((IntToHex((__int64)iBuffer,0))));
			IncludeTextMessage(info);
		}
		break;

        // The 5 Volt Power feature of a channel will be get
        //
    case 1:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_5VOLTS_POWER, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
			info = Format("The 5-Volt Power of the USB/PC-Card is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for automatic reset on BUS-OFF will be get
        //
    case 2:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSOFF_AUTORESET, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The automatic-reset on BUS-OFF is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The CAN option "Listen Only" will be get
        //
    case 3:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_LISTEN_ONLY, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The CAN option \"Listen Only\" is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The feature for logging debug-information will be get
        //
    case 4:
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_LOG_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The feature for logging debug information is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "activated" : "deactivated")));
            IncludeTextMessage(info);
        }
        break;

        // The activation status of the channel option "Receive Status"  will be retrieved
		//
    case 5:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_RECEIVE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The channel option \"Receive Status\" is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF")));
            IncludeTextMessage(info);
        }
        break;

        // The Number of the CAN-Controller used by a PCAN-Channel
        //
    case 6:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CONTROLLER_NUMBER, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The CAN Controller number is %d", ARRAYOFCONST((iBuffer)));
            IncludeTextMessage(info);
        }
        break;

        // The activation status for the feature for tracing data will be retrieved
        //
    case 7:
		stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_TRACE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The feature for tracing data is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF")));
            IncludeTextMessage(info);
        }
        break;

        // The activation status of the Channel Identifying procedure will be retrieved
        //
    case 8:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_IDENTIFYING, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The identification procedure of the selected channel is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF")));
            IncludeTextMessage(info);
        }
        break;

        // The extra capabilities of a hardware will asked
        //
    case 9:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_FEATURES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The channel %s Flexible Data-Rate (CAN-FD)", ARRAYOFCONST(((iBuffer & FEATURE_FD_CAPABLE) ? "does support" : "DOESN'T SUPPORT")));
            IncludeTextMessage(info);
            info = Format("The channel %s an inter-frame delay for sending messages", ARRAYOFCONST(((iBuffer & FEATURE_DELAY_CAPABLE) ? "does support" : "DOESN'T SUPPORT")));
            IncludeTextMessage(info);
            info = Format("The channel %s using I/O pins", ARRAYOFCONST(((iBuffer & FEATURE_IO_CAPABLE) ? "does allow" : "DOESN'T ALLOW")));
            IncludeTextMessage(info);
        }
        break;

        // The status of the speed adapting feature will be retrieved
        //
    case 10:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_ADAPTING, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
			info = Format("The feature for bit rate adaptation is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "ON" : "OFF")));
            IncludeTextMessage(info);
        }
        break;

        // The bitrate of the connected channel will be retrieved (BTR0-BTR1 value)
        //
    case 11:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_INFO, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The bit rate of the channel is %.4Xh", ARRAYOFCONST(((iBuffer))));
            IncludeTextMessage(info);
        }
        break;

        // The bitrate of the connected FD channel will be retrieved (String value)
        //
    case 12:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BITRATE_INFO_FD, strBuffer, 256);
        if (stsResult == PCAN_ERROR_OK)
        {
            IncludeTextMessage("The bit rate FD of the channel is represented by the following values:");
			TStringList *strParts;
            strParts = new TStringList();
            strParts->Delimiter = ',';
            strParts->DelimitedText = strBuffer;
            for(int i=0 ; i < strParts->Count; i++)
                IncludeTextMessage(((AnsiString)"  * ") + strParts->Strings[i]);
            delete strParts;
        }
        break;

        // The nominal speed configured on the CAN bus
        //
    case 13:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSSPEED_NOMINAL, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The nominal speed of the channel is %d bit/s", ARRAYOFCONST((iBuffer)));
            IncludeTextMessage(info);
        }
        break;

        // The data speed configured on the CAN bus
        //
    case 14:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_BUSSPEED_DATA, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The data speed of the channel is %d bit/s", ARRAYOFCONST((iBuffer)));
            IncludeTextMessage(info);
        }
        break;

        // The IP address of a LAN channel as string, in IPv4 format
        //
    case 15:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_IP_ADDRESS, strBuffer, 256);
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The IP address of the channel is %s", ARRAYOFCONST(((AnsiString)strBuffer)));
            IncludeTextMessage(info);
        }
		break;

        // The running status of the LAN Service
        //
    case 16:
        stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_LAN_SERVICE_STATUS, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The LAN service is %s", ARRAYOFCONST(((iBuffer == SERVICE_STATUS_RUNNING) ? "running" : "NOT running")));
            IncludeTextMessage(info);
        }
        break;

        // The reception of Status frames
        //
    case 17:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_STATUS_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of Status frames is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
		break;

        // The reception of RTR frames
        //
    case 18:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_RTR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of RTR frames is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
        break;

        // The reception of Error frames
        //
    case 19:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_ERROR_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The reception of Error frames is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled")));
            IncludeTextMessage(info);
        }
        break;

        // The Interframe delay of an USB channel will be retrieved
        //
    case 20:
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_INTERFRAME_DELAY, (void*)&iBuffer, sizeof(iBuffer));
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("The configured interframe delay is %d ms", ARRAYOFCONST((iBuffer)));
            IncludeTextMessage(info);
        }
		break;

		// The reception of Echo frames
		//
	case 21:
		stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_ALLOW_ECHO_FRAMES, (void*)&iBuffer, sizeof(iBuffer));
		if (stsResult == PCAN_ERROR_OK)
		{
            info = Format("The reception of Echo frames is %s", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "enabled" : "disabled")));
			IncludeTextMessage(info);
        }
		break;

		// The activation of Hard Reset
		//
	case 22:
		stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_HARD_RESET_STATUS, (void*)&iBuffer, sizeof(iBuffer));
		if (stsResult == PCAN_ERROR_OK)
		{
            info = Format("he method PCANBasic.Reset is %s a hardware reset", ARRAYOFCONST(((iBuffer == PCAN_PARAMETER_ON) ? "performing" : "NOT performing")));
            IncludeTextMessage(info);
        }
		break;

        // The current parameter is invalid
        //
    default:
        ::MessageBox(NULL, "Wrong parameter code.", "Error!",MB_ICONERROR);
        return;
    }

    // If the function fail, an error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnReadClick(TObject *Sender)
{
    TPCANStatus stsResult;

	// We execute the "Read" function of the PCANBasic
	//
	stsResult = m_IsFD ? ReadMessageFD() : ReadMessage();
    if (stsResult != PCAN_ERROR_OK)
		// If an error occurred, an information message is included
		//
		IncludeTextMessage(GetFormatedError(stsResult));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnMsgClearClick(TObject *Sender)
{
	// (Protected environment)
    //
    {
		clsCritical locker(m_objpCS);

        // Remove all messages
		//
        lstMessages->Items->Clear();
        while(m_LastMsgsList->Count)
		{
            // Explicit cast to correct pointer type before deletion
			delete static_cast<MessageStatus*>(m_LastMsgsList->First());
			//delete m_LastMsgsList->First();
			m_LastMsgsList->Delete(0);
        }
	}
}
//---------------------------------------------------------------------------


/*
TPCANStatus TForm1::WriteFrame()
{
	TPCANMsg CANMsg;
	TEdit *CurrentTextBox;

	// We configurate the Message.  The ID (max 0x1FF),
	// Length of the Data, Message Type (Standard in
	// this example) and die data
	//
	CANMsg = TPCANMsg();
	CANMsg.ID = StrToInt("0x"+txtID->Text);
	CANMsg.LEN = (BYTE)nudLength->Position;
	CANMsg.MSGTYPE = (chbExtended->Checked) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
	// If a remote frame will be sent, the data bytes are not important.
	//
	if (chbRemote->Checked)
		CANMsg.MSGTYPE |= PCAN_MESSAGE_RTR;
	else
	{
		// We get so much data as the Len of the message
		//
		for (int i = 0; i < GetLengthFromDLC(CANMsg.LEN, true); i++)
		{
			CurrentTextBox = (TEdit*)FindComponent("txtData"+IntToStr(i));
			CANMsg.DATA[i] = (BYTE)(StrToInt("0x" + CurrentTextBox->Text));
		}
	}

	// The message is sent to the configured hardware
	//
	return m_objPCANBasic->Write(m_PcanHandle, &CANMsg);
}
//---------------------------------------------------------------------------

TPCANStatus TForm1::WriteFrameFD()
{
	TPCANMsgFD CANMsg;
	TEdit *CurrentTextBox;
	int iLength;

	// We configurate the Message.  The ID (max 0x1FF),
    // Length of the Data, Message Type (Standard in
	// this example) and die data
    //
	CANMsg = TPCANMsgFD();
	CANMsg.ID = StrToInt("0x"+txtID->Text);
	CANMsg.DLC = (BYTE)nudLength->Position;
    CANMsg.MSGTYPE = (chbExtended->Checked) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
    CANMsg.MSGTYPE |= (chbFD->Checked) ?  PCAN_MESSAGE_FD : PCAN_MESSAGE_STANDARD;
	CANMsg.MSGTYPE |= (chbBRS->Checked) ?  PCAN_MESSAGE_BRS : PCAN_MESSAGE_STANDARD;

	// If a remote frame will be sent, the data bytes are not important.
	//
	if (chbRemote->Checked)
		CANMsg.MSGTYPE |= PCAN_MESSAGE_RTR;
	else
	{
		// We get so much data as the Len of the message
		//
        iLength = GetLengthFromDLC(CANMsg.DLC, !(CANMsg.MSGTYPE & PCAN_MESSAGE_FD));
        for (int i = 0; i < iLength; i++)
		{
			CurrentTextBox = (TEdit*)FindComponent("txtData"+IntToStr(i));
			CANMsg.DATA[i] = (BYTE)(StrToInt("0x" + CurrentTextBox->Text));
		}
	}

	// The message is sent to the configured hardware
	//
	return m_objPCANBasic->WriteFD(m_PcanHandle, &CANMsg);
}
//---------------------------------------------------------------------------
*/
TPCANStatus TForm1::WriteState()
{
	TPCANMsg CANMsg;
		CANFRM_0x405_VCU_KEEP_ALIVE keepAlive;

	CANFRM_0x400_VCU_COMMAND command;

	CANFRM_0x404_VCU_MODULE_COMMAND moduleCommand;


	TEdit *CurrentTextBox;
	char status;
	uint16_t hv_voltage;



	hv_voltage = editInverter->Text.ToInt() / VCU_HV_FACTOR;

	if (chkDMC->Checked	 == true){

		// Direct Module Control
		CANMsg = TPCANMsg();
		CANMsg.ID = ID_VCU_MODULE_COMMAND + (packID * 0x100);
		CANMsg.LEN = 8;
		CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

		moduleCommand.module_id					= cboModuleId->Text.ToInt();
		moduleCommand.module_contactor_ctrl		= editSelectedState->TextHint.ToInt();
		moduleCommand.module_cell_balance_ctrl	= 0;
		moduleCommand.module_hv_bus_actv_iso 	= 0;
		moduleCommand.vcu_hv_bus_voltage 		= hv_voltage;
		moduleCommand.UNUSED_06_15              = 0;
		moduleCommand.UNUSED_32_63              = 0;

		memcpy(CANMsg.DATA, &moduleCommand, sizeof(moduleCommand));


		// The message is sent to the configured hardware
		return m_objPCANBasic->Write(m_PcanHandle, &CANMsg);


	} else {


		// We configurate the Message.  The ID (max 0x1FF),
		// Length of the Data, Message Type (Standard in
		// this example) and die data
		//
		CANMsg = TPCANMsg();
		CANMsg.ID = ID_VCU_COMMAND + (packID * 0x100);
		CANMsg.LEN = 8;
		CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;



		command.vcu_contactor_ctrl = editSelectedState->TextHint.ToInt();
		command.vcu_cell_balance_ctrl 	= 0;
		command.vcu_hv_bus_actv_iso_en 	= 0;
		command.UNUSED_06_15 			= 0;
		command.vcu_hv_bus_voltage 		= hv_voltage;
		command.UNUSED_32_63            = 0;

		memcpy(CANMsg.DATA, &command, sizeof(command));

		//CANMsg.DATA[0]= cboState->ItemIndex;


		// The message is sent to the configured hardware
		//
		return m_objPCANBasic->Write(m_PcanHandle, &CANMsg);
	}

}

/*
void __fastcall TForm1::btnSendClick(TObject *Sender)
{
	TPCANStatus stsResult;

	// Send the message
	//
	stsResult = m_IsFD ? WriteFrameFD() : WriteFrame();

	// The Hardware was successfully sent
	//
	if (stsResult == PCAN_ERROR_OK)
		IncludeTextMessage("Message was successfully SENT");
	else
		// An error occurred.  We show the error.
		//
		::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
}
*/
//---------------------------------------------------------------------------

void __fastcall TForm1::btnGetVersionsClick(TObject *Sender)
{
    TPCANStatus stsResult;
    char buffer[256];

    AnsiString info;
    int iPos;

    memset(buffer,'\0',256);

    // We get the vesion of the PCAN-Basic API
    //
    stsResult = m_objPCANBasic->GetValue(PCAN_NONEBUS, PCAN_API_VERSION, buffer, 256);
    if (stsResult == PCAN_ERROR_OK)
    {
        info = Format("API Version: %s", ARRAYOFCONST((buffer)));
        IncludeTextMessage(info);

		// We get the version of the firmware on the device
        //
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_FIRMWARE_VERSION, buffer, 256);
        if (stsResult == PCAN_ERROR_OK)
        {
            info = Format("Firmware Version: %s", ARRAYOFCONST((buffer)));
            IncludeTextMessage(info);
        }
        // We get the driver version of the channel being used
        //
        stsResult = m_objPCANBasic->GetValue(m_PcanHandle, PCAN_CHANNEL_VERSION, buffer, 256);

        if (stsResult == PCAN_ERROR_OK)
        {
            IncludeTextMessage("Channel/Driver Version: ");

            // Because this information contains line control characters (several lines)
            // we split this also in several entries in the Information List-Box
            //
            info = (AnsiString)buffer;
			while(info != "")
            {
                iPos = info.Pos("\n");
                if (iPos == 0)
                    iPos = info.Length();
                IncludeTextMessage("     * " + info.SubString(1, iPos));
                info.Delete(1,iPos);
            }
        }
    }

    // If the function fail, an error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnInfoClearClick(TObject *Sender)
{
    lbxInfo->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnStatusClick(TObject *Sender)
{
    TPCANStatus status;
    AnsiString errorName;
    AnsiString info;

    // Gets the current BUS status of a PCAN Channel.
    //
    status = m_objPCANBasic->GetStatus(m_PcanHandle);

    // Switch On Error Name
    //
    switch(status)
    {
        case PCAN_ERROR_INITIALIZE:
            errorName = "PCAN_ERROR_INITIALIZE";
            break;

        case PCAN_ERROR_BUSLIGHT:
			errorName = "PCAN_ERROR_BUSLIGHT";
            break;

        case PCAN_ERROR_BUSHEAVY: // PCAN_ERROR_BUSWARNING
            errorName = m_IsFD ? "PCAN_ERROR_BUSWARNING" : "PCAN_ERROR_BUSHEAVY";
            break;

        case PCAN_ERROR_BUSPASSIVE:
            errorName = "PCAN_ERROR_BUSPASSIVE";
            break;

        case PCAN_ERROR_BUSOFF:
            errorName = "PCAN_ERROR_BUSOFF";
            break;

        case PCAN_ERROR_OK:
            errorName = "PCAN_ERROR_OK";
            break;

        default:
            errorName = "See Documentation";
			break;
    }

    // Display Message
    //
    info = Format("Status: %s (%Xh)", ARRAYOFCONST((errorName, status)));
    IncludeTextMessage(info);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnResetClick(TObject *Sender)
{
    TPCANStatus stsResult;

    // Resets the receive and transmit queues of a PCAN Channel.
    //
    stsResult = m_objPCANBasic->Reset(m_PcanHandle);

    // If it fails, a error message is shown
    //
    if (stsResult != PCAN_ERROR_OK)
        ::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
    else
        IncludeTextMessage("Receive and transmit queues successfully reset");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbbChannelChange(TObject *Sender)
{
    bool bNonPnP;
    AnsiString strTemp;

    // Get the handle from the text being shown
    //
    strTemp = cbbChannel->Text;
    strTemp = strTemp.SubString(strTemp.Pos("(") + 1, 3);

    strTemp = StringReplace(strTemp, "h", " ", TReplaceFlags() << rfReplaceAll).Trim();

    // Determines if the handle belong to a No Plug&Play hardware
    //
    m_PcanHandle = (TPCANHandle)StrToInt("0x"+strTemp);
	bNonPnP = m_PcanHandle <= PCAN_DNGBUS1;

    // Activates/deactivates configuration controls according with the
    // kind of hardware
    //
    cbbIO->Enabled = bNonPnP;
    cbbInterrupt->Enabled = bNonPnP;
    cbbHwType->Enabled = bNonPnP;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbbBaudratesChange(TObject *Sender)
{
    // We save the corresponding Baudrate enumeration
    // type value for every selected Baudrate from the
    // list.
    //
    switch(cbbBaudrates->ItemIndex)
    {
    case 0:
        m_Baudrate = PCAN_BAUD_1M;
        break;
	case 1:
        m_Baudrate = PCAN_BAUD_800K;
        break;
    case 2:
        m_Baudrate = PCAN_BAUD_500K;
        break;
    case 3:
        m_Baudrate = PCAN_BAUD_250K;
        break;
    case 4:
        m_Baudrate = PCAN_BAUD_125K;
        break;
    case 5:
        m_Baudrate = PCAN_BAUD_100K;
        break;
    case 6:
        m_Baudrate = PCAN_BAUD_95K;
        break;
    case 7:
        m_Baudrate = PCAN_BAUD_83K;
        break;
    case 8:
        m_Baudrate = PCAN_BAUD_50K;
        break;
    case 9:
        m_Baudrate = PCAN_BAUD_47K;
        break;
    case 10:
        m_Baudrate = PCAN_BAUD_33K;
        break;
    case 11:
        m_Baudrate = PCAN_BAUD_20K;
        break;
    case 12:
        m_Baudrate = PCAN_BAUD_10K;
        break;
    case 13:
        m_Baudrate = PCAN_BAUD_5K;
        break;
    default:
        m_Baudrate = (TPCANBaudrate)0;
		break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbbHwTypeChange(TObject *Sender)
{
    // Saves the current type for a no-Plug&Play hardware
    //
    switch (cbbHwType->ItemIndex)
    {
    case 0:
        m_HwType = PCAN_TYPE_ISA;
        break;
    case 1:
        m_HwType = PCAN_TYPE_ISA_SJA;
        break;
    case 2:
        m_HwType = PCAN_TYPE_ISA_PHYTEC;
        break;
    case 3:
        m_HwType = PCAN_TYPE_DNG;
        break;
	case 4:
        m_HwType = PCAN_TYPE_DNG_EPP;
        break;
    case 5:
        m_HwType = PCAN_TYPE_DNG_SJA;
        break;
    case 6:
        m_HwType = PCAN_TYPE_DNG_SJA_EPP;
        break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbbParameterChange(TObject *Sender)
{
	// Activates/deactivates controls according with the selected
    // PCAN-Basic parameter
    //
    rdbParamActive->Enabled = (cbbParameter->ItemIndex != 0) && (cbbParameter->ItemIndex != 20);
    rdbParamInactive->Enabled = rdbParamActive->Enabled;
	txtDeviceIdOrDelay->Enabled = (!rdbParamActive->Enabled);
	laDeviceOrDelay->Caption = (cbbParameter->ItemIndex == 20) ? "Delay (ms):" : "Device ID (Hex):";
	txtDeviceIdOrDelay->Text = "0";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::chbFilterExtClick(TObject *Sender)
{
    // Updates the from and To ID-text according with the Message type
    //
	//txtIdFromExit(txtIdFrom);
	//txtIdFromExit(txtIdTo);
}
//---------------------------------------------------------------------------
/*
void __fastcall TForm1::chbExtendedClick(TObject *Sender)
{
	// Updates the ID-text according with the Message type
	//
	txtIDExit(txtID);
}
*/
//---------------------------------------------------------------------------

void __fastcall TForm1::rdbTimerClick(TObject *Sender)
{
    TRadioButton *radioBtn;

    radioBtn =  (TRadioButton*)Sender;
	if (radioBtn && !radioBtn->Checked)
		return;

	if (radioBtn->Name == "rdbTimer")
		m_ActiveReadingMode = 0;
	if (radioBtn->Name == "rdbEvent")
		m_ActiveReadingMode = 1;
	if (radioBtn->Name == "rdbManual")
		m_ActiveReadingMode = 2;

	ReadingModeChanged();
}
//---------------------------------------------------------------------------
/*
void TForm1::EnableDisableDataFields(int length)
{
	TEdit *CurrentTextBox;

	CurrentTextBox = txtData0;
	for (int i = 0; i <= 8; i++)
	{
		CurrentTextBox->Enabled = i <= length;
		if (i < 8)
			CurrentTextBox = (TEdit*)FindComponent("txtData"+IntToStr(i));
	}
}


//---------------------------------------------------------------------------

void __fastcall TForm1::nudLengthClick(TObject *Sender, TUDBtnType Button)
{
	int iLength;

	iLength = GetLengthFromDLC((int)nudLength->Position, !chbFD->Checked);
	EnableDisableDataFields(iLength);
	laLength->Caption = Format("%d B.", ARRAYOFCONST((iLength)));
}

*/
//---------------------------------------------------------------------------

void __fastcall TForm1::txtIdFromExit(TObject *Sender)
{
	unsigned int iTempMax, iTempValue;
	TEdit *IdBox;

	IdBox = (TEdit*)Sender;

	// Calculates the text length and Maximum ID value according
	// with the Frame Type
	//
   // IdBox->MaxLength = (chbFilterExt->Checked) ? 8 : 3;
	//iTempMax = (chbFilterExt->Checked) ? 0x1FFFFFFF : 0x7FF;

	// Adjusts the ID value to refresh
	//
	iTempValue = StrToInt("0x" + IdBox->Text);
	if (iTempValue < iTempMax)
		iTempMax = iTempValue;

	// Refreshes the ID value
	//
	IdBox->Text = IntToHex((int)iTempMax,IdBox->MaxLength);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::txtIdFromKeyPress(TObject *Sender, char &Key)
{
	// We convert the Character to its Upper case equivalent
	//
	Key = *(UpperCase((AnsiString)Key)).c_str();

	// The Key is the Delete (Backspace) Key
	//
	if (Key == 8)
		return;
	// The Key is a number between 0-9
	//
	if ((Key > 47)&&(Key < 58))
		return;
	// The Key is a character between A-F
	//
	if ((Key > 64)&&(Key < 71))
		return;

	// Is neither a number nor a character between A(a) and F(f)
	//
	Key = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::txtDeviceIdOrDelayKeyPress(TObject *Sender, char &Key)
{
	// We convert the Character to its Upper case equivalent
	//
	Key = *(UpperCase((AnsiString)Key)).c_str();

	// The Key is the Delete (Backspace) Key
	//
	if (Key == 8)
		return;
	// The Key is a number between 0-9
	//
	if ((Key > 47)&&(Key < 58))
		return;

	// Device ID is engtered as Hex
	if(cbbParameter->ItemIndex == 0)
		// The Key is a character between A-F
		//
		if ((Key > 64)&&(Key < 71))
			return;

	// Is neither a number nor a character between A(a) and F(f)
    //
    Key = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::txtDeviceIdOrDelayExit(TObject *Sender)
{
	__int64 iTemp;

	// Checks the range of the given device ID (Hex)
	//
	if (cbbParameter->ItemIndex == 0)
		iTemp =  StrToInt64("0x" + txtDeviceIdOrDelay->Text);
	else
		// Checks the range of the given delay (Dec)
		//
		iTemp =  StrToInt64(txtDeviceIdOrDelay->Text);

	if (iTemp > MAXUINT)
		txtDeviceIdOrDelay->Text = (cbbParameter->ItemIndex == 0) ? IntToHex((__int64)MAXUINT,8) : FloatToStr(MAXUINT);
}
//---------------------------------------------------------------------------
/*
void __fastcall TForm1::txtIDExit(TObject *Sender)
{
	unsigned int iTempMax, iTempValue;

	// Calculates the text length and Maximum ID value according
	// with the Frame Type
	//
	txtID->MaxLength = (chbExtended->Checked) ? 8 : 3;
	iTempMax = (chbExtended->Checked) ? 0x1FFFFFFF : 0x7FF;

	// Adjusts the ID value to refresh
	//
	iTempValue = StrToInt("0x" + txtID->Text);
	if (iTempValue < iTempMax)
		iTempMax = iTempValue;

	// Refreshes the ID value
	//
	txtID->Text = IntToHex((int)iTempMax,txtID->MaxLength);
}
*/
//---------------------------------------------------------------------------

void __fastcall TForm1::txtData0Exit(TObject *Sender)
{
    TEdit *CurrentEdit;

    // Checks the length of the given value
    //
    if (String(Sender->ClassName()) == "TEdit")
    {
        CurrentEdit = (TEdit*)Sender;
        while(CurrentEdit->Text.Length() != 2)
            CurrentEdit->Text = ("0" + CurrentEdit->Text);
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::chbShowPeriodClick(TObject *Sender)
{
    MessageStatus *msg;

    // According with the check-value of this checkbox,
    // the recieved time of a messages will be interpreted as
    // period (time between the two last messages) or as time-stamp
    // (the elapsed time since windows was started).
    // - (Protected environment)
    //
	if(locked==0){
		//clsCritical locker(m_objpCS);
		locked = 1;
		for(int i=0; i < m_LastMsgsList->Count; i++)
		{
			msg = (MessageStatus*)m_LastMsgsList->Items[i];
			msg->ShowingPeriod =  chbShowPeriod->Checked;
		}
		locked = 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::tmrDisplayTimer(TObject *Sender)
{
    DisplayMessages();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::chbCanFDClick(TObject *Sender)
{
    m_IsFD = chbCanFD->Checked;

    cbbBaudrates->Visible = !m_IsFD;
    cbbHwType->Visible = !m_IsFD;
    cbbInterrupt->Visible = !m_IsFD;
    cbbIO->Visible = !m_IsFD;
    laBaudrate->Visible = !m_IsFD;
	laHwType->Visible = !m_IsFD;
	laIOPort->Visible = !m_IsFD;
	laInterrupt->Visible = !m_IsFD;

	txtBitrate->Visible = m_IsFD;
	laBitrate->Visible = m_IsFD;
	//chbFD->Visible = m_IsFD;
	//chbBRS->Visible = m_IsFD;
   /*
	if ((nudLength->Max > 8) && !m_IsFD)
		chbFD->Checked = false;
*/
}
//---------------------------------------------------------------------------
 /*
void __fastcall TForm1::chbFDClick(TObject *Sender)
{
	int iMax;

	chbRemote->Enabled = !chbFD->Checked;
	chbBRS->Enabled = chbFD->Checked;
	if (!chbBRS->Enabled)
		chbBRS->Checked = false;
	iMax = chbFD->Checked ? 15 : 8;
	nudLength->Max = iMax;
	txtLength->Text = IntToStr(nudLength->Position);
	EnableDisableDataFields(GetLengthFromDLC(nudLength->Position, chbFD->Checked));
}

//---------------------------------------------------------------------------

void __fastcall TForm1::chbRemoteClick(TObject *Sender)
{
	// Show or Hide control according message is a RTR
	//
	EnableDisableDataFields(chbRemote->Checked ? 0 : nudLength->Position);
	chbFD->Enabled = !chbRemote->Checked;
}
*/
//---------------------------------------------------------------------------

/***************************************************************************************************************
*     P r o c e s s S t a t e
***************************************************************************************************************/
void TForm1::ProcessState(TPCANMsgFD theMsg){

  CANFRM_0x410_BMS_STATE state;

  float soh = 0;
  AnsiString sState  ="";
  AnsiString sStatus ="";

  // copy received data to status structure
  memset(&state,0,sizeof(state));
  memcpy(&state, theMsg.DATA, sizeof(state));

  switch (state.bms_state){
	case 0:
		sState = "Off";
		break;
	case 1:
		sState = "Standby";
		break;
	case 2:
		sState = "Precharge";
		break;
	case 3:
		sState = "On";
		break;
	default:
		break;

  }

	switch (state.bms_status){
	case 0:
		sStatus = "Off";
		break;
	case 1:
		sStatus = "Empty";
		break;
	case 2:
		sStatus = "Normal";
		break;
	case 3:
		sStatus = "Full";
		break;
	default:
		break;

  }
  soh = VCU_SOH_PERCENTAGE_BASE + (state.bms_soh * VCU_SOH_PERCENTAGE_FACTOR);

  editSoh->Text 			= FloatToStrF(soh,ffFixed,5,2)  + "%";
  editState->Text 			= sState;
  editStatus->Text 			= sStatus;
  editFault->Text  			= state.bms_module_off;
  editTotalModules->Text  	= state.bms_total_mod_cnt;
  editActiveModules->Text 	= state.bms_active_mod_cnt;
}


/***************************************************************************************************************
*     P r o c e s s D a t a 1
***************************************************************************************************************/
void TForm1::ProcessData1(TPCANMsgFD theMsg){

  CANFRM_0x421_BMS_DATA_1 data;

  float voltage = 0;
  float current = 0;


  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  voltage = data.bms_pack_voltage * VCU_VOLTAGE_FACTOR;
  current = VCU_CURRENT_BASE + (data.bms_pack_current * VCU_CURRENT_FACTOR);

  editVoltage->Text = FloatToStrF(voltage,ffFixed,5,2) + "V";
  editCurrent->Text = FloatToStrF(current,ffFixed,5,2) + "A";

}

/***************************************************************************************************************
*     P r o c e s s D a t a 2
***************************************************************************************************************/
void TForm1::ProcessData2(TPCANMsgFD theMsg){

  CANFRM_0x422_BMS_DATA_2 data;

  float soc         = 0;
  float hiCellVolt  = 0;
  float loCellVolt  = 0;
  float avgCellVolt = 0;


  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  hiCellVolt  = data.bms_high_cell_volt * VCU_CELL_VOLTAGE_FACTOR;
  loCellVolt  = data.bms_low_cell_volt  * VCU_CELL_VOLTAGE_FACTOR;
  avgCellVolt = data.bms_avg_cell_volt  * VCU_CELL_VOLTAGE_FACTOR;
  soc         = data.bms_soc            * VCU_SOC_PERCENTAGE_FACTOR;

  editSoc->Text 		= FloatToStrF(soc,ffFixed,5,2)         + "%";
  editHiCellVolt->Text 	= FloatToStrF(hiCellVolt,ffFixed,5,2)  + "V";
  editLoCellVolt->Text 	= FloatToStrF(loCellVolt,ffFixed,5,2)  + "V";
  editAvgCellVolt->Text = FloatToStrF(avgCellVolt,ffFixed,5,2) + "V";

}

/***************************************************************************************************************
*     P r o c e s s D a t a 3
***************************************************************************************************************/
void TForm1::ProcessData3(TPCANMsgFD theMsg){

  CANFRM_0x423_BMS_DATA_3 data;

  float hiCellTemp  = 0;
  float loCellTemp  = 0;
  float avgCellTemp = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  hiCellTemp  = VCU_TEMPERATURE_BASE + (data.bms_high_cell_temp * VCU_TEMPERATURE_FACTOR);
  loCellTemp  = VCU_TEMPERATURE_BASE + (data.bms_low_cell_temp  * VCU_TEMPERATURE_FACTOR);
  avgCellTemp = VCU_TEMPERATURE_BASE + (data.bms_avg_cell_temp  * VCU_TEMPERATURE_FACTOR);

  editHiCellTemp->Text 	= FloatToStrF(hiCellTemp,ffFixed,5,2)  + "C";
  editLoCellTemp->Text 	= FloatToStrF(loCellTemp,ffFixed,5,2)  + "C";;
  editAvgCellTemp->Text = FloatToStrF(avgCellTemp,ffFixed,5,2) + "C";;


}


/***************************************************************************************************************
*     P r o c e s s D a t a 5
***************************************************************************************************************/
void TForm1::ProcessData5(TPCANMsgFD theMsg){

  CANFRM_0x425_BMS_DATA_5 data;

  float chgLimit     = 0;
  float dischgLimit  = 0;
  float endVoltage   = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  chgLimit      = VCU_CURRENT_BASE + (data.bms_charge_limit   * VCU_CURRENT_FACTOR);
  dischgLimit   = VCU_CURRENT_BASE + (data.bms_dischage_limit * VCU_CURRENT_FACTOR);
  endVoltage    = data.bms_charge_end_voltage_limit * VCU_VOLTAGE_FACTOR;

  editChgLimit->Text 		= FloatToStrF(chgLimit,ffFixed,5,2)    + "A";
  editDisChgLimit->Text 	= FloatToStrF(dischgLimit,ffFixed,5,2) + "A";
  editEndVoltLimit->Text 	= FloatToStrF(endVoltage,ffFixed,5,2)  + "V";

}


/***************************************************************************************************************
*    P r o c e s s D a t a 8
***************************************************************************************************************/
void TForm1::ProcessData8(TPCANMsgFD theMsg){

  CANFRM_0x428_BMS_DATA_8 data;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  /*
  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_8  %03x : HIVM=%d LOVM=%d HIVC=%d LOVC=%d",rxObj.bF.id.SID, data.bms_max_volt_mod, data.bms_min_volt_mod, data.bms_max_volt_cell, data.bms_min_volt_cell) ; serialOut(tempBuffer);}
  */

}


/***************************************************************************************************************
*     P r o c e s s D a t a 9
***************************************************************************************************************/
void TForm1::ProcessData9(TPCANMsgFD theMsg){

  CANFRM_0x429_BMS_DATA_9 data;


	  // copy received data to status structure
	  memset(&data,0,sizeof(data));
	  memcpy(&data, theMsg.DATA, sizeof(data));
	  /*
	  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_9  %03x : HITM=%d LOTM=%d HITC=%d LOTC=%d",rxObj.bF.id.SID,data.bms_max_temp_mod, data.bms_min_temp_mod, data.bms_max_temp_cell, data.bms_min_temp_cell) ; serialOut(tempBuffer);}
	  */

}


/***************************************************************************************************************
*     P r o c e s s D a t a 1 0
***************************************************************************************************************/
void TForm1::ProcessData10(TPCANMsgFD theMsg){

  CANFRM_0x430_BMS_DATA_10 data;

  float isolation     = 0;
   {
	clsCritical locker(m_objpCS);

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, theMsg.DATA, sizeof(data));

  isolation = data.bms_hv_bus_actv_iso * VCU_ISOLATION_FACTOR;
  //isolation = data.bms_hv_bus_actv_iso * VCU_ISOLATION_FACTOR;
  editIsolation->Text = FloatToStrF(isolation,ffFixed,2,2) + "Ohm/V";
  /*
  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_10 %03x : ISOL=%.2fOhms/V",rxObj.bF.id.SID,isolation) ; serialOut(tempBuffer);}
  */
   }
}

/***************************************************************************************************************
*     P r o c e s s T i m e R e q u e s t
***************************************************************************************************************/
void TForm1::ProcessTimeRequest(void){

	CANFRM_0x401_VCU_TIME vcuTime;

	TPCANMsg CANMsg;
	TEdit *CurrentTextBox;

	memset(&vcuTime,0,sizeof(vcuTime));

	time_t now = time(0);
	vcuTime.vcu_time = now;

	// We configurate the Message.  The ID (max 0x1FF),
	// Length of the Data, Message Type (Standard in
	// this example) and die data
	//
	CANMsg = TPCANMsg();
	CANMsg.ID = ID_VCU_TIME + (packID * 0x100);
	CANMsg.LEN = 8;
	CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;
	memcpy(CANMsg.DATA, &vcuTime, sizeof(vcuTime));

	// The message is sent to the configured hardware
	m_objPCANBasic->Write(m_PcanHandle, &CANMsg);

}



void __fastcall TForm1::tmrStateTimer(TObject *Sender)
{
	TPCANStatus stsResult;
	TPCANMsg CANMsg;
	CANFRM_0x405_VCU_KEEP_ALIVE keepAlive;


	if (chkSendState->Checked == true) {
		// We send State (else we send keep alives)
		stsResult = WriteState();

	} else {

		// If the send state checkbox is unchecked we send keep alives
		CANMsg = TPCANMsg();
		CANMsg.ID = ID_VCU_KEEP_ALIVE + (packID * 0x100);
		CANMsg.LEN = 8;
		CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

		if(chkDMC->Checked == true){
			keepAlive.module_id = cboModuleId->Text.ToInt();
		} else {
			keepAlive.module_id = 0;
		}

		keepAlive.UNUSED_08_31 = 0;
		keepAlive.UNUSED_32_63 = 0;

		memcpy(CANMsg.DATA, &keepAlive, sizeof(keepAlive));

		// The message is sent to the configured hardware
		m_objPCANBasic->Write(m_PcanHandle, &CANMsg);

	}



	// Send the message
	//


	// The Hardware was successfully sent
	//
   /*
	if (stsResult == PCAN_ERROR_OK)
		IncludeTextMessage("Message was successfully SENT");
	else
		// An error occurred.  We show the error.
		//
		::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
   */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::chkSendStateClick(TObject *Sender)
{
	//tmrState->Enabled = chkSendState->Checked;

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

void __fastcall TForm1::btnPackParamSetClick(TObject *Sender)
{


	TPCANMsg CANMsg;
	CANFRM_0x403_VCU_WRITE_EEPROM eeprom;

    clearData();

	// We configurate the Message.  The ID (max 0x1FF),
	// Length of the Data, Message Type (Standard in
	// this example) and die data
	//
	CANMsg			= TPCANMsg();
	CANMsg.ID 		= ID_VCU_WRITE_EEPROM + (packID * 0x100);
	CANMsg.LEN 		= 8;
	CANMsg.MSGTYPE 	= PCAN_MESSAGE_STANDARD;

	eeprom.bms_eeprom_data_register = cboPackParam->ItemIndex +3;
	eeprom.UNUSED_8_31 				= 0 ;
	eeprom.bms_eeprom_data 			= edtPackValue->Text.ToInt();

	memcpy(CANMsg.DATA, &eeprom, sizeof(eeprom));

	// The message is sent to the configured hardware
	//
	m_objPCANBasic->Write(m_PcanHandle, &CANMsg);

}
//---------------------------------------------------------------------------




void __fastcall TForm1::cboPackIDChange(TObject *Sender)
{
	char caption[100];
	packID = cboPackID->Text.ToInt();
	sprintf(caption, "(Address base 0x%03x)", 0x400 + (packID * 0x100));
	lblCANbase->Caption = caption;
	sprintf(caption, "0x%03x:", 0x410 + (packID * 0x100));
	lbl0x410->Caption = caption;
	sprintf(caption, "0x%03x:", 0x421 + (packID * 0x100));
	lbl0x421->Caption = caption;
	sprintf(caption, "0x%03x:", 0x422 + (packID * 0x100));
	lbl0x422->Caption = caption;
	sprintf(caption, "0x%03x:", 0x423 + (packID * 0x100));
	lbl0x423->Caption = caption;
	sprintf(caption, "0x%03x:", 0x425 + (packID * 0x100));
	lbl0x425->Caption = caption;
	sprintf(caption, "0x%03x:", 0x430 + (packID * 0x100));
	lbl0x430->Caption = caption;
	clearData();
}
//---------------------------------------------------------------------------
void TForm1::clearData(void){
	editState->Text = "0";
	editStatus->Text = "0";
	editSoh->Text = "0";
	editTotalModules->Text = "0";
	editFault->Text = "0";
	editActiveModules->Text = "0";
	editVoltage->Text = "0";
	editCurrent->Text = "0";
	editHiCellVolt->Text = "0";
	editLoCellVolt->Text = "0";
	editAvgCellVolt->Text = "0";
	editSoc->Text = "0";
	editHiCellTemp->Text = "0";
	editLoCellTemp->Text = "0";
	editAvgCellTemp->Text = "0";
	editChgLimit->Text = "0";
	editDisChgLimit->Text = "0";
	editEndVoltLimit->Text = "0";
	editIsolation->Text = "0";
}

void __fastcall TForm1::chkDMCClick(TObject *Sender)
{
	if (chkDMC->Checked	 == true){
		// shut off the pack


		// Set up the form
		cboModuleId->Enabled = true;
		grpState->Caption = "Module State Control";
		grpPackData->Visible = false;
		grpModuleData->Visible = true;
		UpdateModuleDisplay();
		WriteState();

	} else {
		cboModuleId->Enabled = false;
		grpState->Caption = "Pack State Control";
		grpPackData->Visible = true;
		grpModuleData->Visible = false;
		WriteState();
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnRefreshClick(TObject *Sender)
{
	// request active module list from pack controller
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------



void __fastcall TForm1::cboModuleIdChange(TObject *Sender)
{
	UpdateModuleDisplay();
}

/***************************************************************************************************************
*     U p d a t e M o d u l e D i s p l a y
***************************************************************************************************************/
void TForm1::UpdateModuleDisplay(void)
{

  uint8_t index = cboModuleId->Text.ToInt();

  float soh = 0;
  float voltage = 0;
  float current = 0;
  float soc         = 0;
  float hiCellVolt  = 0;
  float loCellVolt  = 0;
  float avgCellVolt = 0;
  float hiCellTemp  = 0;
  float loCellTemp  = 0;
  float avgCellTemp = 0;
  float chgLimit     = 0;
  float dischgLimit  = 0;
  float endVoltage   = 0;
  AnsiString sState  ="";
  AnsiString sStatus ="";


  switch (module[index].currentState){
	case 0:
		sState = "Off";
		break;
	case 1:
		sState = "Standby";
		break;
	case 2:
		sState = "Precharge";
		break;
	case 3:
		sState = "On";
		break;
	default:
		break;

  }

  switch (module[index].status){
	case 0:
		sStatus = "Off";
		break;
	case 1:
		sStatus = "Empty";
		break;
	case 2:
		sStatus = "Normal";
		break;
	case 3:
		sStatus = "Full";
		break;
	default:
		break;

  }

  soh 		  = MODULE_PERCENTAGE_BASE + (module[index].soh * MODULE_PERCENTAGE_FACTOR);
  soc 		  = MODULE_PERCENTAGE_BASE + (module[index].soc * MODULE_PERCENTAGE_FACTOR);
  voltage 	  = MODULE_VOLTAGE_BASE    + (module[index].mmv * MODULE_VOLTAGE_FACTOR);
  current 	  = MODULE_CURRENT_BASE    + (module[index].mmc * MODULE_CURRENT_FACTOR);

  hiCellVolt  = MODULE_VOLTAGE_BASE + (module[index].cellHiVolt  * MODULE_CELL_VOLTAGE_FACTOR);
  loCellVolt  = MODULE_VOLTAGE_BASE + (module[index].cellLoVolt  * MODULE_CELL_VOLTAGE_FACTOR);
  avgCellVolt = MODULE_VOLTAGE_BASE + (module[index].cellAvgVolt * MODULE_CELL_VOLTAGE_FACTOR);


  hiCellTemp  = MODULE_TEMPERATURE_BASE + (module[index].cellHiTemp  * MODULE_TEMPERATURE_FACTOR);
  loCellTemp  = MODULE_TEMPERATURE_BASE + (module[index].cellLoTemp  * MODULE_TEMPERATURE_FACTOR);
  avgCellTemp = MODULE_TEMPERATURE_BASE + (module[index].cellAvgTemp * MODULE_TEMPERATURE_FACTOR);

  chgLimit    = MODULE_CURRENT_BASE + (module[index].maxChargeA  	* MODULE_CURRENT_FACTOR);
  dischgLimit = MODULE_CURRENT_BASE + (module[index].maxDischargeA 	* MODULE_CURRENT_FACTOR);
  endVoltage  = MODULE_VOLTAGE_BASE + (module[index].maxChargeEndV 	* MODULE_VOLTAGE_FACTOR);

  grpModuleData->Caption			= "Module Data for Module #" + IntToStr(index);

  editModuleState->Text 			= sState;
  editModuleStatus->Text 			= sStatus;
  editModuleSoc->Text 				= FloatToStrF(soc,ffFixed,5,2)         + "%";
  editModuleSoh->Text 				= FloatToStrF(soh,ffFixed,5,2)  + "%";

  editModuleBalanceActive->Text 	= "N/A";
  editModuleBalanceStatus->Text 	= "N/A";
  editModuleCellCount->Text         = IntToStr(module[index].cellCount);

  editModuleFault0->Text 			= module[index].faultCode.commsError;
  editModuleFault1->Text 			= module[index].faultCode.hwIncompatible;
  editModuleFault2->Text 			= module[index].faultCode.overCurrent;
  editModuleFault3->Text 			= module[index].faultCode.overTemperature;
  editModuleFault4->Text 			= module[index].faultCode.overVoltage;


  editModuleVoltage->Text			= FloatToStrF(voltage,ffFixed,5,2) + "V";
  editModuleCurrent->Text 			= FloatToStrF(current,ffFixed,5,2) + "A";

  editModuleHiCellVolt->Text 		= FloatToStrF(hiCellVolt,ffFixed,5,2)  + "V";
  editModuleLoCellVolt->Text 		= FloatToStrF(loCellVolt,ffFixed,5,2)  + "V";
  editModuleAvgCellVolt->Text 		= FloatToStrF(avgCellVolt,ffFixed,5,2) + "V";

  editModuleHiCellTemp->Text 		= FloatToStrF(hiCellTemp,ffFixed,5,2)  + "C";
  editModuleLoCellTemp->Text 		= FloatToStrF(loCellTemp,ffFixed,5,2)  + "C";
  editModuleAvgCellTemp->Text 		= FloatToStrF(avgCellTemp,ffFixed,5,2) + "C";

  editModuleChargeLimit->Text 		= FloatToStrF(chgLimit,ffFixed,5,2) + "A";
  editModuleDischargeLimit->Text	= FloatToStrF(dischgLimit,ffFixed,5,2) + "A";
  editModuleEndVoltage->Text 		= FloatToStrF(endVoltage,ffFixed,5,2)  + "V";

  editModuleIsolation->Text 		= "N/A";

}
//---------------------------------------------------------------------------

/*
		editSelectedState->Text = cboState->Text;
	editSelectedState->TextHint = cboState->ItemIndex;
    */
void __fastcall TForm1::cboStateChange(TObject *Sender)
{
	editSelectedState->Text = cboState->Text;
	editSelectedState->TextHint = cboState->ItemIndex;
}
//---------------------------------------------------------------------------


void TForm1::ProcessModuleState(TPCANMsgFD theMsg){

	CANFRM_0x411_MODULE_STATE modState;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modState,0,sizeof(modState));
	memcpy(&modState, theMsg.DATA, sizeof(modState));

	moduleId = modState.module_id;

    // Add explicit cast here
	module[moduleId].currentState = static_cast<moduleState>(modState.module_state);
	module[moduleId].soh            			= modState.module_soh;
	module[moduleId].soc            			= modState.module_soc;
	module[moduleId].status						= modState.module_status;
	module[moduleId].faultCode.commsError       = modState.module_fault_code & 0x01;
	module[moduleId].faultCode.hwIncompatible   = modState.module_fault_code & 0x02;
	module[moduleId].faultCode.commsError       = modState.module_fault_code & 0x04;
	module[moduleId].faultCode.overCurrent		= modState.module_fault_code & 0x08;
	module[moduleId].faultCode.overTemperature  = modState.module_fault_code & 0x10;
	module[moduleId].faultCode.overVoltage      = modState.module_fault_code & 0x20;

	module[moduleId].cellCount      			= modState.module_cell_count;

	/*
	modState.module_cell_balance_status;
	modState.module_cell_balance_active;

	modState.module_count_total;
	modState.module_count_active;
   */

   UpdateModuleDisplay();
}

void TForm1::ProcessModulePower(TPCANMsgFD theMsg){

	CANFRM_0x412_MODULE_POWER modPower;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modPower,0,sizeof(modPower));
	memcpy(&modPower, theMsg.DATA, sizeof(modPower));

	moduleId 				= modPower.module_id;
	module[moduleId].mmc	= modPower.module_current;
	module[moduleId].mmv	= modPower.module_voltage;

	UpdateModuleDisplay();
}

void TForm1::ProcessModuleCellVoltage(TPCANMsgFD theMsg){

	CANFRM_0x413_MODULE_CELL_VOLTAGE modCellVoltage;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modCellVoltage,0,sizeof(modCellVoltage));
	memcpy(&modCellVoltage, theMsg.DATA, sizeof(modCellVoltage));

	moduleId 						= modCellVoltage.module_id;
	module[moduleId].cellHiVolt		= modCellVoltage.module_high_cell_volt;
	module[moduleId].cellLoVolt		= modCellVoltage.module_low_cell_volt;
	module[moduleId].cellAvgVolt	= modCellVoltage.module_avg_cell_volt;

	UpdateModuleDisplay();
}

void TForm1::ProcessModuleCellTemp(TPCANMsgFD theMsg){

	CANFRM_0x414_MODULE_CELL_TEMP modCellTemp;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modCellTemp,0,sizeof(modCellTemp));
	memcpy(&modCellTemp, theMsg.DATA, sizeof(modCellTemp));

	moduleId 						= modCellTemp.module_id;
	module[moduleId].cellHiTemp		= modCellTemp.module_high_cell_temp;
	module[moduleId].cellLoTemp		= modCellTemp.module_low_cell_temp;
	module[moduleId].cellAvgTemp	= modCellTemp.module_avg_cell_temp;

	UpdateModuleDisplay();
}

void TForm1::ProcessModuleCellId(TPCANMsgFD theMsg){

	CANFRM_0x415_MODULE_CELL_ID modCellId;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modCellId,0,sizeof(modCellId));
	memcpy(&modCellId, theMsg.DATA, sizeof(modCellId));

	/*
	DATA UNUSED AT PRESENT
	*/

	UpdateModuleDisplay();
}

void TForm1::ProcessModuleLimits(TPCANMsgFD theMsg){

	CANFRM_0x416_MODULE_LIMITS modLimits;

	uint8_t moduleId;

	// copy received data to status structure
	memset(&modLimits,0,sizeof(modLimits));
	memcpy(&modLimits, theMsg.DATA, sizeof(modLimits));

	moduleId 						= modLimits.module_id;
	module[moduleId].maxDischargeA	= modLimits.module_dischage_limit;
	module[moduleId].maxChargeA		= modLimits.module_charge_limit;
	module[moduleId].maxChargeEndV	= modLimits.module_charge_end_voltage_limit;

	UpdateModuleDisplay();
}

void TForm1::ProcessModuleList(TPCANMsgFD theMsg){
	CANFRM_0x41F_MODULE_LIST modList;

	// copy received data to status structure
	memset(&modList,0,sizeof(modList));
	memcpy(&modList, theMsg.DATA, sizeof(modList));

	/*
	DATA UNUSED AT PRESENT
	*/

}
void __fastcall TForm1::btnSendStateClick(TObject *Sender)
{
	 TPCANStatus stsResult;

	// Send the message
	//
	stsResult = WriteState();

	// The Hardware was successfully sent
	//
   /*
	if (stsResult == PCAN_ERROR_OK)
		IncludeTextMessage("Message was successfully SENT");
	else
		// An error occurred.  We show the error.
		//
		::MessageBox(NULL, GetFormatedError(stsResult).c_str(), "Error!",MB_ICONERROR);
   */
}
//---------------------------------------------------------------------------

void TForm1::UpdateStatus(const String& status)
{
	lblWeb4Status->Caption = "Status: " + status;
	Application->ProcessMessages();
}


void TForm1::ShowProgress(int position)
{
	web4Progress->Position = position;
    Application->ProcessMessages();
}

void TForm1::RegisterSingleComponent(const System::UnicodeString& componentId,
                                   const System::UnicodeString& componentType)
{
    try {
        LogMessage("  Attempting to register: " + componentId);

        // FIXED: More defensive programming
        if (!FWeb4Client) {
            LogMessage("  ✗ Web4 client is null");
            return;
        }

        if (!FWeb4Client->IsConnected()) {
            LogMessage("  ✗ Web4 client not connected");
            return;
        }

        // Register the component with proper error handling
        std::unique_ptr<TComponentRegistration> registration;

        try {
            registration = FWeb4Client->RegisterComponent(
                FConfig.DefaultCreator,   // creator (uses default from config)
                componentId,              // component_data
                componentType + "_demo"   // context
            );
        }
        catch (const EWeb4Exception& webEx) {
            LogMessage("  ✗ Web4 Exception: " + webEx.Message);
            if (webEx.ErrorCode > 0) {
                LogMessage("    Error Code: " + IntToStr(webEx.ErrorCode));
            }
            return;
        }
        catch (const Exception& ex) {
            LogMessage("  ✗ General Exception: " + ex.Message);
            return;
        }
        catch (...) {
            LogMessage("  ✗ Unknown exception occurred");
            return;
        }

        if (registration) {
			LogMessage("  ✓ Registered: " + componentId);
			LogMessage("    Component ID: " + registration->ComponentId);
			LogMessage("    Identity: " + registration->ComponentIdentity);
			LogMessage("    LCT ID: " + registration->LctId);
			LogMessage("    Status: " + registration->Status);

            // FIXED: Optional verification with better error handling
            try {
                std::unique_ptr<TComponentRegistration> verification =
                    FWeb4Client->GetComponent(registration->ComponentId);

                if (verification) {
                    LogMessage("  ✓ Verification successful");
                } else {
                    LogMessage("  ⚠ Component verification failed - but registration succeeded");
                }
            }
            catch (...) {
                LogMessage("  ⚠ Verification check failed - but registration succeeded");
            }
        } else {
            LogMessage("  ✗ Failed to register: " + componentId);
            LogMessage("    Registration returned null");
        }
    }
    catch (const Exception& e) {
        LogMessage("  ✗ Top-level exception in RegisterSingleComponent: " + e.Message);
    }
    catch (...) {
        LogMessage("  ✗ Unknown top-level exception in RegisterSingleComponent");
    }
}
void TForm1::RegisterSingleComponentDebug(const System::UnicodeString& componentId,
										const System::UnicodeString& componentType)
{
	LogMessage(" ComponentId: " + componentId);
	LogMessage(" ComponentType: " + componentType);
	LogMessage(" DefaultCreator: " + FConfig.DefaultCreator);

	try {
        if (!FWeb4Client) {
			LogMessage(" ERROR: FWeb4Client is null");
            return;
        }

		LogMessage(" Client exists, checking connection...");
        if (!FWeb4Client->IsConnected()) {
			LogMessage(" ERROR: Client not connected");
            return;
        }

		LogMessage(" Connected, attempting registration...");

        // Test the connection first
        try {
            System::Json::TJSONObject* health = FWeb4Client->CheckHealth();
            if (health) {
				LogMessage(" Health check passed");
                delete health;
            } else {
				LogMessage(" Health check failed");
                return;
            }
        }
        catch (...) {
			LogMessage(" Health check threw exception");
            return;
        }

		LogMessage(" About to call RegisterComponent...");

		std::unique_ptr<TComponentRegistration> registration =
			FWeb4Client->RegisterComponent(
				FConfig.DefaultCreator,
				componentId,
				componentType + "_demo"
			);

		LogMessage(" RegisterComponent call completed");

		if (registration) {
			LogMessage(" Registered: " + componentId);
			
			// Log the complete API response details
			LogMessage("  API Response Details:");
			LogMessage("    Component Data: " + registration->ComponentData);
			LogMessage("    Generated Component ID: " + registration->ComponentId);
			LogMessage("    Component Identity: " + registration->ComponentIdentity);
			LogMessage("    LCT ID: " + registration->LctId);
			LogMessage("    Context: " + registration->Context);
			LogMessage("    Creator: " + registration->Creator);
			LogMessage("    Status: " + registration->Status);
			LogMessage("    Transaction Hash: " + registration->TxHash);
			
			// Store the generated component ID for later use in LCT creation
			if (componentType == "battery_pack") {
				FRegisteredPackId = registration->ComponentId;
				LogMessage("    Stored Pack ID: " + FRegisteredPackId);
			} else if (componentType == "battery_module") {
				FRegisteredModuleId = registration->ComponentId;
				LogMessage("    Stored Module ID: " + FRegisteredModuleId);
			} else if (componentType == "host_system") {
				FRegisteredHostId = registration->ComponentId;
				LogMessage("    Stored Host ID: " + FRegisteredHostId);
			}
		} else {
			LogMessage(" Registration returned null");
		}
    }
    catch (const EWeb4Exception& e) {
		LogMessage(" Web4Exception: " + e.Message);
		LogMessage(" Error Code: " + IntToStr(e.ErrorCode));
		LogMessage(" Error Type: " + e.ErrorType);
    }
    catch (const Exception& e) {
		LogMessage(" Exception: " + e.Message);
		LogMessage(" Class: " + e.ClassName());
    }
    catch (...) {
		LogMessage(" Unknown exception caught");
    }
}



void TForm1::LogMessage(const System::UnicodeString& message)
{
    System::UnicodeString timestamp = FormatDateTime("hh:nn:ss", Now());
	memoWeb4Log->Lines->Add("[" + timestamp + "] " + message);

    // Auto-scroll to bottom
	SendMessage(memoWeb4Log->Handle, WM_VSCROLL, SB_BOTTOM, 0);

    // Force update
    Application->ProcessMessages();
}


/*
void TForm1::LogMessage(const String& message)
{
	memoWeb4Log->Lines->Add(FormatDateTime("hh:nn:ss", Now()) + " - " + message);
	Application->ProcessMessages();
}
  */

void __fastcall TForm1::btnRegisterClick(TObject *Sender)
{
	btnRegister->Enabled = false;  // Disable button immediately

	try
	{

		FWeb4Client.reset(new TWeb4BridgeClient(FConfig));
		if (FWeb4Client->Connect()) {
			UpdateStatus("Connected to API Bridge");
			web4Progress->Position = 10;
			LogMessage("Successfully connected to API Bridge at " +  FConfig.ApiEndpoint + ":" + FConfig.RestPort);

			// Test health check
			System::Json::TJSONObject* health = FWeb4Client->CheckHealth();
			if (health) {
				LogMessage("Health check passed");
				delete health;
			}

			UpdateStatus("Registering components...");
			ShowProgress(0);

			// Register Battery Pack
			LogMessage("Registering battery pack: " + edtBatteryPackId->Text);

			RegisterSingleComponentDebug(edtBatteryPackId->Text, "battery_pack");
			ShowProgress(33);

			// Register Battery Module
			LogMessage("Registering battery module: " + edtBatteryModuleId->Text);
			RegisterSingleComponentDebug(edtBatteryModuleId->Text, "battery_module");
			ShowProgress(66);

			// Register Host System
			LogMessage("Registering host system: " + edtHostSystemId->Text);
			RegisterSingleComponentDebug(edtHostSystemId->Text, "host_system");
			ShowProgress(100);

			UpdateStatus("Registration complete");
			LogMessage("Registration complete");
			LogMessage("");

		}

	}
	catch (const Exception& e)
	{
		LogMessage("! Registration failed: " + e.Message);
		UpdateStatus("Registration failed");
		ShowMessage("Registration failed: " + e.Message);

	}
	btnRegister->Enabled = true;   // Re-enable button
	ShowProgress(0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnCreateClick(TObject *Sender)
{
    try
    {
        // Check if client exists and is connected
        if (!FWeb4Client) {
            LogMessage("✗ Web4 client is null - please register components first");
            UpdateStatus("Error: Web4 client not initialized");
            ShowMessage("Please register components first before creating LCT relationships.");
            return;
        }

        if (!FWeb4Client->IsConnected()) {
            LogMessage("✗ Web4 client not connected");
            UpdateStatus("Error: Web4 client not connected");
            ShowMessage("Web4 client is not connected. Please register components first.");
            return;
        }

        // Validate input fields
        if (edtBatteryModuleId->Text.IsEmpty() || edtBatteryPackId->Text.IsEmpty()) {
            LogMessage("✗ Missing required component IDs");
            UpdateStatus("Error: Missing component IDs");
            ShowMessage("Please enter both Battery Module ID and Battery Pack ID.");
            return;
        }

        UpdateStatus("Creating LCT relationships...");
        ShowProgress(25);
        LogMessage("Starting LCT relationship creation...");

        // Create LCT between battery module and pack
        LogMessage("Creating LCT: " + edtBatteryModuleId->Text + " ↔ " + edtBatteryPackId->Text);
        LogMessage("Parameters: creator='" + FConfig.DefaultCreator + "', context='race_car_demo'");
        
        // Check if we have the registered component IDs
        LogMessage("Checking for registered component IDs...");
        if (FRegisteredModuleId.IsEmpty() || FRegisteredPackId.IsEmpty()) {
            LogMessage("✗ Registered component IDs not found");
            LogMessage("  Module ID: " + (FRegisteredModuleId.IsEmpty() ? "(empty)" : FRegisteredModuleId));
            LogMessage("  Pack ID: " + (FRegisteredPackId.IsEmpty() ? "(empty)" : FRegisteredPackId));
            LogMessage("  Host ID: " + (FRegisteredHostId.IsEmpty() ? "(empty)" : FRegisteredHostId));
            LogMessage("");
            LogMessage("SOLUTION: Click the 'Register' button first to register these components:");
            LogMessage("  1. " + edtBatteryModuleId->Text);
            LogMessage("  2. " + edtBatteryPackId->Text);
            if (!edtHostSystemId->Text.IsEmpty()) {
                LogMessage("  3. " + edtHostSystemId->Text);
            }
            
            UpdateStatus("Components not registered - please register them first");
            ShowMessage("Components not registered!\n\nPlease click the 'Register' button first to register these components:\n\n• " + 
                       edtBatteryModuleId->Text + "\n• " + edtBatteryPackId->Text + 
                       (edtHostSystemId->Text.IsEmpty() ? "" : "\n• " + edtHostSystemId->Text) +
                       "\n\nThen try creating the LCT relationships again.");
            ShowProgress(0);
            return;
        }
        
        LogMessage("✓ Using registered component IDs:");
        LogMessage("  Module: " + FRegisteredModuleId + " (from " + edtBatteryModuleId->Text + ")");
        LogMessage("  Pack: " + FRegisteredPackId + " (from " + edtBatteryPackId->Text + ")");
        if (!FRegisteredHostId.IsEmpty()) {
            LogMessage("  Host: " + FRegisteredHostId + " (from " + edtHostSystemId->Text + ")");
        }
        
        LogMessage("Calling CreateLct with:");
        LogMessage("  Creator: " + FConfig.DefaultCreator);
        LogMessage("  ComponentA: " + FRegisteredModuleId);
        LogMessage("  ComponentB: " + FRegisteredPackId);
        LogMessage("  Context: race_car_demo");
        
        std::unique_ptr<TLctRelationship> lct1;
        try {
            // Test connection before LCT creation
        try {
            LogMessage("Testing connection before LCT creation...");
            System::Json::TJSONObject* health = FWeb4Client->CheckHealth();
            if (health) {
                LogMessage("Connection test passed");
                delete health;
            } else {
                LogMessage("Connection test failed");
                UpdateStatus("Connection lost");
                ShowProgress(0);
                return;
            }
        }
        catch (const Exception& e) {
            LogMessage("Connection test exception: " + e.Message);
            UpdateStatus("Connection error");
            ShowProgress(0);
            return;
        }
        
        LogMessage("Creating LCT relationship...");
        
        LogMessage("About to call FWeb4Client->CreateLct...");
            lct1 = FWeb4Client->CreateLct(
                FConfig.DefaultCreator,           // creator
                FRegisteredModuleId,             // componentA (use stored registered ID)
                FRegisteredPackId,               // componentB (use stored registered ID)
                "race_car_demo"                  // context
            );
            LogMessage("CreateLct call completed");
        }
        catch (const Exception& e) {
            LogMessage("✗ Exception during CreateLct: " + e.Message);
            UpdateStatus("Exception during LCT creation");
            ShowProgress(0);
            return;
        }

        if (lct1) {
            LogMessage("✓ LCT relationship created: " + edtBatteryModuleId->Text + " ↔ " + edtBatteryPackId->Text);
            LogMessage("  Actual IDs used: " + FRegisteredModuleId + " ↔ " + FRegisteredPackId);
            LogMessage("  LCT ID: " + lct1->LctId);
            LogMessage("  Status: " + lct1->Status);
            LogMessage("  Device Key Half: " + lct1->DeviceKeyHalf);
            LogMessage("  LCT Key Half: " + lct1->LctKeyHalf);
            
            // Store Web4 data in battery structures for CAN transmission
            StoreWeb4DataInModule(edtBatteryModuleId->Text, lct1.get(), FRegisteredModuleId);
            StoreWeb4DataInPack(edtBatteryPackId->Text, lct1.get(), FRegisteredPackId);
            LogMessage("  Web4 data stored in battery structures for CAN transmission");
        } else {
            LogMessage("✗ Failed to create LCT relationship between module and pack");
            LogMessage("  This usually means the API returned an error or validation failed");
            UpdateStatus("Failed to create first LCT relationship");
            ShowProgress(0);
            return;
        }

        ShowProgress(60);

        // Create LCT between app (host) and pack controller for encrypted CAN communication
        std::unique_ptr<TLctRelationship> appToPackLct;
        if (!FRegisteredHostId.IsEmpty()) {
            LogMessage("Creating App ↔ Pack Controller LCT for encrypted CAN communication");
            LogMessage("Parameters: creator='" + FConfig.DefaultCreator + "', app_id='" + FRegisteredHostId + "', pack_id='" + FRegisteredPackId + "'");
            
            try {
                appToPackLct = FWeb4Client->CreateLct(
                    FConfig.DefaultCreator,
                    FRegisteredHostId,        // app component ID
                    FRegisteredPackId,        // pack controller component ID  
                    "app_pack_communication"  // context for app-pack communication
                );
                
                if (appToPackLct) {
                    LogMessage("✓ App-to-Pack LCT created successfully");
                    LogMessage("  LCT ID: " + appToPackLct->LctId);
                    LogMessage("  Status: " + appToPackLct->Status);
                    LogMessage("  App Device Key Half: " + appToPackLct->DeviceKeyHalf);
                    LogMessage("  Pack Device Key Half: " + appToPackLct->LctKeyHalf);
                    
                    // Store app's portion of the keys (these will be used for CAN encryption)
                    LogMessage("  Storing app-pack encryption keys for CAN communication...");
                    
                    FAppDeviceKeyHalf = appToPackLct->DeviceKeyHalf;
                    FPackDeviceKeyHalf = appToPackLct->LctKeyHalf;
                    
                    LogMessage("  ✓ App-pack encryption keys stored for CAN communication");
                    LogMessage("  Keys ready for distribution to pack controller hardware");
                    
                } else {
                    LogMessage("✗ Failed to create App-to-Pack LCT");
                    UpdateStatus("Failed to create app-pack LCT");
                    ShowProgress(0);
                    return;
                }
            } catch (const Exception& e) {
                LogMessage("✗ Exception during App-to-Pack LCT creation: " + e.Message);
                UpdateStatus("Exception during app-pack LCT creation");
                ShowProgress(0);
                return;
            }
        } else {
            LogMessage("Host not registered - skipping app-pack LCT creation");
            LogMessage("  Register the host system first to enable encrypted app-pack communication");
        }

        ShowProgress(75);

        // Create LCT between battery pack and host system (if host system ID is provided)
        if (!edtHostSystemId->Text.IsEmpty() && !FRegisteredHostId.IsEmpty()) {
            LogMessage("Creating LCT: " + edtBatteryPackId->Text + " ↔ " + edtHostSystemId->Text);
            std::unique_ptr<TLctRelationship> lct2 = FWeb4Client->CreateLct(
                FConfig.DefaultCreator,
                FRegisteredPackId,        // Use stored registered pack ID
                FRegisteredHostId,        // Use stored registered host ID
                "race_car_demo"
            );

            if (lct2) {
                LogMessage("✓ LCT relationship created: " + edtBatteryPackId->Text + " ↔ " + edtHostSystemId->Text);
                LogMessage("  Actual IDs used: " + FRegisteredPackId + " ↔ " + FRegisteredHostId);
                LogMessage("  LCT ID: " + lct2->LctId);
                LogMessage("  Status: " + lct2->Status);
                LogMessage("  Device Key Half: " + lct2->DeviceKeyHalf);
                LogMessage("  LCT Key Half: " + lct2->LctKeyHalf);
            } else {
                LogMessage("✗ Failed to create LCT relationship between pack and host");
                UpdateStatus("Failed to create second LCT relationship");
                ShowProgress(0);
                return;
            }
        } else if (!edtHostSystemId->Text.IsEmpty() && FRegisteredHostId.IsEmpty()) {
            LogMessage("Host System ID provided but not registered - skipping pack-to-host LCT creation");
            LogMessage("  Register the host system first to create this relationship");
        } else {
            LogMessage("Host System ID not provided - skipping pack-to-host LCT creation");
        }

        ShowProgress(90);
        
        // Distribute keys to pack controller hardware
        if (!FAppDeviceKeyHalf.IsEmpty() && !FPackDeviceKeyHalf.IsEmpty()) {
            LogMessage("Distributing encryption keys to pack controller hardware...");
            DistributeKeysToPackController();
        } else {
            LogMessage("No app-pack encryption keys available - skipping key distribution");
        }

        ShowProgress(100);
        UpdateStatus("LCT relationships created and keys distributed successfully");
        LogMessage("All LCT relationships created and keys distributed successfully!");
        ShowMessage("LCT relationships created and keys distributed successfully!");
    }
    catch (const Exception& e)
    {
        LogMessage("! Relationship creation failed: " + e.Message);
        UpdateStatus("Relationship creation failed");
        ShowMessage("Relationship creation failed: " + e.Message);
    }

	ShowProgress(0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btnDemoClick(TObject *Sender)
{
		 try
	{
		/*
		UpdateStatus("Running complete Web4 demo workflow...");
		LogMessage("Starting Complete Web4 Demo Workflow...");
		web4Progress->Position = 20;

		std::vector<String> moduleIds = {edtWeb4ModuleId->Text, edtWeb4ModuleId->Text + "-002", edtWeb4ModuleId->Text + "-003"};
		std::vector<String> packIds = {edtWeb4PackId->Text, edtWeb4PackId->Text.SubString(1, edtWeb4PackId->Text.Length()-1) + "B"};

		//auto results = FWeb4Client->DemoCompleteWeb4Workflow(moduleIds, packIds, edtWeb4HostId->Text);
		web4Progress->Position = 100;

		LogMessage("Complete Web4 Demo Workflow Finished!");
		LogMessage("Summary:");

		auto registrations = dynamic_cast<TJSONArray*>(results["component_registrations"]);
		auto relationships = dynamic_cast<TJSONArray*>(results["lct_relationships"]);
		auto tensors = dynamic_cast<TJSONArray*>(results["trust_tensors"]);
		auto operations = dynamic_cast<TJSONArray*>(results["energy_operations"]);
		auto balances = dynamic_cast<TJSONArray*>(results["final_balances"]);

		LogMessage(" - Components: " + IntToStr(registrations ? registrations->Count : 0));
		LogMessage(" - LCT Relationships: " + IntToStr(relationships ? relationships->Count : 0));
		LogMessage(" - Trust Tensors: " + IntToStr(tensors ? tensors->Count : 0));
		LogMessage(" - Energy Operations: " + IntToStr(operations ? operations->Count : 0));
		LogMessage(" - Final Balances: " + IntToStr(balances ? balances->Count : 0));

		UpdateStatus("Complete demo workflow finished successfully");
		ShowMessage("Complete Web4 demo workflow executed successfully!\n\n"
					" - All 6 Web4 modules integrated\n"
					" - LCTs as relationships demonstrated\n"
					" - Trust tensors operational\n"
					" - ATP/ADP energy cycles active");
		*/
	}

	catch (const Exception& e)
	{
		LogMessage("! Demo workflow failed: " + e.Message);
		UpdateStatus("Demo workflow failed");
		ShowMessage("Demo workflow failed: " + e.Message + "\n\nMake sure your Web4 blockchain is running on localhost:1317");
	}

	ShowProgress(0);

}
//---------------------------------------------------------------------------

// Web4 integration helper functions
void TForm1::StoreWeb4DataInModule(const System::UnicodeString& originalModuleId, const TLctRelationship* lct, const System::UnicodeString& generatedComponentId) {
    // Find the module in your module array and update it
    // This assumes you have a way to map from originalModuleId to your module array index
    // You'll need to implement your specific logic here based on how you identify modules
    
    // Example implementation (you'll need to adapt this to your module management):
    for (int i = 0; i < MAX_MODULES_PER_PACK; i++) {
        // You'll need to determine which module corresponds to originalModuleId
        // This could be based on module ID, index, or some other identifier
        
        // For now, let's assume we're storing in the first available module
        // You should replace this with your actual module selection logic
        if (i == 0) { // Replace with proper module identification
            // Copy key halves to the module structure
            strncpy(module[i].web4DeviceKeyHalf, AnsiString(lct->DeviceKeyHalf).c_str(), 63);
            module[i].web4DeviceKeyHalf[63] = '\0'; // Ensure null termination
            
            strncpy(module[i].web4LctKeyHalf, AnsiString(lct->LctKeyHalf).c_str(), 63);
            module[i].web4LctKeyHalf[63] = '\0';
            
            strncpy(module[i].web4ComponentId, AnsiString(generatedComponentId).c_str(), 63);
            module[i].web4ComponentId[63] = '\0';
            
            module[i].web4Registered = true;
            
            LogMessage("    Module Web4 data stored:");
            LogMessage("      Device Key: " + String(module[i].web4DeviceKeyHalf));
            LogMessage("      Component ID: " + String(module[i].web4ComponentId));
            break;
        }
    }
}

void TForm1::StoreWeb4DataInPack(const System::UnicodeString& originalPackId, const TLctRelationship* lct, const System::UnicodeString& generatedComponentId) {
    try {
        // Copy key halves to the global pack structure  
        strncpy(pack.web4DeviceKeyHalf, AnsiString(lct->DeviceKeyHalf).c_str(), 63);
        pack.web4DeviceKeyHalf[63] = '\0'; // Ensure null termination
        
        strncpy(pack.web4LctKeyHalf, AnsiString(lct->LctKeyHalf).c_str(), 63);
        pack.web4LctKeyHalf[63] = '\0';
        
        strncpy(pack.web4ComponentId, AnsiString(generatedComponentId).c_str(), 63);
        pack.web4ComponentId[63] = '\0';
        
        pack.web4Registered = true;
        
        LogMessage("    ✓ Pack Web4 data stored successfully:");
        LogMessage("      Device Key: " + lct->DeviceKeyHalf);
        LogMessage("      LCT Key: " + lct->LctKeyHalf);
        LogMessage("      Component ID: " + generatedComponentId);
        LogMessage("      Original Pack ID: " + originalPackId);
        
    } catch (const Exception& e) {
        LogMessage("    ✗ Failed to store pack Web4 data: " + e.Message);
    }
}
//---------------------------------------------------------------------------

///// Web4 Key Distribution Functions /////

void TForm1::DistributeKeysToPackController(void) {
    LogMessage("Starting key distribution to pack controller hardware...");
    
    // Check if we have the required keys
    if (FAppDeviceKeyHalf.IsEmpty() || FPackDeviceKeyHalf.IsEmpty()) {
        LogMessage("✗ No app-pack keys available for distribution");
        LogMessage("  Create LCT relationships first to generate encryption keys");
        return;
    }
    
    if (FRegisteredPackId.IsEmpty() || FRegisteredHostId.IsEmpty()) {
        LogMessage("✗ Component IDs not available for distribution");
        return;
    }
    
    try {
        LogMessage("Distributing Web4 keys via CAN...");
        
        // Send pack controller's device key half
        LogMessage("  Sending pack controller device key half...");
        TPCANStatus result1 = SendKeyData(ID_VCU_WEB4_PACK_KEY_HALF, FPackDeviceKeyHalf);
        if (result1 != PCAN_ERROR_OK) {
            LogMessage("  ✗ Failed to send pack key half: " + GetFormatedError(result1));
            return;
        }
        LogMessage("  ✓ Pack key half sent successfully");
        
        // Small delay between messages
        Sleep(10);
        
        // Send app's device key half  
        LogMessage("  Sending app device key half...");
        TPCANStatus result2 = SendKeyData(ID_VCU_WEB4_APP_KEY_HALF, FAppDeviceKeyHalf);
        if (result2 != PCAN_ERROR_OK) {
            LogMessage("  ✗ Failed to send app key half: " + GetFormatedError(result2));
            return;
        }
        LogMessage("  ✓ App key half sent successfully");
        
        // Small delay between messages
        Sleep(10);
        
        // Send component IDs
        LogMessage("  Sending component IDs...");
        TPCANStatus result3 = SendComponentIds();
        if (result3 != PCAN_ERROR_OK) {
            LogMessage("  ✗ Failed to send component IDs: " + GetFormatedError(result3));
            return;
        }
        LogMessage("  ✓ Component IDs sent successfully");
        
        LogMessage("✓ All Web4 keys distributed to pack controller successfully!");
        LogMessage("  Pack controller should now have:");
        LogMessage("    - Pack device key half (64 bytes)");
        LogMessage("    - App device key half (64 bytes)");  
        LogMessage("    - Pack component ID");
        LogMessage("    - App component ID");
        LogMessage("  Pack controller can now combine keys for encrypted CAN communication");
        
    } catch (const Exception& e) {
        LogMessage("✗ Key distribution failed: " + e.Message);
    }
}

TPCANStatus TForm1::SendKeyData(uint32_t baseCanId, const System::UnicodeString& keyData) {
    // Convert Unicode string to AnsiString and then to bytes
    AnsiString ansiKeyData = AnsiString(keyData);
    const char* keyBytes = ansiKeyData.c_str();
    int keyLength = ansiKeyData.Length();
    
    LogMessage(String().sprintf(L"  Sending %d bytes in 8 chunks with acknowledgment...", keyLength));
    
    // Send 8 chunks of 8 bytes each for 64-byte key
    for (int chunk = 0; chunk < 8; chunk++) {
        int retries = 0;
        bool chunkAcked = false;
        
        while (!chunkAcked && retries < 3) {
            // Send chunk with chunk number in extended ID
            TPCANMsg CANMsg = TPCANMsg();
            CANMsg.ID = baseCanId | (chunk << 8) | packID;  // Chunk in bits 8-15
            CANMsg.LEN = 8;
            CANMsg.MSGTYPE = PCAN_MESSAGE_EXTENDED;
            
            // Create generic 8-byte structure using individual byte fields  
            // All Web4 structures (0x407, 0x408, 0x409) have identical layout
            struct {
                uint8_t BYTE0, BYTE1, BYTE2, BYTE3, BYTE4, BYTE5, BYTE6, BYTE7;
            } keyFrame = {0};
            
            // Copy bytes individually for this chunk
            int startByte = chunk * 8;
            int bytesToCopy = std::min(8, keyLength - startByte);
            if (bytesToCopy >= 1) keyFrame.BYTE0 = keyBytes[startByte + 0];
            if (bytesToCopy >= 2) keyFrame.BYTE1 = keyBytes[startByte + 1];
            if (bytesToCopy >= 3) keyFrame.BYTE2 = keyBytes[startByte + 2];
            if (bytesToCopy >= 4) keyFrame.BYTE3 = keyBytes[startByte + 3];
            if (bytesToCopy >= 5) keyFrame.BYTE4 = keyBytes[startByte + 4];
            if (bytesToCopy >= 6) keyFrame.BYTE5 = keyBytes[startByte + 5];
            if (bytesToCopy >= 7) keyFrame.BYTE6 = keyBytes[startByte + 6];
            if (bytesToCopy >= 8) keyFrame.BYTE7 = keyBytes[startByte + 7];
            
            memcpy(CANMsg.DATA, &keyFrame, sizeof(keyFrame));
            
            TPCANStatus sendResult = m_objPCANBasic->Write(m_PcanHandle, &CANMsg);
            if (sendResult != PCAN_ERROR_OK) {
                LogMessage(String().sprintf(L"  ✗ Failed to send chunk %d: %s", chunk, GetFormatedError(sendResult)));
                return sendResult;
            }
            
            LogMessage(String().sprintf(L"  → Sent chunk %d/8 (%d bytes)", chunk + 1, bytesToCopy));
            
            // Wait for ACK
            chunkAcked = WaitForChunkAck(baseCanId, chunk, 1000); // 1 second timeout
            
            if (!chunkAcked) {
                retries++;
                LogMessage(String().sprintf(L"  ⚠ Chunk %d not acknowledged, retry %d/3", chunk + 1, retries));
                Sleep(100); // Brief delay before retry
            }
        }
        
        if (!chunkAcked) {
            LogMessage(String().sprintf(L"  ✗ Chunk %d failed after 3 retries", chunk + 1));
            return PCAN_ERROR_XMTFULL; // Use existing error code
        }
        
        LogMessage(String().sprintf(L"  ✓ Chunk %d acknowledged", chunk + 1));
    }
    
    LogMessage(L"  ✓ All chunks sent and acknowledged successfully");
    return PCAN_ERROR_OK;
}

bool TForm1::WaitForChunkAck(uint32_t baseCanId, int expectedChunk, int timeoutMs) {
    DWORD startTime = GetTickCount();
    
    while ((GetTickCount() - startTime) < timeoutMs) {
        TPCANMsg receivedMsg;
        TPCANTimestamp timestamp;
        TPCANStatus result = m_objPCANBasic->Read(m_PcanHandle, &receivedMsg, &timestamp);
        
        if (result == PCAN_ERROR_OK) {
            // Calculate expected response ID
            uint32_t responseBaseId = baseCanId + 0xA0;  // Response ID (e.g., 0x407 + 0xA0 = 0x4A7)
            uint32_t expectedId = responseBaseId | (expectedChunk << 8) | packID;
            
            if (receivedMsg.ID == expectedId && receivedMsg.LEN >= 4) {
                // Verify ACK content
                uint8_t ackStatus = receivedMsg.DATA[0];
                uint8_t ackChunk = receivedMsg.DATA[1];
                uint16_t checksum = (receivedMsg.DATA[2] << 8) | receivedMsg.DATA[3];
                
                if (ackStatus == 0x01 && ackChunk == expectedChunk) {
                    // ACK received with correct chunk number
                    // Could verify checksum here if needed
                    LogMessage(String().sprintf(L"    ← ACK received for chunk %d (checksum: 0x%04X)", 
                              expectedChunk + 1, checksum));
                    return true;
                } else if (ackStatus == 0x00) {
                    // NACK received
                    LogMessage(String().sprintf(L"    ← NACK received for chunk %d", expectedChunk + 1));
                    return false; // Will trigger retry
                }
            }
        } else if (result != PCAN_ERROR_QRCVEMPTY) {
            // Only log non-empty queue errors
            LogMessage(String().sprintf(L"    CAN read error while waiting for ACK: %s", 
                      GetFormatedError(result)));
        }
        
        Sleep(10); // Small delay to prevent busy waiting
    }
    
    LogMessage(String().sprintf(L"    ⏱ Timeout waiting for ACK on chunk %d", expectedChunk + 1));
    return false; // Timeout
}

TPCANStatus TForm1::SendComponentIds(void) {
    // Send pack component ID
    TPCANStatus result1 = SendKeyData(ID_VCU_WEB4_COMPONENT_IDS, FRegisteredPackId);
    if (result1 != PCAN_ERROR_OK) {
        return result1;
    }
    
    Sleep(10);
    
    // Send app component ID with different CAN ID (add 1 to distinguish)
    TPCANStatus result2 = SendKeyData(ID_VCU_WEB4_COMPONENT_IDS + 1, FRegisteredHostId);
    
    return result2;
}
//---------------------------------------------------------------------------

