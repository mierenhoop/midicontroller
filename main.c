#include <Windows.h>

#include <assert.h>
#include <conio.h>
#include <stdio.h>


#include <ViGEm/Client.h>

PVIGEM_CLIENT client;
PVIGEM_TARGET pad;

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance,
                         DWORD dwParam1, DWORD dwParam2) {
  switch (wMsg) {
  case MIM_OPEN:

    printf("wMsg=MIM_OPEN\n");
    break;
  case MIM_CLOSE:
    printf("wMsg=MIM_CLOSE\n");
    break;
  case MIM_DATA: {
    XUSB_REPORT report;
    memset(&report, 0, sizeof(report));
    if (dwParam1 == 0x007f1990) {
      report.wButtons |= 0x1000;
    }
    if ((dwParam1&0xffff) == 0x39b0) {
        int e = ((dwParam1 & 0x00ff0000) >> 8);
        printf("%d", e * 2 - 0xffff / 2 - 1);
        report.sThumbLX = e * 2 - 0xffff / 2 - 1;
    }
    printf("wMsg=MIM_DATA, dwInstance=%08x, dwParam1=%08x, dwParam2=%08x\n",
           dwInstance, dwParam1, dwParam2);
    vigem_target_x360_update(client, pad, report);
    break;
  }

  default:
    printf("wMsg = unknown\n");
    break;
  }
  return;
}

int main(int argc, char *argv[]) {
  HMIDIIN handle;
  int c = 0;

  assert(midiInGetNumDevs() > 0);

  assert(midiInOpen(&handle, 0, (DWORD_PTR)(void *)MidiInProc, 0,
                    CALLBACK_FUNCTION) == MMSYSERR_NOERROR);

  client = vigem_alloc();
  assert(client);
  assert(VIGEM_SUCCESS(vigem_connect(client)));
  pad = vigem_target_x360_alloc();
  assert(VIGEM_SUCCESS(vigem_target_add(client, pad)));

  midiInStart(handle);

  while (c != VK_ESCAPE && c != 'q') {
    if (!_kbhit()) {
      Sleep(100);
      continue;
    }
    c = _getch();
  }

  midiInStop(handle);
  midiInClose(handle);

  vigem_target_remove(client, pad);
  vigem_target_free(pad);

  return 0;
}

// const auto client = vigem_alloc();

// if (client == nullptr)
// {
//     std::cerr << "Uh, not enough memory to do that?!" << std::endl;
//     return -1;
// }

// const auto retval = vigem_connect(client);

// if (!VIGEM_SUCCESS(retval))
// {
//     std::cerr << "ViGEm Bus connection failed with error code: 0x" <<
//     std::hex << retval << std::endl; return -1;
// }

// //
// // Allocate handle to identify new pad
// //
// const auto pad = vigem_target_x360_alloc();

// //
// // Add client to the bus, this equals a plug-in event
// //
// const auto pir = vigem_target_add(client, pad);

// //
// // Error handling
// //
// if (!VIGEM_SUCCESS(pir))
// {
//     std::cerr << "Target plugin failed with error code: 0x" << std::hex <<
//     retval << std::endl; return -1;
// }

// while (1) {
// if (!_kbhit()) {
// 			Sleep(100);
// 			continue;
// 		}
// 		int c = _getch();
// 		if (c == VK_ESCAPE) break;
// 		if (c == 'q') break;
// }

// //
// // We're done with this pad, free resources (this disconnects the virtual
// device)
// //
// vigem_target_remove(client, pad);
// vigem_target_free(pad);

// }
