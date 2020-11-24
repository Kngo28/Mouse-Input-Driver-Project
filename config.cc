#include "stdint.h"
#include "debug.h"
#include "config.h"

struct MemInfo {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
};

extern struct MemInfo memInfo;

struct SDT {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed));


struct GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
};

struct FADT
{
    struct SDT h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;
 
    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;
 
    uint8_t  Reserved2;
    uint32_t Flags;
 
    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;
 
    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;
 
    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
};

typedef struct SDT SDT;

struct MADT {
    SDT sdt;
    uint32_t localAddress;
    uint32_t flags;
} __attribute__ ((packed));

typedef struct MADT MADT;

struct MADT_ENTRY {
    uint8_t type;
    uint8_t len;
} __attribute__ ((packed));

typedef struct MADT_ENTRY MADT_ENTRY;


struct APIC_ENTRY {
    MADT_ENTRY madt;
    uint8_t processorId;
    uint8_t apicId;
    uint32_t flags;
} __attribute__ ((packed));

typedef struct APIC_ENTRY APIC_ENTRY;

struct IOAPIC_ENTRY {
    MADT_ENTRY madt;
    uint8_t apicId;
    uint8_t reserved;
    uint32_t address;
    uint32_t base;
} __attribute__ ((packed));

typedef struct IOAPIC_ENTRY IOAPIC_ENTRY;

struct RSD {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    SDT* rootSDT;
} __attribute__ ((packed));

typedef struct RSD RSD;

static int iseq(const char* a, const char* b, uint32_t len) {
    for (uint32_t i = 0; i<len; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static RSD* findRSDInRange(uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i+= 16) {
        RSD *p = (RSD*)i;
        if (iseq(p->Signature,"RSD PTR ",8)) return p;
    }
    return 0;
}

static RSD* findRSD() {
    RSD* it = findRSDInRange(0x000E0000,0x00100000);

    if (it == 0) {
        uint32_t base = ((uint32_t) (*((uint16_t*) 0x40E))) << 4;
        it = findRSDInRange(base, base + (1 << 10));
    }
    return it;
}

static SDT* findSDT(RSD* rsdp, const char* name) {
    SDT *rsdt = rsdp->rootSDT;
    SDT **others = (SDT**) (rsdt + 1);
    int entries = (rsdt->Length - sizeof(SDT)) / 4;
 
    for (int i = 0; i < entries; i++)
    {
        SDT *h = others[i];
        if (iseq(h->Signature,name,4)) return h;
    }
 
    return 0;
}

static uint32_t memAbove1M() {
   ASSERT((memInfo.cx == 15 * 1024) || (memInfo.dx == 0));

   return (((uint32_t) (memInfo.ax | memInfo.cx)) << 10) +
          (((uint32_t) (memInfo.bx | memInfo.dx)) << 16);

}

Config kConfig;

void configInit(Config* config) {
    config->memSize = memAbove1M() + (1 << 20);
    RSD* rsdp = findRSD();
    //Debug::printf("found rsd %x\n",(uint32_t)rsdp);
    if (rsdp != 0) {
        //char oemid[7];
        for (int i=0; i<6; i++) {
           config->oemid[i] = rsdp->OEMID[i];
        }
        config->oemid[6] = 0;
        //Debug::printf("%s\n",oemid);
        //Debug::printf("RSD Revision %d\n",(uint32_t) rsdp->Revision);
    }

    MADT* madt = (MADT*) findSDT(rsdp,"APIC");
    FADT* fadt = (FADT*) findSDT(rsdp,"FACP");

    // SDT* temp = &fadt->h;
    // SDT* MadtTemp = &madt->sdt;
    // Debug::printf("header: %s\n", temp->Signature);
    // Debug::printf("size: %d\n", temp->Length);
    // Debug::printf("MADT header: %s\n", MadtTemp->Signature);

    Debug::printf("flag: %x\n", fadt->BootArchitectureFlags);
    Debug::printf("flag: %x\n", fadt->Reserved2);
    if(fadt->BootArchitectureFlags == 0) {
        //there's a PS2
        Debug::panic("NO PS2 CONTROLLER!");
    }

    //Debug::printf("found madt %x\n",(uint32_t) madt);

    config->localAPIC = madt->localAddress;
    config->madtFlags = madt->flags;

    int32_t bytesForEntries = madt->sdt.Length - sizeof(MADT);
    //Debug::printf("MADT bytes for entries %x\n",bytesForEntries);

    MADT_ENTRY* entryPtr = (MADT_ENTRY*)(madt + 1);

    config->nOtherProcs = 0; // Base processor not in tables

    while (bytesForEntries > 0) {
        uint32_t len = entryPtr->len;
        //Debug::printf("entry type %x\n",entryPtr->type);
        //Debug::printf("entry length %x\n",len);
        entryPtr = (MADT_ENTRY*) (((uintptr_t) entryPtr) + len);
        bytesForEntries -= len;

        if (entryPtr->type == 0) {
            APIC_ENTRY *apic = (APIC_ENTRY*) entryPtr;
            if (apic->processorId == 0) {
                continue;
            }
            ApicInfo * info = &config->apicInfo[config->nOtherProcs ++];
            info->processorId = apic->processorId;
            info->apicId = apic->apicId;
            info->flags = apic->flags;
        }
        else if (entryPtr->type == 1) {
            IOAPIC_ENTRY *apic = (IOAPIC_ENTRY*) entryPtr;
            // Debug::printf("ID: %d, address: 0x%x, base: %d\n", apic->apicId, apic->address, apic->base);
            config->ioAPIC = apic->address;
        }
    }

    config->totalProcs = config->nOtherProcs + 1;
}
