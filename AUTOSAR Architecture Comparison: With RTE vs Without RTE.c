/*
 * AUTOSAR WITHOUT RTE: DIRECT API CALLS TO SERVICE LAYER
 * ======================================================
 * Door Switch (ECU A) → Interior Dimmer (ECU B) WITHOUT RTE LAYER
 * 
 * ARCHITECTURE WITHOUT RTE:
 * ┌─────────────────────────────────────────────────────────────────────┐
 * │ APPLICATION LAYER (ASW)                                             │
 * │ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐        │
 * │ │ DoorControl App │ │ SensorControl   │ │ LightControl    │        │
 * │ │ DIRECT CALLS    │ │ DIRECT CALLS    │ │ DIRECT CALLS    │        │
 * │ └─────────────────┘ └─────────────────┘ └─────────────────┘        │
 * │            │                   │                   │                │
 * │            └───────────────────┼───────────────────┘                │
 * │                                │ DIRECT API CALLS                   │
 * │                                ▼ (NO RTE ABSTRACTION)               │
 * ├─────────────────────────────────────────────────────────────────────┤
 * │ ❌ NO RUNTIME ENVIRONMENT (RTE) ❌                                   │
 * │ • No standardized interfaces                                        │
 * │ • No data transformation                                            │
 * │ • No inter-runnable communication                                   │
 * │ • No mode management                                                │
 * ├─────────────────────────────────────────────────────────────────────┤
 * │ SERVICE LAYER (BSW SERVICES) - DIRECT ACCESS                       │
 * │ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐        │
 * │ │   COM   │ │  PduR   │ │   DEM   │ │   DCM   │ │   NvM   │        │
 * │ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘        │
 * └─────────────────────────────────────────────────────────────────────┘
 */

/* ========================================================================
 * ECU A (BCM) - DOOR SWITCH WITHOUT RTE LAYER
 * ======================================================================== */

/* APPLICATION LAYER - DIRECT BSW CALLS (NO RTE) */
// File: DoorControl_NoRte.c
#include "Com.h"              // Direct COM API inclusion
#include "Dem.h"              // Direct DEM API inclusion  
#include "NvM.h"              // Direct NvM API inclusion
#include "IoHwAb.h"           // Direct IoHwAb API inclusion
#include "Os.h"               // Direct OS API inclusion

// PROBLEM 1: Application must know all BSW module details
#define COM_SIGNAL_DOOR_STATUS_ID    0u    // Magic numbers in application
#define NVM_BLOCK_DOOR_CONFIG_ID     1u    // Application knows NvM internals
#define DEM_EVENT_DOOR_SENSOR_FAIL   2u    // Application knows DEM internals

// PROBLEM 2: No standardized data types - using BSW types directly
Com_SignalIdType door_signal_id = COM_SIGNAL_DOOR_STATUS_ID;
NvM_BlockIdType door_config_block = NVM_BLOCK_DOOR_CONFIG_ID;

// PROBLEM 3: Global variables for communication (no RTE ports)
static boolean g_door_status_current = FALSE;
static boolean g_door_status_previous = FALSE;
static uint8 g_door_debounce_counter = 0;

void DoorControl_NoRte_MainFunction(void) {
    // Step 1: Direct IoHwAb call (should be abstracted by RTE)
    IoHwAb_BoolType raw_door_switch = FALSE;
    Std_ReturnType ioResult = IoHwAb_Digital_Read(IOHWAB_DOOR_SWITCH_CHANNEL, &raw_door_switch);
    
    // PROBLEM 4: Application handles low-level error codes directly
    if (ioResult != E_OK) {
        // Application must handle IoHwAb errors directly
        Dem_ReportErrorStatus(DEM_EVENT_DOOR_SENSOR_FAIL, DEM_EVENT_STATUS_FAILED);
        return; // Early return - no standardized error handling
    }
    
    // Step 2: Manual data conversion (RTE would handle this)
    boolean door_switch_conditioned = (raw_door_switch == IOHWAB_LOW) ? TRUE : FALSE;
    
    // Step 3: Application-level debouncing (should be in dedicated SWC)
    if (door_switch_conditioned == g_door_status_previous) {
        g_door_debounce_counter++;
    } else {
        g_door_debounce_counter = 0;
    }
    
    if (g_door_debounce_counter >= 5) {
        g_door_status_current = door_switch_conditioned;
        g_door_status_previous = door_switch_conditioned;
        
        // Step 4: Direct COM API call (NO RTE abstraction)
        // PROBLEM 5: Application must know COM signal format
        uint8 signal_data = (g_door_status_current == TRUE) ? 1u : 0u;
        
        // PROBLEM 6: Direct BSW API call from application
        Std_ReturnType comResult = Com_SendSignal(door_signal_id, &signal_data);
        
        // PROBLEM 7: Application handles COM-specific errors
        if (comResult != E_OK) {
            // No standardized error handling
            switch (comResult) {
                case COM_SERVICE_NOT_AVAILABLE:
                    // Application knows COM internals
                    Dem_ReportErrorStatus(DEM_EVENT_COM_NOT_AVAILABLE, DEM_EVENT_STATUS_FAILED);
                    break;
                case COM_BUSY:
                    // Application handles COM timing issues
                    // Retry logic needed here
                    break;
                default:
                    // Generic error handling
                    break;
            }
        }
        
        // Step 5: Direct NvM call for storing door events (NO RTE)
        // PROBLEM 8: Application manages NvM directly
        static uint32 door_event_counter = 0;
        door_event_counter++;
        
        Std_ReturnType nvmResult = NvM_WriteBlock(door_config_block, &door_event_counter);
        
        // PROBLEM 9: Application handles NvM queue management
        if (nvmResult == NVM_REQ_PENDING) {
            // Application must track NvM operations
            // No abstraction for async operations
        }
    }
}

// PROBLEM 10: Application must implement OS integration directly
void DoorControl_NoRte_Init(void) {
    // Direct OS calls from application
    AlarmBaseType alarm_base;
    GetAlarmBase(ALARM_DOOR_CONTROL_10MS, &alarm_base);
    
    // Application manages OS scheduling
    SetRelAlarm(ALARM_DOOR_CONTROL_10MS, alarm_base.mincycle, alarm_base.mincycle);
    
    // Direct BSW initialization calls
    Com_IpduGroupStart(COM_IPDU_GROUP_DOOR_MESSAGES, TRUE);
    
    // PROBLEM 11: No mode management abstraction
    // Application must handle mode transitions directly
}

/* ADDITIONAL APPLICATION FUNCTIONS WITH DIRECT BSW CALLS */

void DoorControl_NoRte_DiagnosticHandler(void) {
    // PROBLEM 12: Application handles diagnostics directly
    Dem_EventStatusType event_status;
    Dem_GetEventStatus(DEM_EVENT_DOOR_SENSOR_FAIL, &event_status);
    
    if (event_status == DEM_EVENT_STATUS_FAILED) {
        // Application must implement diagnostic responses
        // No RTE abstraction for diagnostic communication
        
        // Direct DCM interaction
        Dcm_NegativeResponseCodeType nrc = DCM_E_CONDITIONSNOTCORRECT;
        // Application handles UDS responses directly
    }
}

void DoorControl_NoRte_CalibrationAccess(void) {
    // PROBLEM 13: No calibration abstraction
    // Application must access calibration data directly
    
    uint16* calibration_ptr = (uint16*)0x80010000;  // Hardcoded address
    uint16 debounce_threshold = *calibration_ptr;   // Direct memory access
    
    // No RTE parameter interface
    // No tool support for calibration
}

/* ========================================================================
 * ECU B (ICM) - INTERIOR DIMMER WITHOUT RTE LAYER
 * ======================================================================== */

/* APPLICATION LAYER - DIRECT BSW CALLS (NO RTE) */
// File: LightControl_NoRte.c
#include "Com.h"              // Direct COM inclusion
#include "Dem.h"              // Direct DEM inclusion
#include "IoHwAb.h"           // Direct IoHwAb inclusion

// PROBLEM 14: Global variables for inter-function communication
static boolean g_received_door_status = FALSE;
static uint16 g_current_dimmer_level = 0;
static uint16 g_target_dimmer_level = 0;

void LightControl_NoRte_MainFunction(void) {
    // Step 1: Direct COM reception (NO RTE abstraction)
    // PROBLEM 15: Application must know COM signal details
    uint8 received_signal_data = 0;
    Std_ReturnType comResult = Com_ReceiveSignal(COM_SIGNAL_DOOR_STATUS_ID, &received_signal_data);
    
    // PROBLEM 16: Application handles COM reception details
    if (comResult == E_OK) {
        g_received_door_status = (received_signal_data == 1u) ? TRUE : FALSE;
    } else if (comResult == COM_SERVICE_NOT_AVAILABLE) {
        // Application handles timeout conditions
        // No standardized timeout handling from RTE
        g_received_door_status = FALSE;  // Default value
        
        // Report timeout to DEM directly
        Dem_ReportErrorStatus(DEM_EVENT_CAN_TIMEOUT, DEM_EVENT_STATUS_FAILED);
    }
    
    // Step 2: Business logic implementation
    if (g_received_door_status == TRUE) {
        g_target_dimmer_level = 1000;  // Full brightness
    } else {
        g_target_dimmer_level = 0;     // Off
    }
    
    // Step 3: Fade effect implementation
    if (g_current_dimmer_level != g_target_dimmer_level) {
        if (g_current_dimmer_level < g_target_dimmer_level) {
            g_current_dimmer_level += 10;  // Fade in
            if (g_current_dimmer_level > g_target_dimmer_level) {
                g_current_dimmer_level = g_target_dimmer_level;
            }
        } else {
            g_current_dimmer_level -= 10;  // Fade out
            if (g_current_dimmer_level < g_target_dimmer_level) {
                g_current_dimmer_level = g_target_dimmer_level;
            }
        }
        
        // Step 4: Direct IoHwAb call for PWM output (NO RTE)
        // PROBLEM 17: Application must know IoHwAb channel details
        Std_ReturnType ioResult = IoHwAb_Analog_Write(IOHWAB_DIMMER_PWM_CHANNEL, g_current_dimmer_level);
        
        // PROBLEM 18: Application handles IoHwAb errors directly
        if (ioResult != E_OK) {
            // No standardized error propagation from RTE
            Dem_ReportErrorStatus(DEM_EVENT_DIMMER_ACTUATOR_FAIL, DEM_EVENT_STATUS_FAILED);
        }
    }
}

/* ========================================================================
 * PROBLEMS AND DISADVANTAGES OF NO RTE APPROACH
 * ======================================================================== */

/*
 * DISADVANTAGE 1: TIGHT COUPLING BETWEEN APPLICATION AND BSW
 * ===========================================================
 */

// Application must include all BSW headers directly
#include "Com.h"
#include "PduR.h"
#include "CanIf.h"
#include "Dem.h"
#include "Dcm.h"
#include "NvM.h"
#include "Fee.h"
#include "Fls.h"
#include "BswM.h"
#include "ComM.h"
#include "EcuM.h"
#include "Os.h"
#include "IoHwAb.h"

// PROBLEM: Application becomes dependent on ALL BSW modules
// Any BSW interface change requires application modification

/*
 * DISADVANTAGE 2: NO STANDARDIZED DATA TYPES
 * ===========================================
 */

void NoRte_DataTypeProblems(void) {
    // Applications must use BSW-specific data types
    Com_SignalIdType signal_id;           // COM specific type
    NvM_BlockIdType block_id;            // NvM specific type
    Dem_EventIdType event_id;            // DEM specific type
    
    // No application-level standardized types
    // Different suppliers may use different type definitions
    // Portability issues between different BSW implementations
}

/*
 * DISADVANTAGE 3: NO INTER-RUNNABLE COMMUNICATION
 * ===============================================
 */

// PROBLEM: Global variables needed for communication
static boolean g_door_sensor_status;     // Global state
static uint16 g_engine_rpm;             // Global state
static uint8 g_transmission_gear;       // Global state

void Runnable1_NoRte(void) {
    // No RTE ports for communication
    g_door_sensor_status = TRUE;  // Write to global
}

void Runnable2_NoRte(void) {
    // No RTE ports for communication
    if (g_door_sensor_status == TRUE) {  // Read from global
        // Process door status
    }
}

// PROBLEM: Race conditions, no data consistency guarantees
// PROBLEM: No standardized communication mechanism

/*
 * DISADVANTAGE 4: NO MODE MANAGEMENT
 * ==================================
 */

void NoRte_ModeManagement(void) {
    // PROBLEM: Application must manage modes directly
    
    // No RTE mode switch events
    // Application must handle mode transitions manually
    
    if (/* some condition */) {
        // Direct BswM call
        BswM_RequestMode(BSWM_USER_APPLICATION, BSWM_MODE_SLEEP);
        
        // Application must coordinate with multiple BSW modules
        ComM_RequestComMode(COMM_USER_APP, COMM_NO_COMMUNICATION);
        EcuM_RequestRUN(ECUM_USER_APP);
        
        // No standardized mode management abstraction
    }
}

/*
 * DISADVANTAGE 5: NO DATA TRANSFORMATION
 * ======================================
 */

void NoRte_DataTransformation(void) {
    // PROBLEM: Application must handle all data conversions
    
    // Read sensor value (physical units)
    uint16 adc_raw_value = 0;
    Adc_ReadGroup(ADC_GROUP_SENSORS, &adc_raw_value);
    
    // Application must do physical conversion
    float32 physical_value = (float32)adc_raw_value * 0.00488f;  // Manual scaling
    
    // Application must do signal packing for COM
    uint16 signal_value = (uint16)(physical_value * 100.0f);     // Manual packing
    
    // No RTE data transformation services
    // No automatic scaling, offset, or unit conversion
}

/*
 * DISADVANTAGE 6: NO ERROR ABSTRACTION
 * ====================================
 */

void NoRte_ErrorHandling(void) {
    // PROBLEM: Application must handle BSW-specific errors
    
    Std_ReturnType com_result = Com_SendSignal(0, NULL);
    
    // Application must know COM-specific error codes
    switch (com_result) {
        case COM_SERVICE_NOT_AVAILABLE:
            // Handle COM-specific error
            break;
        case COM_BUSY:
            // Handle COM-specific error
            break;
        case E_NOT_OK:
            // Generic error - no context
            break;
    }
    
    // No standardized error handling across BSW modules
    // Different modules return different error codes
    // No error abstraction or translation
}

/*
 * DISADVANTAGE 7: NO MEASUREMENT AND CALIBRATION SUPPORT
 * ======================================================
 */

// PROBLEM: No standardized calibration interface
uint16 NoRte_CalibrationParameters[10];  // Manual parameter array

void NoRte_CalibrationAccess(void) {
    // No RTE parameter interface
    // No tool support for online calibration
    // No automatic parameter validation
    
    // Manual memory access for calibration
    uint16* cal_data = (uint16*)0x80000000;  // Hardcoded address
    uint16 threshold = cal_data[5];           // Manual indexing
    
    // No RTE measurement points
    // No standardized measurement interface for tools
}

/*
 * DISADVANTAGE 8: NO TIMING AND SCHEDULING ABSTRACTION
 * ====================================================
 */

void NoRte_SchedulingProblems(void) {
    // PROBLEM: Application must handle OS directly
    
    // No RTE runnable scheduling
    ActivateTask(TASK_DOOR_CONTROL);         // Direct OS call
    SetEvent(TASK_LIGHT_CONTROL, EVENT_1);  // Direct OS call
    
    // No timing protection
    // No inter-runnable timing coordination
    // No standardized scheduling interface
}

/*
 * DISADVANTAGE 9: NO STANDARDIZED COMMUNICATION INTERFACE
 * =======================================================
 */

void NoRte_CommunicationProblems(void) {
    // PROBLEM: Application must know communication details
    
    // Direct COM calls require knowledge of:
    Com_SendSignal(0, NULL);    // Signal ID mapping
    Com_ReceiveSignal(1, NULL); // Signal layout
    Com_TriggerIPDUSend(2);     // I-PDU structure
    
    // No communication abstraction
    // No automatic signal routing
    // No standardized communication interface
}

/*
 * DISADVANTAGE 10: POOR TESTABILITY
 * =================================
 */

void NoRte_TestabilityProblems(void) {
    // PROBLEM: Cannot test application independently
    
    // Application directly calls BSW modules
    Com_SendSignal(0, NULL);           // Cannot stub easily
    Dem_ReportErrorStatus(1, 2);       // Cannot mock easily
    IoHwAb_Digital_Read(3, NULL);      // Cannot simulate easily
    
    // No RTE interface for test abstraction
    // No standardized test interfaces
    // Difficult to create unit tests
    // Integration testing becomes complex
}

/*
 * DISADVANTAGE 11: NO SUPPLIER INDEPENDENCE
 * =========================================
 */

void NoRte_SupplierDependency(void) {
    // PROBLEM: Application tied to specific BSW implementation
    
    // Different suppliers may have different APIs
    // Supplier A: Com_SendSignal()
    // Supplier B: Com_TransmitSignal()
    // Supplier C: ComSendSignal()
    
    // No standardized interface means:
    // - Cannot switch BSW suppliers easily
    // - Application code not portable
    // - Integration effort for each supplier
}

/*
 * DISADVANTAGE 12: NO DEVELOPMENT TOOL SUPPORT
 * ============================================
 */

void NoRte_ToolSupport(void) {
    // PROBLEM: No standardized development environment
    
    // RTE provides:
    // - Automatic interface generation
    // - Configuration consistency checking
    // - Communication matrix validation
    // - Measurement/calibration tool integration
    
    // Without RTE:
    // - Manual interface management
    // - No automatic validation
    // - No tool integration
    // - Manual configuration management
}

/*
 * SUMMARY OF DISADVANTAGES WITHOUT RTE:
 * =====================================
 * 
 * 1. TIGHT COUPLING: Application directly coupled to BSW modules
 * 2. NO STANDARDIZATION: No standard data types or interfaces  
 * 3. COMPLEX COMMUNICATION: No inter-runnable communication mechanism
 * 4. NO MODE MANAGEMENT: No standardized mode handling
 * 5. MANUAL DATA HANDLING: No automatic data transformation
 * 6. POOR ERROR HANDLING: No error abstraction across BSW modules
 * 7. NO CALIBRATION SUPPORT: No measurement/calibration abstraction
 * 8. COMPLEX SCHEDULING: No timing and scheduling abstraction
 * 9. COMMUNICATION COMPLEXITY: No communication interface abstraction
 * 10. POOR TESTABILITY: Cannot test application independently
 * 11. SUPPLIER DEPENDENCY: Application tied to specific BSW implementations
 * 12. NO TOOL SUPPORT: No development tool integration
 * 
 * IMPACT ON YOUR OEM:
 * ===================
 * 
 * • SUPPLIER INTEGRATION: Each supplier requires different integration effort
 * • MAINTENANCE: Changes to BSW affect application code directly
 * • TESTING: Cannot validate application independently of hardware
 * • PORTABILITY: Application code cannot be reused across platforms
 * • DEVELOPMENT TIME: Manual interface management increases development effort
 * • QUALITY: No systematic validation of interfaces and communication
 * • TOOL SUPPORT: Cannot use standard AUTOSAR development tools
 * • SCALABILITY: Adding new functions requires extensive integration work
 * 
 * WHY RTE IS ESSENTIAL:
 * ====================
 * 
 * The RTE layer provides the crucial abstraction that enables:
 * - Standardized interfaces between Application and BSW
 * - Supplier independence and portability
 * - Systematic testing and validation
 * - Tool support and automatic code generation
 * - Reduced integration effort with suppliers
 * - Maintainable and scalable software architecture
 */
