//	The code mixes naming conventions as I worked on the linux version before porting to windows
//  This code is also not the best so forgive me as I was in a hurry. You are very much welcome to open a PR with your changes to improve the code.
//  This also works ONLY on BIOS systems as it makes use of bootvid.dll

#include <ntddk.h>
#include <stdint.h>

#include "bootvid.h"

#include "BootImage.h"

// static const unsigned char BootImage[]
#define BOOT_IMAGE_SIZE  3476752
#define SCREEN_CHAR_WIDTH 80
#define SCREEN_CHAR_HEIGHT 30

// Hello old friend!
BOOLEAN UsingCapsOrNot = FALSE;

static char ktocSHIFT(uint8_t key) {
	char c = 0;
	uint8_t dict[2][94] = {
		{41, 2,	 3,	 4,	 5,	 6,	 7,	 8,	 9,	 10, 11, 12, 13, 26, 27, 43,
		 39, 40, 51, 52, 53, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37,
		 38, 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44, 57},
		{126, 33, 64, 35, 36, 37, 94, 38, 42, 40, 41, 95, 43, 123, 125, 124,
		 58,  34, 60, 62, 63, 65, 66, 67, 68, 69, 70, 71, 72, 73,  74,	75,
		 76,  77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,  90,	32} };
	for (int i = 0; i < 94; i++) {
		if (dict[0][i] == key) {
			c = (char)dict[1][i];
		}
	}
	return c;
}


static char ktoc(uint8_t key) {
	char c = 0;
	uint8_t dict[2][94] = {
		{57, 40, 51, 12, 52, 53, 11, 2,	 3,	 4,	 5,	 6,	 7,	 8,	 9,	 10,
		 39, 13, 26, 43, 27, 41, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36,
		 37, 38, 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44},
		{32,  39,  44,	45,	 46,  47,  48,	49,	 50,  51,  52,	53,
		 54,  55,  56,	57,	 59,  61,  91,	92,	 93,  96,  97,	98,
		 99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
		 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122} };
	for (int i = 0; i < 94; i++) {
		if (dict[0][i] == key) {
			c = (char)dict[1][i];
		}
	}
	return c;
}


// keep track of the cursor from our side
INT cX = 0;
INT cY = 0;

BOOLEAN Vt100Parsing = FALSE;

void mini_rv32ima_putchar(char c) {
	DbgPrint("%c", c);
	char string[2] = { c, 0 };

	// Ignore any ANSI
	// Yes I have tried parsing ANSI escape sequences but bootvid is sloowww
	if (c == '\033') {
		Vt100Parsing = TRUE;
	}

	if (Vt100Parsing) {
		if (c == 'm' || c == 'A' || c == 'B' || c == 'C' || c == 'D') Vt100Parsing = FALSE;
		return;
	}

	if (c == '\n') cY += 16;
	if (c == '\r') cX = 0;

	// bootvid's scrolling is horribly broken so we clear the screen instead
	// not the best solution
	if (cY > 479) { VidResetDisplay(0); cX = 0; cY = 0; };

	if (cX > 639) { cX = 0; cY += 16; }

	VidDisplayString(string);
	cX += 8;
}

void mini_rv32ima_print(char* string) {
	DbgPrint("RV32: %s\n", string);
}

int mini_rv32ima_key_hit(void) {
	return (__inbyte(0x64) & 1);
}

int mini_rv32ima_get_key(void) {
	char key = __inbyte(0x60);
	if (key == 0x1c) return '\r';
	if (key == 0x3A) { UsingCapsOrNot = !UsingCapsOrNot; return -1; }
	if (UsingCapsOrNot) return ktocSHIFT(key);
	else return ktoc(key);
}

void* mini_rv32ima_malloc(size_t count) {
	return ExAllocatePoolWithTag(NonPagedPool, count, 'CISR');
}

uint32_t mini_rv32ima_handle_control_store(uint32_t addy, uint32_t val) {
	if (addy == 0x10000000) mini_rv32ima_putchar(val);

	return 0;
}

uint32_t mini_rv32ima_handle_control_load(uint32_t addy) {
	if (addy == 0x10000005) return 0x60 | mini_rv32ima_key_hit();

	return 0;
}

void mini_rv32ima_handle_other_csr_write(uint16_t csrno, uint32_t value) {
	if (csrno == 0x139) mini_rv32ima_putchar(value);
}

uint32_t mini_rv32ima_handle_other_csr_read(uint16_t csrno) {
	if (csrno == 0x140) {
		if (!mini_rv32ima_key_hit()) return -1;
		return mini_rv32ima_get_key();
	}
	return 0;
}

#define RAM_SIZE  64 * 1024 * 1024

#define MINIRV32WARN
#define MINIRV32_DECORATE  static
#define MINI_RV32_RAM_SIZE RAM_SIZE
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_HANDLE_MEM_STORE_CONTROL(addy, val) if(mini_rv32ima_handle_control_store( addy, val )) return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL(addy, rval) rval = mini_rv32ima_handle_control_load( addy );
#define MINIRV32_OTHERCSR_WRITE(csrno, value) mini_rv32ima_handle_other_csr_write(csrno, value);
#define MINIRV32_OTHERCSR_READ(csrno, value) value = mini_rv32ima_handle_other_csr_read(csrno);

#include "mini-rv32ima.h"
#include "default64mbdtc.h"

UCHAR* memory = NULL;
struct MiniRV32IMAState* cpu = NULL;

PKBUGCHECK_CALLBACK_RECORD BugcheckCallbackRecord = NULL;

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	if (BugcheckCallbackRecord != NULL) {
		KeDeregisterBugCheckCallback(BugcheckCallbackRecord);
		ExFreePoolWithTag(BugcheckCallbackRecord, 'CSIR');
	}

	DbgPrint("[*] Goodbye Cruel World\n");
}

VOID BugcheckCallback(PVOID Buffer, ULONG Length) {
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(Length);

	VidResetDisplay(TRUE);
	VidSetTextColor(BV_COLOR_WHITE);
	VidSolidColorFill(0, 0, 639, 479, BV_COLOR_BLACK);
	VidSetScrollRegion(0, 0, 639, 479);
	
	// Enable the PS/2 Keyboard
	while ((__inbyte(0x64) & 2) != 0)
		;
	__outbyte(0x64, 0xae);
	

	BOOLEAN running = TRUE;
	while (running) {
		int ret = MiniRV32IMAStep(cpu, memory, 0, 1, 1024); // Execute upto 1024 cycles before breaking out.

		//printf("Ret %d PC %p\n", ret, cpu->pc);

		switch (ret) {
			case 0: break;
			case 1: break;
			case 3: break;
			case 21845: mini_rv32ima_print("poweroff requested. shutting down\n"); running = FALSE;
			default: running = FALSE;
		}
	}
	VidDisplayString("Halting forever now\n");
	for (;;) __halt();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = DriverUnload;

	memory = mini_rv32ima_malloc(RAM_SIZE);
	RtlZeroMemory(memory, RAM_SIZE);

	RtlCopyMemory(memory, BootImage, BOOT_IMAGE_SIZE);

	ULONG_PTR dtbPtr = RAM_SIZE - sizeof(default64mbdtb) - sizeof(struct MiniRV32IMAState);
	RtlCopyMemory(memory + dtbPtr, default64mbdtb, sizeof(default64mbdtb));

	cpu = (struct MiniRV32IMAState*)(memory + RAM_SIZE - sizeof(struct MiniRV32IMAState));
	cpu->pc = MINIRV32_RAM_IMAGE_OFFSET;
	cpu->regs[10] = 0x00; //hart ID
	cpu->regs[11] = (dtbPtr + MINIRV32_RAM_IMAGE_OFFSET); //dtb_pa (Must be valid pointer) (Should be pointer to dtb)
	cpu->extraflags |= 3; // Machine-mode.

	uint32_t* dtb = (uint32_t*)(memory + dtbPtr);
	if (dtb[0x13c / 4] == 0x00c0ff03) {
		uint32_t validram = dtbPtr;
		dtb[0x13c / 4] = (validram >> 24) | (((validram >> 16) & 0xff) << 8) | (((validram >> 8) & 0xff) << 16) | ((validram & 0xff) << 24);
	}

	BugcheckCallbackRecord = ExAllocatePoolZero(NonPagedPool, sizeof(KBUGCHECK_CALLBACK_RECORD), 'CSIR');
	if (BugcheckCallbackRecord == NULL) return STATUS_INSUFFICIENT_RESOURCES;

	KeInitializeCallbackRecord(BugcheckCallbackRecord);

	BOOLEAN Registered = KeRegisterBugCheckCallback(BugcheckCallbackRecord, BugcheckCallback, NULL, 0, (PUCHAR)"Loonix");
	if (!Registered) {
		ExFreePoolWithTag(BugcheckCallbackRecord, 'BCHK');

		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}