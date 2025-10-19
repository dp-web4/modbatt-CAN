# PCAN-Basic API License Summary

**Investigation Date**: October 18, 2025
**Source**: PEAK-System official forum and documentation

---

## Summary

**PCAN-Basic API is freely redistributable with open-source software**, with appropriate disclosures.

---

## Official PEAK-System Statement

From PEAK-System Forum ([viewtopic.php?t=2087](https://forum.peak-system.com/viewtopic.php?t=2087)):

> For Windows, PEAK-System holds the copyright of the DLL and Header Files, and you can share the DLL (Binary) and the needed Header file with your own Software as often as you want, or link to the download package of the API.

---

## License Terms

### ✅ Permitted

1. **Free Use**: PCAN-Basic is completely free for both personal and commercial use
2. **No Activation**: No license keys or activation required
3. **Redistribution**: You may distribute the binary DLL and header files with your software
4. **Open Source Compatible**: Can be used in open-source projects under AGPLv3 or other licenses

### ⚠️ Restrictions

1. **Hardware Requirement**: The API must only be used with genuine PEAK-System CAN hardware
2. **Third-Party Hardware**: Running with non-PEAK hardware is forbidden

### 📋 Redistribution Requirements

When including PCAN-Basic in open-source software:

1. **Include DLL and Headers**: Redistribute `PCANBasic.dll` and necessary header files
2. **Copyright Notice**: Acknowledge PEAK-System copyright
3. **Hardware Disclosure**: Document that genuine PEAK-System hardware is required
4. **Download Link**: Optionally link to official PEAK-System download page

---

## Recommended Disclosure Text

For `modbatt-CAN` README or documentation:

```markdown
## PCAN Hardware Requirement

This software uses the PCAN-Basic API from PEAK-System Technik GmbH.

**Hardware Requirement**: Genuine PEAK-System CAN interface required (e.g., PCAN-USB, PCAN-PCIe)

**PCAN-Basic License**:
- DLL and headers © PEAK-System Technik GmbH
- Freely redistributable with this software
- For use only with genuine PEAK-System hardware
- Download: https://www.peak-system.com/PCAN-Basic.239.0.html

**Note**: PCAN-Basic components are copyrighted by PEAK-System but freely redistributable.
They are not covered by the AGPLv3 license of this project.
```

---

## Open-Source Precedents

Several open-source projects successfully use PCAN-Basic:

1. **CAN API V3 Wrapper**: Dual-licensed BSD-2-Clause / GPL-3.0
   - GitHub: https://github.com/uv-software/PCANBasic-Wrapper
   - Demonstrates GPL compatibility

2. **python-can**: MIT License
   - Uses PCAN-Basic for CAN interface support
   - Documentation: https://python-can.readthedocs.io/en/3.0.0/interfaces/pcan.html

---

## Conclusion for ModBatt

**Status**: ✅ **Approved for use in modbatt-CAN AGPL release**

**Rationale**:
- PEAK-System explicitly permits redistribution with software
- No license conflicts with AGPLv3
- Clear hardware requirement can be documented
- Precedent from other open-source projects

**Action Items**:
1. Include copyright notice for PCAN-Basic in modbatt-CAN README
2. Document hardware requirement prominently
3. Optionally link to PEAK-System download page
4. Include this summary in modbatt-CAN documentation directory

---

**Prepared by**: Claude (Web4 distributed consciousness network)
**Source**: Official PEAK-System forum (https://forum.peak-system.com/viewtopic.php?t=2087)
**Status**: Approved for AGPLv3 release with appropriate disclosures
