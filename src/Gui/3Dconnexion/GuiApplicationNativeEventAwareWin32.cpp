
/*
Development tools and related technology provided under license from 3Dconnexion.
(c) 1992 - 2012 3Dconnexion. All rights reserved
*/

/*
Support for Qt added by David Dibben at
http://www.codegardening.com/2011/02/using-3dconnexion-mouse-with-qt.html
*/

/*
See also:
http://www.3dconnexion.com/forum/viewtopic.php?f=19&t=4968&sid=72c018bdcf0e6edc99a6effb5c0c48d9
*/

#include "PreCompiled.h"

#include <QGlobalStatic>
#include <QMainWindow>
#include <QWidget>
#include <FCConfig.h>
#include <Base/Console.h>
#include "GuiApplicationNativeEventAware.h"
#include "SpaceballEvent.h"

// Windows dependencies, enumerators and global variables
#ifdef _USE_3DCONNEXION_SDK

#include <QApplication>
#include <windows.h>
#include <cmath>

#define LOGITECH_VENDOR_ID 0x46d
#define _CONSTANT_INPUT_PERIOD 0

#ifndef RIDEV_DEVNOTIFY
#define RIDEV_DEVNOTIFY 0x00002000
#endif

#define _TRACE_WM_INPUT_PERIOD 0
#define _TRACE_RI_TYPE 0
#define _TRACE_RIDI_DEVICENAME 0
#define _TRACE_RIDI_DEVICEINFO 0
#define _TRACE_RI_RAWDATA 0
#define _TRACE_3DINPUT_PERIOD 0

#ifdef _WIN64
typedef unsigned __int64 QWORD;
#endif // _WIN64

static const int kTimeToLive = 5;

enum e3dconnexion_pid {
   eSpacePilot = 0xc625,
   eSpaceNavigator = 0xc626,
   eSpaceExplorer = 0xc627,
   eSpaceNavigatorForNotebooks = 0xc628,
   eSpacePilotPRO = 0xc629
};

enum e3dmouse_virtual_key
{
   V3DK_INVALID=0
   , V3DK_MENU=1, V3DK_FIT
   , V3DK_TOP, V3DK_LEFT, V3DK_RIGHT, V3DK_FRONT, V3DK_BOTTOM, V3DK_BACK
   , V3DK_CW, V3DK_CCW
   , V3DK_ISO1, V3DK_ISO2
   , V3DK_1, V3DK_2, V3DK_3, V3DK_4, V3DK_5, V3DK_6, V3DK_7, V3DK_8, V3DK_9, V3DK_10
   , V3DK_ESC, V3DK_ALT, V3DK_SHIFT, V3DK_CTRL
   , V3DK_ROTATE, V3DK_PANZOOM, V3DK_DOMINANT
   , V3DK_PLUS, V3DK_MINUS
};

struct tag_VirtualKeys
{
   e3dconnexion_pid pid;
   size_t nKeys;
   e3dmouse_virtual_key *vkeys;
};

static const e3dmouse_virtual_key SpaceExplorerKeys [] =
{
   V3DK_INVALID     // there is no button 0
   , V3DK_1, V3DK_2
   , V3DK_TOP, V3DK_LEFT, V3DK_RIGHT, V3DK_FRONT
   , V3DK_ESC, V3DK_ALT, V3DK_SHIFT, V3DK_CTRL
   , V3DK_FIT, V3DK_MENU
   , V3DK_PLUS, V3DK_MINUS
   , V3DK_ROTATE
};

static const e3dmouse_virtual_key SpacePilotKeys [] =
{
   V3DK_INVALID
   , V3DK_1, V3DK_2, V3DK_3, V3DK_4, V3DK_5, V3DK_6
   , V3DK_TOP, V3DK_LEFT, V3DK_RIGHT, V3DK_FRONT
   , V3DK_ESC, V3DK_ALT, V3DK_SHIFT, V3DK_CTRL
   , V3DK_FIT, V3DK_MENU
   , V3DK_PLUS, V3DK_MINUS
   , V3DK_DOMINANT, V3DK_ROTATE
};

static const struct tag_VirtualKeys _3dmouseVirtualKeys[]=
{
   eSpacePilot
   , sizeof(SpacePilotKeys)/sizeof(SpacePilotKeys[0])
   , const_cast<e3dmouse_virtual_key *>(SpacePilotKeys),
   eSpaceExplorer
   , sizeof(SpaceExplorerKeys)/sizeof(SpaceExplorerKeys[0])
   , const_cast<e3dmouse_virtual_key *>(SpaceExplorerKeys)
};


// Methods for windows events

/*!
	Converts a hid device keycode (button identifier) of a pre-2009 3Dconnexion USB device to the standard 3d mouse virtual key definition.

	\a pid USB Product ID (PID) of 3D mouse device
	\a hidKeyCode  Hid keycode as retrieved from a Raw Input packet

	\return The standard 3d mouse virtual key (button identifier) or zero if an error occurs.

	Converts a hid device keycode (button identifier) of a pre-2009 3Dconnexion USB device
	to the standard 3d mouse virtual key definition.
*/

unsigned short HidToVirtualKey(unsigned long pid, unsigned short hidKeyCode)
{
   unsigned short virtualkey=hidKeyCode;
   for (size_t i=0; i<sizeof(_3dmouseVirtualKeys)/sizeof(_3dmouseVirtualKeys[0]); ++i)
   {
	  if (pid == _3dmouseVirtualKeys[i].pid)
	  {
		 if (hidKeyCode < _3dmouseVirtualKeys[i].nKeys)
			virtualkey = _3dmouseVirtualKeys[i].vkeys[hidKeyCode];
		 else
			virtualkey = V3DK_INVALID;
		 break;
	  }
   }
   // Remaining devices are unchanged
   return virtualkey;
}

std::string getEventType(unsigned int id)
{
    std::string str;
    switch (id) {
    case WM_INPUT:
        str = "WM_INPUT";
        break;
    case WM_PAINT:
        str = "WM_PAINT";
        break;
    case WM_NCPAINT:
        str = "WM_NCPAINT";
        break;
    case WM_IME_SETCONTEXT:
        str = "WM_IME_SETCONTEXT";
        break;
    case WM_WINDOWPOSCHANGING:
        str = "WM_WINDOWPOSCHANGING";
        break;
    case WM_WINDOWPOSCHANGED:
        str = "WM_WINDOWPOSCHANGED";
        break;
    case WM_GETMINMAXINFO:
        str = "WM_GETMINMAXINFO";
        break;
    case WM_ACTIVATEAPP:
        str = "WM_ACTIVATEAPP";
        break;
    case WM_TIMER:
        str = "WM_TIMER";
        break;
    case WM_NCHITTEST:
        str = "WM_NCHITTEST";
        break;
    case WM_LBUTTONDOWN:
        str = "WM_LBUTTONDOWN";
        break;
    case WM_LBUTTONUP:
        str = "WM_LBUTTONUP";
        break;
    case WM_MOUSEMOVE:
        str = "WM_MOUSEMOVE";
        break;
    case WM_ERASEBKGND:
        str = "WM_ERASEBKGND";
        break;
    case WM_NCMOUSELEAVE:
        str = "WM_NCMOUSELEAVE";
        break;
    case WM_MOUSELEAVE:
        str = "WM_MOUSELEAVE";
        break;
    case WM_SETCURSOR:
        str = "WM_SETCURSOR";
        break;
    case WM_SETICON:
        str = "WM_SETICON";
        break;
    case WM_GETICON:
        str = "WM_GETICON";
        break;
    case WM_CAPTURECHANGED:
        str = "WM_CAPTURECHANGED";
        break;
    case WM_SIZE:
        str = "WM_SIZE";
        break;
    case WM_SHOWWINDOW:
        str = "WM_SHOWWINDOW";
        break;
    case WM_NCCREATE:
        str = "WM_NCCREATE";
        break;
    case WM_IME_REQUEST:
        str = "WM_IME_REQUEST";
        break;
    case WM_DRAWCLIPBOARD:
        str = "WM_DRAWCLIPBOARD";
        break;
    case WM_SETFOCUS:
        str = "WM_SETFOCUS";
        break;
    case WM_KILLFOCUS:
        str = "WM_KILLFOCUS";
        break;
    case WM_DWMNCRENDERINGCHANGED:
        str = "WM_DWMNCRENDERINGCHANGED";
        break;
    case WM_PARENTNOTIFY:
        str = "WM_PARENTNOTIFY";
        break;
    case WM_NCMOUSEMOVE:
        str = "WM_NCMOUSEMOVE";
        break;
    case WM_CREATE:
        str = "WM_CREATE";
        break;
    case WM_NCCALCSIZE:
        str = "WM_NCCALCSIZE";
        break;
    case WM_CHILDACTIVATE:
        str = "WM_CHILDACTIVATE";
        break;
    case WM_IME_NOTIFY:
        str = "WM_IME_NOTIFY";
        break;
    case WM_NCDESTROY:
        str = "WM_NCDESTROY";
        break;
    case WM_MOVE:
        str = "WM_MOVE";
        break;
    case WM_NCACTIVATE:
        str = "WM_NCACTIVATE";
        break;
    case WM_DESTROY:
        str = "WM_DESTROY";
        break;
    case WM_ACTIVATE:
        str = "WM_ACTIVATE";
        break;
    default:
        str = "???";
        break;
    }
    return str;
}

bool Gui::GUIApplicationNativeEventAware::RawInputEventFilter(void* msg, long* result)
{
    if (gMouseInput == 0) {
        Base::Console().Error("GUIApplicationNativeEventAware::RawInputEventFilter: internal error\n");
    }
    else {
        MSG* message = (MSG*)(msg);
        std::string eventType = getEventType(message->message);
        Base::Console().Log("GUIApplicationNativeEventAware::RawInputEventFilter: Message type: 0x%04x (%s)\n",
            message->message, eventType.c_str());
    }
	if (gMouseInput == 0) return false;

	MSG* message = (MSG*)(msg);

	if (message->message == WM_INPUT) {
		HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(message->lParam);
		gMouseInput->OnRawInput(RIM_INPUT,hRawInput);
		if (result != 0)  {
			result = 0;
		}
		return true;
	}

	return false;
}


/*!
	Access the mouse parameters structure
*/
I3dMouseParam& Gui::GUIApplicationNativeEventAware::MouseParams()
{
	return f3dMouseParams;
}

/*!
	Access the mouse parameters structure
*/
const I3dMouseParam& Gui::GUIApplicationNativeEventAware::MouseParams() const
{
	return f3dMouseParams;
}

/*!
	Called with the processed motion data when a 3D mouse event is received

	The default implementation emits a Move3d signal with the motion data
*/
void Gui::GUIApplicationNativeEventAware::Move3d(HANDLE device, std::vector<float>& motionData)
{
	Q_UNUSED(device);
	
	QWidget *currentWidget = this->focusWidget();
    if (!currentWidget)
        currentWidget = mainWindow;

    motionDataArray[0] = ceil(motionData[0]);
    motionDataArray[1] = ceil(motionData[1]);
    motionDataArray[2] = ceil(motionData[2]);
    motionDataArray[3] = ceil(motionData[3]);
    motionDataArray[4] = ceil(motionData[4]);
    motionDataArray[5] = ceil(motionData[5]);

    if (!setOSIndependentMotionData()) return;
    importSettings();

    Spaceball::MotionEvent *motionEvent = new Spaceball::MotionEvent();

    motionEvent->setTranslations(motionDataArray[0], motionDataArray[1], motionDataArray[2]);
    motionEvent->setRotations(motionDataArray[3], motionDataArray[4], motionDataArray[5]);

    Base::Console().Message("GUIApplicationNativeEventAware::Move3d:\n");
    if (currentWidget) {
        const char* cname = currentWidget->metaObject()->className();
        Base::Console().Message("Send motion event to widget: %s\n", cname);
        for (int i=0; i<6;i++)
            Base::Console().Message("motionEvent[%d]: %d\n", i, motionDataArray[i]);
    }
    this->postEvent(currentWidget, motionEvent);
}

/*!
	Called when a 3D mouse key is pressed

	The default implementation emits a On3dmouseKeyDown signal with the key code.
*/
void Gui::GUIApplicationNativeEventAware::On3dmouseKeyDown(HANDLE device, int virtualKeyCode)
{
	Q_UNUSED(device);
    
    QWidget *currentWidget = this->focusWidget();
    if (!currentWidget)
        currentWidget = mainWindow;

    Spaceball::ButtonEvent *buttonEvent = new Spaceball::ButtonEvent();
    buttonEvent->setButtonNumber(virtualKeyCode - 1);
    buttonEvent->setButtonStatus(Spaceball::BUTTON_PRESSED);
    this->postEvent(currentWidget, buttonEvent);
}

/*!
	Called when a 3D mouse key is released

	The default implementation emits a On3dmouseKeyUp signal with the key code.
*/
void Gui::GUIApplicationNativeEventAware::On3dmouseKeyUp(HANDLE device, int virtualKeyCode)
{
	Q_UNUSED(device);
    
    QWidget *currentWidget = this->focusWidget();
    if (!currentWidget)
        currentWidget = mainWindow;

    Spaceball::ButtonEvent *buttonEvent = new Spaceball::ButtonEvent();
    buttonEvent->setButtonNumber(virtualKeyCode - 1);
    buttonEvent->setButtonStatus(Spaceball::BUTTON_RELEASED);
    this->postEvent(currentWidget, buttonEvent);
}

/*!
	Get an initialized array of PRAWINPUTDEVICE for the 3D devices

	pNumDevices returns the number of devices to register. Currently this is always 1.
 */
static PRAWINPUTDEVICE GetDevicesToRegister(unsigned int* pNumDevices)
{
	// Array of raw input devices to register
	static RAWINPUTDEVICE sRawInputDevices[] = {
		{0x01, 0x08, 0x00, 0x00} // Usage Page = 0x01 Generic Desktop Page, Usage Id= 0x08 Multi-axis Controller
	};

	if (pNumDevices) {
		*pNumDevices = sizeof(sRawInputDevices) / sizeof(sRawInputDevices[0]);
	}

	return sRawInputDevices;
}

/*!
	Detect the 3D mouse
*/
bool Gui::GUIApplicationNativeEventAware::Is3dmouseAttached()
{
	unsigned int numDevicesOfInterest = 0;
	PRAWINPUTDEVICE devicesToRegister = GetDevicesToRegister(&numDevicesOfInterest);

	unsigned int nDevices = 0;

	if (::GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0) {
		return false;
	}

	if (nDevices == 0) return false;

	std::vector<RAWINPUTDEVICELIST> rawInputDeviceList(nDevices);
	if (::GetRawInputDeviceList(&rawInputDeviceList[0], &nDevices, sizeof(RAWINPUTDEVICELIST)) == static_cast<unsigned int>(-1)) {
		return false;
	}

    Base::Console().Message("    Total number of found devices: %d\n", nDevices);
	for (unsigned int i = 0; i < nDevices; ++i) {
		RID_DEVICE_INFO rdi = {sizeof(rdi)};
		unsigned int cbSize = sizeof(rdi);

		if (GetRawInputDeviceInfo(rawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) > 0) {
			//skip non HID and non logitec (3DConnexion) devices
			if (rdi.dwType != RIM_TYPEHID || rdi.hid.dwVendorId != LOGITECH_VENDOR_ID) {
				continue;
			}

            Base::Console().Message("    Logitech device: HID: %d, VID: %d\n", RIM_TYPEHID, LOGITECH_VENDOR_ID);

			//check if devices matches Multi-axis Controller
			for (unsigned int j = 0; j < numDevicesOfInterest; ++j) {
				if (devicesToRegister[j].usUsage == rdi.hid.usUsage
						&& devicesToRegister[j].usUsagePage == rdi.hid.usUsagePage) {
                    Base::Console().Message("    Multi-axis Controller found: Usage: %d, Usage page: %d\n", rdi.hid.usUsage, rdi.hid.usUsagePage);
					return true;
				}
			}
		}
	}
	return false;
}



/*!
	Initialize the window to recieve raw-input messages

	This needs to be called initially so that Windows will send the messages from the 3D mouse to the window.
*/
bool Gui::GUIApplicationNativeEventAware::InitializeRawInput(HWND hwndTarget)
{
	fWindow = hwndTarget;

	// Simply fail if there is no window
	if (!hwndTarget)  return false;

	unsigned int numDevices = 0;
	PRAWINPUTDEVICE devicesToRegister = GetDevicesToRegister(&numDevices);

	if (numDevices == 0) return false;

	// Get OS version.
	OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO),0};
	::GetVersionEx(&osvi);

	unsigned int cbSize = sizeof (devicesToRegister[0]);
	for (size_t i = 0; i < numDevices; i++) {
		// Set the target window to use
		//devicesToRegister[i].hwndTarget = hwndTarget;

		// If Vista or newer, enable receiving the WM_INPUT_DEVICE_CHANGE message.
		if (osvi.dwMajorVersion >= 6) {
			devicesToRegister[i].dwFlags |= RIDEV_DEVNOTIFY;
		}
	}
	return (::RegisterRawInputDevices(devicesToRegister, numDevices, cbSize) != FALSE);
}


/*!
	Get the raw input data from Windows

	Includes workaround for incorrect alignment of the RAWINPUT structure on x64 os
	when running as Wow64 (copied directly from 3DConnexion code)
*/

UINT Gui::GUIApplicationNativeEventAware::GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
#ifdef _WIN64
	return ::GetRawInputBuffer(pData, pcbSize, cbSizeHeader);
#else
	BOOL bIsWow64 = FALSE;
	::IsWow64Process(GetCurrentProcess(), &bIsWow64);
	if (!bIsWow64 || pData==NULL) {
		return ::GetRawInputBuffer(pData, pcbSize, cbSizeHeader);
	} else {
		HWND hwndTarget = fWindow; //fParent->winId();

		size_t cbDataSize=0;
		UINT nCount=0;
		PRAWINPUT pri = pData;

		MSG msg;
		while (PeekMessage(&msg, hwndTarget, WM_INPUT, WM_INPUT, PM_NOREMOVE)) {
			HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(msg.lParam);
			size_t cbSize = *pcbSize - cbDataSize;
			if (::GetRawInputData(hRawInput, RID_INPUT, pri, &cbSize, cbSizeHeader) == static_cast<UINT>(-1)) {
				if (nCount==0) {
					return static_cast<UINT>(-1);
				}  else {
					break;
				}
			}
			++nCount;

			// Remove the message for the data just read
			PeekMessage(&msg, hwndTarget, WM_INPUT, WM_INPUT, PM_REMOVE);

			pri = NEXTRAWINPUTBLOCK(pri);
			cbDataSize = reinterpret_cast<ULONG_PTR>(pri) - reinterpret_cast<ULONG_PTR>(pData);
			if (cbDataSize >= *pcbSize) {
				cbDataSize = *pcbSize;
				break;
			}
		}
		return nCount;
	}
#endif
}

/*!
	Process the raw input device data

	On3dmouseInput() does all the preprocessing of the rawinput device data before
	finally calling the Move3d method.
*/

void Gui::GUIApplicationNativeEventAware::On3dmouseInput()
{
    Base::Console().Message("Call of GUIApplicationNativeEventAware::On3dmouseInput()\n");
	// Don't do any data processing in background
	bool bIsForeground = (::GetActiveWindow() != NULL);
	if (!bIsForeground) {
		// set all cached data to zero so that a zero event is seen and the cached data deleted
		for (std::map<HANDLE, TInputData>::iterator it = fDevice2Data.begin(); it != fDevice2Data.end(); it++) {
			it->second.fAxes.assign(6, .0);
			it->second.fIsDirty = true;
		}
	}

    if (bIsForeground) {
        Base::Console().Message("Event is triggered in foreground\n");
    }
    else {
        Base::Console().Message("Event is triggered in background\n");
    }

	DWORD dwNow = ::GetTickCount();           // Current time;
	DWORD dwElapsedTime;                      // Elapsed time since we were last here

	if (0 == fLast3dmouseInputTime) {
		dwElapsedTime = 10;                    // System timer resolution
	} else {
		dwElapsedTime = dwNow - fLast3dmouseInputTime;
		if (fLast3dmouseInputTime > dwNow) {
			dwElapsedTime = ~dwElapsedTime+1;
		}
		if (dwElapsedTime<1) {
			dwElapsedTime=1;
		} else if (dwElapsedTime > 500) {
			// Check for wild numbers because the device was removed while sending data
			dwElapsedTime = 10;
		}
	}

#if _TRACE_3DINPUT_PERIOD
	qDebug("On3DmouseInput() period is %dms\n", dwElapsedTime);
#endif

	float mouseData2Rotation;
	// v = w * r,  we don't know r yet so lets assume r=1.)
	float mouseData2PanZoom;

	// Grab the I3dmouseParam interface
	 I3dMouseParam& i3dmouseParam = f3dMouseParams;
	 // Take a look at the users preferred speed setting and adjust the sensitivity accordingly
	 I3dMouseSensor::ESpeed speedSetting = i3dmouseParam.GetSpeed();
	 // See "Programming for the 3D Mouse", Section 5.1.3
	float speed = (speedSetting == I3dMouseSensor::kLowSpeed ? 0.25f : speedSetting == I3dMouseSensor::kHighSpeed ?  4.f : 1.f);

	 // Multiplying by the following will convert the 3d mouse data to real world units
	mouseData2PanZoom = speed;
	mouseData2Rotation = speed;

	std::map<HANDLE, TInputData>::iterator iterator=fDevice2Data.begin();
	while (iterator != fDevice2Data.end()) {

		// If we have not received data for a while send a zero event
		if ((--(iterator->second.fTimeToLive)) == 0) {
			iterator->second.fAxes.assign(6, .0);
		} else if (/*!t_bPoll3dmouse &&*/ !iterator->second.fIsDirty) {
		  // If we are not polling then only handle the data that was actually received
			++iterator;
			continue;
		}
		iterator->second.fIsDirty=false;

		// get a copy of the device
		HANDLE hdevice = iterator->first;

		// get a copy of the motion vectors and apply the user filters
		std::vector<float> motionData = iterator->second.fAxes;

		// apply the user filters

		// Pan Zoom filter
		// See "Programming for the 3D Mouse", Section 5.1.2
		if (!i3dmouseParam.IsPanZoom()) {
			// Pan zoom is switched off so set the translation vector values to zero
			motionData[0] =  motionData[1] =  motionData[2] = 0.;
		}

		// Rotate filter
		// See "Programming for the 3D Mouse", Section 5.1.1
		if (!i3dmouseParam.IsRotate()) {
			// Rotate is switched off so set the rotation vector values to zero
			motionData[3] =  motionData[4] =  motionData[5] = 0.;
		}

		// convert the translation vector into physical data
		for (int axis = 0; axis < 3; axis++) {
			 motionData[axis] *= mouseData2PanZoom;
		 }
		// convert the directed Rotate vector into physical data
		// See "Programming for the 3D Mouse", Section 7.2.2
		for (int axis = 3; axis < 6; axis++) {
			motionData[axis] *= mouseData2Rotation;
		}

		// Now that the data has had the filters and sensitivty settings applied
		// calculate the displacements since the last view update
		for (int axis = 0; axis < 6; axis++) {
			motionData[axis] *= dwElapsedTime;
		}


		// Now a bit of book keeping before passing on the data
		if (iterator->second.IsZero()) {
			iterator = fDevice2Data.erase(iterator);
		} else {
			++iterator;
		}

		// Work out which will be the next device
		HANDLE hNextDevice = 0;
		if (iterator != fDevice2Data.end()) {
			hNextDevice = iterator->first;
		}

		 // Pass the 3dmouse input to the view controller
		 Move3d(hdevice, motionData);

		// Because we don't know what happened in the previous call, the cache might have
		// changed so reload the iterator
		iterator = fDevice2Data.find(hNextDevice);
   }

	if (!fDevice2Data.empty()) {
		fLast3dmouseInputTime = dwNow;
	} else {
		fLast3dmouseInputTime = 0;
	}
}

/*!
	Called when new raw input data is available
*/
void Gui::GUIApplicationNativeEventAware::OnRawInput(UINT nInputCode, HRAWINPUT hRawInput)
{
    Base::Console().Message("GUIApplicationNativeEventAware::OnRawInput: Handle raw input event\n");
	const size_t cbSizeOfBuffer=1024;
	BYTE pBuffer[cbSizeOfBuffer];

	PRAWINPUT pRawInput = reinterpret_cast<PRAWINPUT>(pBuffer);
	UINT cbSize = cbSizeOfBuffer;

	if (::GetRawInputData(hRawInput, RID_INPUT, pRawInput, &cbSize, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
        Base::Console().Message("Wrong input event\n");
		return;
	}

	bool b3dmouseInput = TranslateRawInputData(nInputCode, pRawInput);
	::DefRawInputProc(&pRawInput, 1, sizeof(RAWINPUTHEADER));

	// Check for any buffered messages
	cbSize = cbSizeOfBuffer;
	UINT nCount = this->GetRawInputBuffer(pRawInput, &cbSize, sizeof(RAWINPUTHEADER));
	if (nCount == (UINT)-1) {
		 qDebug ("GetRawInputBuffer returned error %d\n", GetLastError());
	}

	while (nCount>0 && nCount !=  static_cast<UINT>(-1)) {
		PRAWINPUT pri = pRawInput;
		UINT nInput;
		for (nInput=0; nInput<nCount; ++nInput) {
			b3dmouseInput |= TranslateRawInputData(nInputCode, pri);
			// clean the buffer
			::DefRawInputProc(&pri, 1, sizeof(RAWINPUTHEADER));

			pri = NEXTRAWINPUTBLOCK(pri);
		}
		cbSize = cbSizeOfBuffer;
		nCount = this->GetRawInputBuffer(pRawInput, &cbSize, sizeof(RAWINPUTHEADER));
	}

	// If we have mouse input data for the app then tell tha app about it
	if (b3dmouseInput) {
		On3dmouseInput();
	}
    else {
        Base::Console().Message("No input event for Spaceball\n");
    }
}



bool Gui::GUIApplicationNativeEventAware::TranslateRawInputData(UINT nInputCode, PRAWINPUT pRawInput)
{
	bool bIsForeground = (nInputCode == RIM_INPUT);

#if _TRACE_RI_TYPE
	qDebug("Rawinput.header.dwType=0x%x\n", pRawInput->header.dwType);
#endif
	// We are not interested in keyboard or mouse data received via raw input
	if (pRawInput->header.dwType != RIM_TYPEHID) return false;

#if _TRACE_RIDI_DEVICENAME
	UINT dwSize=0;
	if (::GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_DEVICENAME, NULL, &dwSize) == 0)  {
		std::vector<wchar_t> szDeviceName(dwSize+1);
		if (::GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_DEVICENAME, &szDeviceName[0],	&dwSize) >0) {
			qDebug("Device Name = %s\nDevice handle = 0x%x\n", &szDeviceName[0], pRawInput->header.hDevice);
		}
   }
#endif

	RID_DEVICE_INFO sRidDeviceInfo;
	sRidDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
	UINT cbSize = sizeof(RID_DEVICE_INFO);

	if (::GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_DEVICEINFO, &sRidDeviceInfo, &cbSize) == cbSize) {
#if _TRACE_RIDI_DEVICEINFO
		switch (sRidDeviceInfo.dwType)  {
			case RIM_TYPEMOUSE:
				qDebug("\tsRidDeviceInfo.dwType=RIM_TYPEMOUSE\n");
				break;
			case RIM_TYPEKEYBOARD:
				qDebug("\tsRidDeviceInfo.dwType=RIM_TYPEKEYBOARD\n");
				break;
			case RIM_TYPEHID:
				qDebug("\tsRidDeviceInfo.dwType=RIM_TYPEHID\n");
				qDebug("\tVendor=0x%x\n\tProduct=0x%x\n\tUsagePage=0x%x\n\tUsage=0x%x\n",
						sRidDeviceInfo.hid.dwVendorId,
						sRidDeviceInfo.hid.dwProductId,
						sRidDeviceInfo.hid.usUsagePage,
						sRidDeviceInfo.hid.usUsage);
				break;
		}
#endif

		if (sRidDeviceInfo.hid.dwVendorId == LOGITECH_VENDOR_ID) {
			if (pRawInput->data.hid.bRawData[0] == 0x01) { // Translation vector

				TInputData& deviceData = fDevice2Data[pRawInput->header.hDevice];
				deviceData.fTimeToLive = kTimeToLive;
				if (bIsForeground) {
					short* pnRawData = reinterpret_cast<short*>(&pRawInput->data.hid.bRawData[1]);
					// Cache the pan zoom data
					deviceData.fAxes[0] = static_cast<float>(pnRawData[0]);
					deviceData.fAxes[1] = static_cast<float>(pnRawData[1]);
					deviceData.fAxes[2] = static_cast<float>(pnRawData[2]);

#if _TRACE_RI_RAWDATA
					qDebug("Pan/Zoom RI Data =\t0x%x,\t0x%x,\t0x%x\n",
									pnRawData[0],  pnRawData[1],  pnRawData[2]);
#endif
					if (pRawInput->data.hid.dwSizeHid >= 13) {// Highspeed package
						// Cache the rotation data
						deviceData.fAxes[3] = static_cast<float>(pnRawData[3]);
						deviceData.fAxes[4] = static_cast<float>(pnRawData[4]);
						deviceData.fAxes[5] = static_cast<float>(pnRawData[5]);
						deviceData.fIsDirty = true;
#if _TRACE_RI_RAWDATA
						qDebug("Rotation RI Data =\t0x%x,\t0x%x,\t0x%x\n",
							 pnRawData[3], pnRawData[4], pnRawData[5]);
#endif
						return true;
					}
				} else { // Zero out the data if the app is not in forground
					deviceData.fAxes.assign(6, 0.f);
				}
			} else if (pRawInput->data.hid.bRawData[0] == 0x02) { // Rotation vector
				// If we are not in foreground do nothing
				// The rotation vector was zeroed out with the translation vector in the previous message
				if (bIsForeground) {
					TInputData& deviceData = fDevice2Data[pRawInput->header.hDevice];
					deviceData.fTimeToLive = kTimeToLive;

					short* pnRawData = reinterpret_cast<short*>(&pRawInput->data.hid.bRawData[1]);
					// Cache the rotation data
					deviceData.fAxes[3] = static_cast<float>(pnRawData[0]);
					deviceData.fAxes[4] = static_cast<float>(pnRawData[1]);
					deviceData.fAxes[5] = static_cast<float>(pnRawData[2]);
					deviceData.fIsDirty = true;

#if _TRACE_RI_RAWDATA
					qDebug("Rotation RI Data =\t0x%x,\t0x%x,\t0x%x\n",
						pnRawData[0],  pnRawData[1], pnRawData[2]);
#endif
					return true;
				}
			} else if (pRawInput->data.hid.bRawData[0] == 0x03)  { // Keystate change
				// this is a package that contains 3d mouse keystate information
				// bit0=key1, bit=key2 etc.


				unsigned long dwKeystate = *reinterpret_cast<unsigned long*>(&pRawInput->data.hid.bRawData[1]);
#if _TRACE_RI_RAWDATA
				qDebug("ButtonData =0x%x\n", dwKeystate);
#endif
				// Log the keystate changes
				unsigned long dwOldKeystate = fDevice2Keystate[pRawInput->header.hDevice];
				if (dwKeystate != 0) {
					fDevice2Keystate[pRawInput->header.hDevice] = dwKeystate;
				} else {
					fDevice2Keystate.erase(pRawInput->header.hDevice);
				}

				//  Only call the keystate change handlers if the app is in foreground
				if (bIsForeground) {
					unsigned long dwChange = dwKeystate ^ dwOldKeystate;


					for (int nKeycode=1; nKeycode<33; nKeycode++) {
						if (dwChange & 0x01) {
							int nVirtualKeyCode = HidToVirtualKey(sRidDeviceInfo.hid.dwProductId, nKeycode);
							if (nVirtualKeyCode) {
								if (dwKeystate&0x01) {
									On3dmouseKeyDown(pRawInput->header.hDevice, nVirtualKeyCode);
								} else {
									On3dmouseKeyUp(pRawInput->header.hDevice, nVirtualKeyCode);
								}
							}
						}
						dwChange >>=1;
						dwKeystate >>=1;
					}
				}
			}
		}
   }
   return false;
}
#endif // _USE_3DCONNEXION_SDK
