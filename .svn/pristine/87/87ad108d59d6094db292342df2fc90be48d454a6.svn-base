object Form1: TForm1
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'modbatt Diagnostics and Configuration Utility V1.3.1'
  ClientHeight = 772
  ClientWidth = 759
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Position = poScreenCenter
  OnClose = FormClose
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 0
    Width = 744
    Height = 65
    Caption = ' Connection '
    TabOrder = 0
    object Label1: TLabel
      Left = 11
      Top = 16
      Width = 51
      Height = 13
      Caption = 'Hardware:'
    end
    object laBaudrate: TLabel
      Left = 231
      Top = 16
      Width = 48
      Height = 13
      Caption = 'Baudrate:'
    end
    object laHwType: TLabel
      Left = 353
      Top = 16
      Width = 78
      Height = 13
      Caption = 'Hardware Type:'
    end
    object laIOPort: TLabel
      Left = 479
      Top = 16
      Width = 43
      Height = 13
      Caption = 'I/O Port:'
    end
    object laInterrupt: TLabel
      Left = 540
      Top = 16
      Width = 48
      Height = 13
      Caption = 'Interrupt:'
    end
    object laBitrate: TLabel
      Left = 221
      Top = 9
      Width = 39
      Height = 13
      Caption = 'Bit rate:'
      Visible = False
    end
    object txtBitrate: TMemo
      Left = 221
      Top = 25
      Width = 374
      Height = 35
      Lines.Strings = (
        'txtBitrate')
      ScrollBars = ssVertical
      TabOrder = 8
      Visible = False
    end
    object cbbChannel: TComboBox
      Left = 11
      Top = 35
      Width = 143
      Height = 21
      Style = csDropDownList
      TabOrder = 0
      OnChange = cbbChannelChange
      Items.Strings = (
        'None'
        'DNG-Channel 1'
        'ISA-Channel 1'
        'ISA-Channel 2'
        'ISA-Channel 3'
        'ISA-Channel 4'
        'ISA-Channel 5'
        'ISA-Channel 6'
        'ISA-Channel 7'
        'ISA-Channel 8'
        'PCC-Channel 1'
        'PCC-Channel 2'
        'PCI-Channel 1'
        'PCI-Channel 2'
        'PCI-Channel 3'
        'PCI-Channel 4'
        'PCI-Channel 5'
        'PCI-Channel 6'
        'PCI-Channel 7'
        'PCI-Channel 8'
        'USB-Channel 1'
        'USB-Channel 2'
        'USB-Channel 3'
        'USB-Channel 4'
        'USB-Channel 5'
        'USB-Channel 6'
        'USB-Channel 7'
        'USB-Channel 8')
    end
    object btnHwRefresh: TButton
      Left = 160
      Top = 33
      Width = 55
      Height = 25
      Caption = 'Refresh'
      TabOrder = 1
      OnClick = btnHwRefreshClick
    end
    object cbbBaudrates: TComboBox
      Left = 221
      Top = 35
      Width = 116
      Height = 21
      Style = csDropDownList
      TabOrder = 2
      OnChange = cbbBaudratesChange
      Items.Strings = (
        '1 MBit/sec'
        '800 kBit/sec'
        '500 kBit/sec'
        '250 kBit/sec'
        '125 kBit/sec'
        '100 kBit/sec'
        '95,238 kBit/s'
        '83,333 kBit/s'
        '50 kBit/sec'
        '47,619 kBit/s'
        '33,333 kBit/s'
        '20 kBit/sec'
        '10 kBit/sec'
        '5 kBit/sec')
    end
    object cbbHwType: TComboBox
      Left = 345
      Top = 35
      Width = 120
      Height = 21
      Style = csDropDownList
      TabOrder = 3
      OnChange = cbbHwTypeChange
      Items.Strings = (
        'ISA-82C200'
        'ISA-SJA1000'
        'ISA-PHYTEC'
        'DNG-82C200'
        'DNG-82C200 EPP'
        'DNG-SJA1000'
        'DNG-SJA1000 EPP')
    end
    object cbbIO: TComboBox
      Left = 475
      Top = 35
      Width = 55
      Height = 21
      Style = csDropDownList
      TabOrder = 4
      Items.Strings = (
        '0100'
        '0120'
        '0140'
        '0200'
        '0220'
        '0240'
        '0260'
        '0278'
        '0280'
        '02A0'
        '02C0'
        '02E0'
        '02E8'
        '02F8'
        '0300'
        '0320'
        '0340'
        '0360'
        '0378'
        '0380'
        '03BC'
        '03E0'
        '03E8'
        '03F8')
    end
    object cbbInterrupt: TComboBox
      Left = 540
      Top = 35
      Width = 55
      Height = 21
      Style = csDropDownList
      TabOrder = 5
      Items.Strings = (
        '3'
        '4'
        '5'
        '7'
        '9'
        '10'
        '11'
        '12'
        '15')
    end
    object btnConnect: TButton
      Left = 672
      Top = 11
      Width = 65
      Height = 25
      Caption = 'Connect'
      TabOrder = 6
      OnClick = btnConnectClick
    end
    object btnRelease: TButton
      Left = 672
      Top = 37
      Width = 65
      Height = 25
      Caption = 'Release'
      Enabled = False
      TabOrder = 7
      OnClick = btnReleaseClick
    end
    object chbCanFD: TCheckBox
      Left = 602
      Top = 37
      Width = 63
      Height = 17
      Caption = 'CAN-FD'
      TabOrder = 9
      OnClick = chbCanFDClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 8
    Top = 71
    Width = 744
    Height = 58
    Caption = ' Configuration Parameters '
    TabOrder = 1
    object Label8: TLabel
      Left = 11
      Top = 13
      Width = 54
      Height = 13
      Caption = 'Parameter:'
    end
    object Label9: TLabel
      Left = 240
      Top = 13
      Width = 52
      Height = 13
      Caption = 'Activation:'
    end
    object laDeviceOrDelay: TLabel
      Left = 409
      Top = 11
      Width = 80
      Height = 13
      Caption = 'Device ID (Hex):'
    end
    object btnParameterSet: TButton
      Left = 601
      Top = 25
      Width = 65
      Height = 25
      Caption = 'Set'
      TabOrder = 0
      OnClick = btnParameterSetClick
    end
    object btnParameterGet: TButton
      Left = 672
      Top = 25
      Width = 65
      Height = 25
      Caption = 'Get'
      TabOrder = 1
      OnClick = btnParameterGetClick
    end
    object cbbParameter: TComboBox
      Left = 11
      Top = 29
      Width = 217
      Height = 21
      Style = csDropDownList
      DropDownCount = 12
      TabOrder = 2
      OnChange = cbbParameterChange
      Items.Strings = (
        'Device ID'
        '5V Power'
        'Auto-reset on BUS-OFF'
        'CAN Listen-Only'
        'Debug'#39's Log'
        'Receive Status'
        'CAN Controller Number'
        'Trace File'
        'Channel Identification (USB)'
        'Channel Capabilities'
        'Bit rate Adaptation'
        'Get Bit rate Information'
        'Get Bit rate FD Information'
        'Get CAN Nominal Speed Bit/s'
        'Get CAN Data Speed Bit/s'
        'Get IP Address'
        'Get LAN Service Status'
        'Reception of Status Frames'
        'Reception of RTR Frames'
        'Reception of Error Frames'
        'Interframe Transmit Delay'
        'Reception of Echo Frames'
        'Hard Reset Status')
    end
    object rdbParamActive: TRadioButton
      Left = 234
      Top = 32
      Width = 49
      Height = 17
      Caption = 'Active'
      Checked = True
      TabOrder = 3
      TabStop = True
    end
    object rdbParamInactive: TRadioButton
      Left = 298
      Top = 32
      Width = 71
      Height = 17
      Caption = 'Inactive'
      TabOrder = 4
    end
    object txtDeviceIdOrDelay: TEdit
      Left = 408
      Top = 30
      Width = 104
      Height = 21
      MaxLength = 10
      TabOrder = 5
      Text = '0'
      OnExit = txtDeviceIdOrDelayExit
      OnKeyPress = txtDeviceIdOrDelayKeyPress
    end
  end
  object GroupBox4: TGroupBox
    Left = 8
    Top = 227
    Width = 744
    Height = 130
    Caption = 'Raw CAN Data'
    TabOrder = 2
    object chbShowPeriod: TCheckBox
      Left = 601
      Top = 68
      Width = 127
      Height = 17
      Caption = 'Timestamp as period'
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = chbShowPeriodClick
    end
    object btnRead: TButton
      Left = 601
      Top = 22
      Width = 65
      Height = 25
      Caption = 'Read'
      Enabled = False
      TabOrder = 1
      OnClick = btnReadClick
    end
    object btnMsgClear: TButton
      Left = 672
      Top = 22
      Width = 65
      Height = 25
      Caption = 'Clear'
      TabOrder = 2
      OnClick = btnMsgClearClick
    end
    object lstMessages: TListView
      Left = 11
      Top = 21
      Width = 584
      Height = 96
      Columns = <
        item
          Caption = 'Type'
          Width = 110
        end
        item
          Caption = 'ID'
          Width = 90
        end
        item
          Caption = 'Length'
        end
        item
          Caption = 'Count'
        end
        item
          Caption = 'Rcv Time'
          Width = 70
        end
        item
          Caption = 'Data'
          Width = 190
        end>
      RowSelect = True
      TabOrder = 3
      ViewStyle = vsReport
      OnDblClick = btnMsgClearClick
    end
  end
  object GroupBox6: TGroupBox
    Left = 8
    Top = 135
    Width = 744
    Height = 87
    Caption = 'Information'
    TabOrder = 3
    object lbxInfo: TListBox
      Left = 11
      Top = 19
      Width = 584
      Height = 56
      ItemHeight = 13
      Items.Strings = (
        
          'modbatt Diagnostics and Configuration Utility (C) Modular Batter' +
          'y Technologies 2024.'
        
          'Select and configure your CAN adapter, then click the "Connect" ' +
          'button.'
        
          'When activated, the Debug-Log file will be found in the same dir' +
          'ectory as this application'
        
          'When activated, the PCAN-Trace file will be found in the same di' +
          'rectory as this application')
      TabOrder = 0
      OnDblClick = btnInfoClearClick
    end
    object btnGetVersions: TButton
      Left = 601
      Top = 19
      Width = 65
      Height = 25
      Caption = 'Versions'
      Enabled = False
      TabOrder = 1
      OnClick = btnGetVersionsClick
    end
    object btnInfoClear: TButton
      Left = 672
      Top = 19
      Width = 65
      Height = 25
      Caption = 'Clear'
      TabOrder = 2
      OnClick = btnInfoClearClick
    end
    object btnReset: TButton
      Left = 672
      Top = 50
      Width = 65
      Height = 25
      Caption = 'Reset'
      Enabled = False
      TabOrder = 3
      OnClick = btnResetClick
    end
    object btnStatus: TButton
      Left = 602
      Top = 50
      Width = 65
      Height = 25
      Caption = 'Status'
      Enabled = False
      TabOrder = 4
      OnClick = btnStatusClick
    end
  end
  object grpPackData: TGroupBox
    Left = 8
    Top = 486
    Width = 744
    Height = 214
    Caption = 'Pack Data'
    TabOrder = 4
    object lbl0x410: TLabel
      Left = 10
      Top = 26
      Width = 37
      Height = 13
      Caption = '0x410 :'
    end
    object lbl0x421: TLabel
      Left = 10
      Top = 80
      Width = 37
      Height = 13
      Caption = '0x421 :'
    end
    object lbl0x422: TLabel
      Left = 10
      Top = 107
      Width = 34
      Height = 13
      Caption = '0x422:'
    end
    object lbl0x423: TLabel
      Left = 10
      Top = 134
      Width = 37
      Height = 13
      Caption = '0x423 :'
    end
    object lbl0x425: TLabel
      Left = 10
      Top = 161
      Width = 37
      Height = 13
      Caption = '0x425 :'
    end
    object lbl0x430: TLabel
      Left = 10
      Top = 188
      Width = 37
      Height = 13
      Caption = '0x430 :'
    end
    object Label3: TLabel
      Left = 60
      Top = 80
      Width = 36
      Height = 13
      Caption = 'Voltage'
    end
    object Label4: TLabel
      Left = 230
      Top = 80
      Width = 37
      Height = 13
      Caption = 'Current'
    end
    object Label5: TLabel
      Left = 570
      Top = 107
      Width = 21
      Height = 13
      Caption = 'SOC'
    end
    object Label10: TLabel
      Left = 60
      Top = 107
      Width = 50
      Height = 13
      Caption = 'Hi Cell Volt'
    end
    object Label14: TLabel
      Left = 230
      Top = 107
      Width = 52
      Height = 13
      Caption = 'Lo Cell Volt'
    end
    object Label16: TLabel
      Left = 400
      Top = 107
      Width = 60
      Height = 13
      Caption = 'Avg Cell Volt'
    end
    object Label17: TLabel
      Left = 60
      Top = 134
      Width = 58
      Height = 13
      Caption = 'Hi Cell Temp'
    end
    object Label18: TLabel
      Left = 230
      Top = 134
      Width = 60
      Height = 13
      Caption = 'Lo Cell Temp'
    end
    object Label19: TLabel
      Left = 400
      Top = 134
      Width = 68
      Height = 13
      Caption = 'Avg Cell Temp'
    end
    object Label20: TLabel
      Left = 60
      Top = 161
      Width = 59
      Height = 13
      Caption = 'Charge Limit'
    end
    object Label21: TLabel
      Left = 230
      Top = 161
      Width = 71
      Height = 13
      Caption = 'Discharge Limit'
    end
    object Label22: TLabel
      Left = 400
      Top = 161
      Width = 63
      Height = 13
      Caption = 'End Volt Limit'
    end
    object Label23: TLabel
      Left = 60
      Top = 26
      Width = 26
      Height = 13
      Caption = 'State'
    end
    object Label24: TLabel
      Left = 230
      Top = 26
      Width = 31
      Height = 13
      Caption = 'Status'
    end
    object Label25: TLabel
      Left = 570
      Top = 26
      Width = 21
      Height = 13
      Caption = 'SOH'
    end
    object lblModules: TLabel
      Left = 60
      Top = 53
      Width = 39
      Height = 13
      Caption = 'Modules'
    end
    object Label27: TLabel
      Left = 400
      Top = 53
      Width = 24
      Height = 13
      Caption = 'Fault'
    end
    object lblActiveModules: TLabel
      Left = 230
      Top = 53
      Width = 72
      Height = 13
      Caption = 'Active Modules'
    end
    object Label30: TLabel
      Left = 60
      Top = 188
      Width = 41
      Height = 13
      Caption = 'Isolation'
    end
    object editSoh: TEdit
      Left = 650
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 0
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editVoltage: TEdit
      Left = 140
      Top = 76
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 1
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editSoc: TEdit
      Left = 650
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 2
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editCurrent: TEdit
      Left = 310
      Top = 76
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 3
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editHiCellVolt: TEdit
      Left = 140
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 4
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editLoCellVolt: TEdit
      Left = 310
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 5
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editAvgCellVolt: TEdit
      Left = 480
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 6
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editHiCellTemp: TEdit
      Left = 140
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 7
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editLoCellTemp: TEdit
      Left = 310
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 8
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editAvgCellTemp: TEdit
      Left = 480
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 9
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editChgLimit: TEdit
      Left = 140
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 10
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editDisChgLimit: TEdit
      Left = 310
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 11
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editEndVoltLimit: TEdit
      Left = 480
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 12
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editState: TEdit
      Left = 140
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 13
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editStatus: TEdit
      Left = 310
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 14
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editTotalModules: TEdit
      Left = 140
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 15
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editFault: TEdit
      Left = 480
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 16
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editActiveModules: TEdit
      Left = 310
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 17
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editIsolation: TEdit
      Left = 140
      Top = 184
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 18
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
  end
  object grpState: TGroupBox
    Left = 8
    Top = 418
    Width = 744
    Height = 62
    Caption = 'Pack State Control'
    TabOrder = 5
    object Label28: TLabel
      Left = 11
      Top = 15
      Width = 65
      Height = 13
      Caption = 'Select State :'
    end
    object Label35: TLabel
      Left = 650
      Top = 15
      Width = 86
      Height = 13
      Caption = 'Inverter Voltage :'
    end
    object chkSendState: TCheckBox
      Left = 310
      Top = 35
      Width = 150
      Height = 17
      Caption = 'Send State Automatically'
      Enabled = False
      TabOrder = 0
      OnClick = chkSendStateClick
    end
    object editInverter: TEdit
      Left = 650
      Top = 33
      Width = 75
      Height = 21
      Enabled = False
      MaxLength = 3
      TabOrder = 1
      Text = '400'
      OnKeyPress = txtIdFromKeyPress
    end
    object cboState: TComboBox
      Left = 10
      Top = 33
      Width = 120
      Height = 21
      Style = csDropDownList
      DropDownCount = 12
      Enabled = False
      ItemIndex = 0
      TabOrder = 2
      Text = 'Off'
      OnChange = cboStateChange
      Items.Strings = (
        'Off'
        'StandBy'
        'PreCharge'
        'On')
    end
    object editSelectedState: TEdit
      Left = 650
      Top = 33
      Width = 75
      Height = 21
      Enabled = False
      MaxLength = 3
      TabOrder = 3
      Text = 'Off'
      TextHint = '0'
      Visible = False
      OnKeyPress = txtIdFromKeyPress
    end
    object btnSendState: TButton
      Left = 141
      Top = 31
      Width = 65
      Height = 25
      Caption = 'Send State'
      TabOrder = 4
      OnClick = btnSendStateClick
    end
  end
  object grpPackParam: TGroupBox
    Left = 8
    Top = 706
    Width = 744
    Height = 58
    Caption = 'Pack Configuration Parameters '
    TabOrder = 6
    object Label36: TLabel
      Left = 11
      Top = 13
      Width = 54
      Height = 13
      Caption = 'Parameter:'
    end
    object Label38: TLabel
      Left = 409
      Top = 11
      Width = 63
      Height = 13
      Caption = 'Value (Hex) :'
    end
    object cboPackParam: TComboBox
      Left = 11
      Top = 29
      Width = 250
      Height = 21
      Style = csDropDownList
      DropDownCount = 12
      TabOrder = 0
      Items.Strings = (
        'Pack Controller ID')
    end
    object edtPackValue: TEdit
      Left = 408
      Top = 30
      Width = 104
      Height = 21
      MaxLength = 10
      TabOrder = 1
      Text = '0'
      OnExit = txtDeviceIdOrDelayExit
      OnKeyPress = txtDeviceIdOrDelayKeyPress
    end
    object btnPackParamGet: TButton
      Left = 672
      Top = 25
      Width = 65
      Height = 25
      Caption = 'Get'
      TabOrder = 2
    end
    object btnPackParamSet: TButton
      Left = 601
      Top = 25
      Width = 65
      Height = 25
      Caption = 'Set'
      TabOrder = 3
      OnClick = btnPackParamSetClick
    end
  end
  object grpPackSelect: TGroupBox
    Left = 8
    Top = 363
    Width = 743
    Height = 49
    Caption = 'Pack/Module Selection'
    TabOrder = 7
    object Label37: TLabel
      Left = 11
      Top = 23
      Width = 40
      Height = 13
      Caption = 'Pack ID:'
    end
    object lblCANbase: TLabel
      Left = 136
      Top = 23
      Width = 106
      Height = 13
      Caption = '(Address base 0x400)'
    end
    object Label2: TLabel
      Left = 601
      Top = 23
      Width = 52
      Height = 13
      Caption = 'Module ID:'
    end
    object cboPackID: TComboBox
      Left = 70
      Top = 20
      Width = 60
      Height = 21
      TabOrder = 0
      Text = '0'
      OnChange = cboPackIDChange
      Items.Strings = (
        '0'
        '1')
    end
    object cboModuleId: TComboBox
      Left = 665
      Top = 20
      Width = 60
      Height = 21
      Enabled = False
      TabOrder = 1
      Text = '1'
      OnChange = cboModuleIdChange
      Items.Strings = (
        '1'
        '2'
        '3'
        '4'
        '5'
        '6'
        '7'
        '8'
        '9'
        '10'
        '11'
        '12'
        '13'
        '14'
        '15'
        '16'
        '17'
        '18'
        '19'
        '20'
        '21'
        '22'
        '23'
        '24'
        '25'
        '26'
        '27'
        '28'
        '29'
        '30')
    end
    object chkDMC: TCheckBox
      Left = 310
      Top = 22
      Width = 165
      Height = 17
      Caption = 'Enable Direct Module Control'
      TabOrder = 2
      OnClick = chkDMCClick
    end
  end
  object grpModuleData: TGroupBox
    Left = 8
    Top = 486
    Width = 744
    Height = 214
    Caption = 'Module Data for Module #1'
    TabOrder = 8
    Visible = False
    object Label6: TLabel
      Left = 10
      Top = 26
      Width = 37
      Height = 13
      Caption = '0x411 :'
    end
    object Label7: TLabel
      Left = 10
      Top = 80
      Width = 37
      Height = 13
      Caption = '0x412 :'
    end
    object Label11: TLabel
      Left = 10
      Top = 107
      Width = 34
      Height = 13
      Caption = '0x413:'
    end
    object Label12: TLabel
      Left = 10
      Top = 134
      Width = 37
      Height = 13
      Caption = '0x414 :'
    end
    object Label13: TLabel
      Left = 10
      Top = 161
      Width = 37
      Height = 13
      Caption = '0x416 :'
    end
    object Label15: TLabel
      Left = 10
      Top = 188
      Width = 37
      Height = 13
      Caption = '0x417 :'
    end
    object Label26: TLabel
      Left = 60
      Top = 80
      Width = 36
      Height = 13
      Caption = 'Voltage'
    end
    object Label29: TLabel
      Left = 230
      Top = 80
      Width = 37
      Height = 13
      Caption = 'Current'
    end
    object Label31: TLabel
      Left = 400
      Top = 26
      Width = 21
      Height = 13
      Caption = 'SOC'
    end
    object Label32: TLabel
      Left = 60
      Top = 107
      Width = 50
      Height = 13
      Caption = 'Hi Cell Volt'
    end
    object Label33: TLabel
      Left = 230
      Top = 107
      Width = 52
      Height = 13
      Caption = 'Lo Cell Volt'
    end
    object Label34: TLabel
      Left = 400
      Top = 107
      Width = 60
      Height = 13
      Caption = 'Avg Cell Volt'
    end
    object Label39: TLabel
      Left = 60
      Top = 134
      Width = 58
      Height = 13
      Caption = 'Hi Cell Temp'
    end
    object Label40: TLabel
      Left = 230
      Top = 134
      Width = 60
      Height = 13
      Caption = 'Lo Cell Temp'
    end
    object Label41: TLabel
      Left = 400
      Top = 134
      Width = 68
      Height = 13
      Caption = 'Avg Cell Temp'
    end
    object Label42: TLabel
      Left = 60
      Top = 161
      Width = 59
      Height = 13
      Caption = 'Charge Limit'
    end
    object Label43: TLabel
      Left = 230
      Top = 161
      Width = 71
      Height = 13
      Caption = 'Discharge Limit'
    end
    object Label44: TLabel
      Left = 400
      Top = 161
      Width = 63
      Height = 13
      Caption = 'End Volt Limit'
    end
    object Label45: TLabel
      Left = 60
      Top = 26
      Width = 26
      Height = 13
      Caption = 'State'
    end
    object Label46: TLabel
      Left = 230
      Top = 26
      Width = 31
      Height = 13
      Caption = 'Status'
    end
    object Label47: TLabel
      Left = 570
      Top = 26
      Width = 21
      Height = 13
      Caption = 'SOH'
    end
    object Label48: TLabel
      Left = 400
      Top = 52
      Width = 49
      Height = 13
      Caption = 'Cell Count'
    end
    object Label49: TLabel
      Left = 230
      Top = 53
      Width = 70
      Height = 13
      Caption = 'Balance Active'
    end
    object Label50: TLabel
      Left = 60
      Top = 53
      Width = 71
      Height = 13
      Caption = 'Balance Status'
    end
    object Label51: TLabel
      Left = 60
      Top = 188
      Width = 41
      Height = 13
      Caption = 'Isolation'
    end
    object Label54: TLabel
      Left = 591
      Top = 81
      Width = 61
      Height = 13
      Caption = 'Comms Error'
    end
    object Label52: TLabel
      Left = 571
      Top = 53
      Width = 64
      Height = 13
      Caption = 'Fault Codes :'
    end
    object Label55: TLabel
      Left = 591
      Top = 100
      Width = 111
      Height = 13
      Caption = 'Hardware Incompatible'
    end
    object Label56: TLabel
      Left = 591
      Top = 120
      Width = 64
      Height = 13
      Caption = 'Over Current'
    end
    object Label57: TLabel
      Left = 591
      Top = 140
      Width = 89
      Height = 13
      Caption = 'Over Temperature'
    end
    object Label58: TLabel
      Left = 591
      Top = 160
      Width = 63
      Height = 13
      Caption = 'Over Voltage'
    end
    object editModuleSoh: TEdit
      Left = 650
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 0
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleVoltage: TEdit
      Left = 140
      Top = 76
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 1
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleSoc: TEdit
      Left = 479
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 2
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleCurrent: TEdit
      Left = 310
      Top = 76
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 3
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleHiCellVolt: TEdit
      Left = 140
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 4
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleLoCellVolt: TEdit
      Left = 310
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 5
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object Edit7: TEdit
      Left = -760
      Top = 279
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 6
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleHiCellTemp: TEdit
      Left = 140
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 7
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleLoCellTemp: TEdit
      Left = 310
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 8
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleAvgCellTemp: TEdit
      Left = 480
      Top = 130
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 9
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleChargeLimit: TEdit
      Left = 140
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 10
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleDischargeLimit: TEdit
      Left = 310
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 11
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object Edit13: TEdit
      Left = -800
      Top = 349
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 12
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleState: TEdit
      Left = 140
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 13
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleStatus: TEdit
      Left = 310
      Top = 22
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 14
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleCellCount: TEdit
      Left = 479
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 15
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleBalanceActive: TEdit
      Left = 310
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 16
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleBalanceStatus: TEdit
      Left = 140
      Top = 49
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 17
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleIsolation: TEdit
      Left = 140
      Top = 184
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 18
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleEndVoltage: TEdit
      Left = 480
      Top = 157
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 19
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleAvgCellVolt: TEdit
      Left = 480
      Top = 103
      Width = 75
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 20
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleFault0: TEdit
      Left = 712
      Top = 78
      Width = 13
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 21
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleFault1: TEdit
      Left = 712
      Top = 97
      Width = 13
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 22
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleFault2: TEdit
      Left = 712
      Top = 117
      Width = 13
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 23
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleFault3: TEdit
      Left = 712
      Top = 137
      Width = 13
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 24
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
    object editModuleFault4: TEdit
      Left = 712
      Top = 157
      Width = 13
      Height = 21
      Alignment = taRightJustify
      Enabled = False
      MaxLength = 2
      TabOrder = 25
      Text = '0'
      OnExit = txtData0Exit
      OnKeyPress = txtIdFromKeyPress
    end
  end
  object tmrDisplay: TTimer
    Enabled = False
    Interval = 100
    OnTimer = tmrDisplayTimer
    Left = 704
    Top = 312
  end
  object tmrState: TTimer
    Enabled = False
    Interval = 250
    OnTimer = tmrStateTimer
    Left = 720
    Top = 448
  end
end
