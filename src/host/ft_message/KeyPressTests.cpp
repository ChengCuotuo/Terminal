/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "precomp.h"

#include "..\..\inc\consoletaeftemplates.hpp"

#include <memory>
#include <utility>
#include <iostream>

class KeyPressTests
{
    TEST_CLASS(KeyPressTests);

    TEST_METHOD(TestAltGr)
    {
        Log::Comment(L"Testing that alt-gr behavior hasn't changed");
        BOOL successBool;
        HWND hwnd = GetConsoleWindow();
        VERIFY_IS_NOT_NULL(hwnd);
        HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        DWORD events = 0;

        // flush input buffer
        FlushConsoleInputBuffer(inputHandle);
        successBool = GetNumberOfConsoleInputEvents(inputHandle, &events);
        VERIFY_IS_TRUE(successBool);
        VERIFY_ARE_EQUAL(events, 0);

        // send alt-gr + q keypress (@ on german keyboard)
        DWORD repeatCount = 1;
        SendMessage(hwnd,
                    WM_CHAR,
                    0x51, // q
                    repeatCount | HIWORD(KF_EXTENDED | KF_ALTDOWN));
        // make sure the the keypresses got processed
        events = 0;
        successBool = GetNumberOfConsoleInputEvents(inputHandle, &events);
        VERIFY_IS_TRUE(successBool);
        VERIFY_IS_GREATER_THAN(events, 0, NoThrowString().Format(L"%d", events));
        std::unique_ptr<INPUT_RECORD[]> inputBuffer = std::make_unique<INPUT_RECORD[]>(1);
        PeekConsoleInput(inputHandle,
                         inputBuffer.get(),
                         1,
                         &events);
        VERIFY_ARE_EQUAL(events, 1);

        // compare values against those that have historically been
        // returned with the same arguments to SendMessage
        VERIFY_ARE_EQUAL(inputBuffer[0].EventType, KEY_EVENT);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.bKeyDown, 1);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.wRepeatCount, 1);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.wVirtualKeyCode, 0);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.wVirtualScanCode, 0);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.dwControlKeyState, 32);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.uChar.UnicodeChar, 'Q');
    }

    TEST_METHOD(TestCoalesceSameKeyPress)
    {
        Log::Comment(L"Testing that key events are properly coalesced when the same key is pressed repeatedly");
        BOOL successBool;
        HWND hwnd = GetConsoleWindow();
        VERIFY_IS_NOT_NULL(hwnd);
        HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        DWORD events = 0;

        // flush input buffer
        FlushConsoleInputBuffer(inputHandle);
        successBool = GetNumberOfConsoleInputEvents(inputHandle, &events);
        VERIFY_IS_TRUE(successBool);
        VERIFY_ARE_EQUAL(events, 0);

        // send a bunch of 'a' keypresses to the console
        DWORD repeatCount = 1;
        const unsigned int messageSendCount = 1000;
        for (unsigned int i = 0; i < messageSendCount; ++i)
        {
            SendMessage(hwnd,
                        WM_CHAR,
                        0x41,
                        repeatCount);
        }

        // make sure the the keypresses got processed and coalesced
        events = 0;
        successBool = GetNumberOfConsoleInputEvents(inputHandle, &events);
        VERIFY_IS_TRUE(successBool);
        VERIFY_IS_GREATER_THAN(events, 0, NoThrowString().Format(L"%d", events));
        std::unique_ptr<INPUT_RECORD[]> inputBuffer = std::make_unique<INPUT_RECORD[]>(1);
        PeekConsoleInput(inputHandle,
                         inputBuffer.get(),
                         1,
                         &events);
        VERIFY_ARE_EQUAL(events, 1);
        VERIFY_ARE_EQUAL(inputBuffer[0].EventType, KEY_EVENT);
        VERIFY_ARE_EQUAL(inputBuffer[0].Event.KeyEvent.wRepeatCount, messageSendCount, NoThrowString().Format(L"%d", inputBuffer[0].Event.KeyEvent.wRepeatCount));

    }
};