#include <Windows.h>

#include <assert.h>
#include <conio.h>
#include <stdint.h>
#include <stdio.h>

#include <ViGEm/Client.h>

PVIGEM_CLIENT client;
PVIGEM_TARGET pad;

XUSB_REPORT report;

static const int turningspeed = 4000;

short safeadd(short x, short y) {
  short ret = (x < 0) ? INT16_MIN : INT16_MAX;
  short comp = ret - x;
  if ((x < 0) == (y > comp))
    ret = x + y;
  return ret;
}

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
    // Left disc (turn)
    if (dwParam1 == 0x000130b0 || dwParam1 == 0x000132b0) {
      report.sThumbLX = safeadd(report.sThumbLX, turningspeed);
    }
    else if (dwParam1 == 0x007f30b0 || dwParam1 == 0x007f32b0) {
      report.sThumbLX = safeadd(report.sThumbLX, -turningspeed);
      // Sync left disc (reset turn)
    }
    else if (dwParam1 == 0x007f1790) {
      report.sThumbLX = 0;
      // Right volume (speed)
    }
    else if ((dwParam1 & 0xffff) == 0x3bb0) {
      char c = dwParam1 >> 16;
      if (c > 80) {
        report.wButtons |= 0x1000;
        report.wButtons &= ~0x2000;
      } else if (c > 40) {
        report.wButtons &= ~0x1000;
        report.wButtons &= ~0x2000;
      } else {
        report.wButtons &= ~0x1000;
        report.wButtons |= 0x2000;
      }
      // Drift
    }
    else if (dwParam1 == 0x007f1a90) {
      report.wButtons |= 0x0200;
    }
    else if (dwParam1 == 0x00001a90) {
      report.wButtons &= ~0x0200;
    }
    else if (dwParam1 == 0x007f3690) {
      report.wButtons |= 0x0001;
    }
    else if (dwParam1 == 0x00003690) {
      report.wButtons &= ~0x0001;
    }
    else if (dwParam1 == 0x007f3790) {
      report.wButtons |= 0x0002;
    }
    else if (dwParam1 == 0x00003790) {
      report.wButtons &= ~0x0002;
    }

    printf("dwParam1=%08x, dwParam2=%08x\n", (unsigned)dwParam1,
           (unsigned)dwParam2);
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

  memset(&report, 0, sizeof(report));

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
