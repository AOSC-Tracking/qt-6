// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stddef.h>

#include "base/strings/string_util_win.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_handle.h"
#include "sandbox/win/src/handle_closer_agent.h"
#include "sandbox/win/src/nt_internals.h"
#include "sandbox/win/src/sandbox.h"
#include "sandbox/win/src/sandbox_factory.h"
#include "sandbox/win/src/target_services.h"
#include "sandbox/win/src/win_utils.h"
#include "sandbox/win/tests/common/controller.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const wchar_t* kFileExtensions[] = {L".1", L".2", L".3", L".4"};

// Returns a handle to a unique marker file that can be retrieved between runs.
HANDLE GetMarkerFile(const wchar_t* extension) {
  wchar_t path_buffer[MAX_PATH + 1];
  CHECK(::GetTempPath(MAX_PATH, path_buffer));
  std::wstring marker_path = path_buffer;
  marker_path += L"\\sbox_marker_";

  // Generate a unique value from the exe's size and timestamp.
  CHECK(::GetModuleFileName(nullptr, path_buffer, MAX_PATH));
  base::win::ScopedHandle module(
      ::CreateFile(path_buffer, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, nullptr,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  CHECK(module.is_valid());
  FILETIME timestamp;
  CHECK(::GetFileTime(module.get(), &timestamp, nullptr, nullptr));
  marker_path += base::ASCIIToWide(base::StringPrintf(
      "%08lx%08lx%08lx", ::GetFileSize(module.get(), nullptr),
      timestamp.dwLowDateTime, timestamp.dwHighDateTime));
  marker_path += extension;

  // Make the file delete-on-close so cleanup is automatic.
  return CreateFile(marker_path.c_str(), FILE_ALL_ACCESS,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
}

// Used by the thread pool tests.
HANDLE finish_event;
const int kWaitCount = 20;

}  // namespace

namespace sandbox {

// Checks for the presence of a list of files (in object path form).
// Format: CheckForFileHandle (Y|N) \path\to\file1 [\path\to\file2 ...]
// - Y or N depending if the file should exist or not.
SBOX_TESTS_COMMAND int CheckForFileHandles(int argc, wchar_t** argv) {
  if (argc < 2)
    return SBOX_TEST_FAILED_TO_RUN_TEST;
  bool should_find = argv[0][0] == L'Y';
  if (argv[0][1] != L'\0' || (!should_find && argv[0][0] != L'N'))
    return SBOX_TEST_FAILED_TO_RUN_TEST;

  static int state = BEFORE_INIT;
  switch (state++) {
    case BEFORE_INIT:
      // Create a unique marker file that is open while the test is running.
      // The handles leak, but it will be closed by the test or on exit.
      for (const wchar_t* kExtension : kFileExtensions)
        CHECK_NE(GetMarkerFile(kExtension), INVALID_HANDLE_VALUE);
      return SBOX_TEST_SUCCEEDED;

    case AFTER_REVERT: {
      // Brute force the handle table to find what we're looking for.
      DWORD handle_count = UINT_MAX;
      const int kInvalidHandleThreshold = 100;
      const size_t kHandleOffset = 4;  // Handles are always a multiple of 4.
      HANDLE handle = nullptr;
      int invalid_count = 0;

      if (!::GetProcessHandleCount(::GetCurrentProcess(), &handle_count))
        return SBOX_TEST_FAILED_TO_RUN_TEST;

      while (handle_count && invalid_count < kInvalidHandleThreshold) {
        reinterpret_cast<size_t&>(handle) += kHandleOffset;
        auto handle_name = GetPathFromHandle(handle);
        if (handle_name) {
          for (int i = 1; i < argc; ++i) {
            if (handle_name.value() == argv[i])
              return should_find ? SBOX_TEST_SUCCEEDED : SBOX_TEST_FAILED;
          }
          --handle_count;
        } else {
          ++invalid_count;
        }
      }

      return should_find ? SBOX_TEST_FAILED : SBOX_TEST_SUCCEEDED;
    }

    default:  // Do nothing.
      break;
  }

  return SBOX_TEST_SUCCEEDED;
}

// Checks that supplied handle is an Event and it's not waitable.
// Format: CheckForEventHandles
SBOX_TESTS_COMMAND int CheckForEventHandles(int argc, wchar_t** argv) {
  static int state = BEFORE_INIT;
  static std::vector<HANDLE> to_check;

  switch (state++) {
    case BEFORE_INIT:
      // Create a unique marker file that is open while the test is running.
      for (const wchar_t* kExtension : kFileExtensions) {
        HANDLE handle = GetMarkerFile(kExtension);
        CHECK_NE(handle, INVALID_HANDLE_VALUE);
        to_check.push_back(handle);
      }
      return SBOX_TEST_SUCCEEDED;

    case AFTER_REVERT:
      for (HANDLE handle : to_check) {
        auto type_name = GetTypeNameFromHandle(handle);
        CHECK(type_name);
        CHECK(base::EqualsCaseInsensitiveASCII(type_name.value(), L"Event"));

        // Should not be able to wait.
        CHECK_EQ(WaitForSingleObject(handle, INFINITE), WAIT_FAILED);

        // Should be able to close.
        CHECK(::CloseHandle(handle));
      }
      return SBOX_TEST_SUCCEEDED;

    default:  // Do nothing.
      break;
  }

  return SBOX_TEST_SUCCEEDED;
}

TEST(HandleCloserTest, CheckForMarkerFiles) {
  TestRunner runner;
  runner.SetTimeout(2000);
  runner.SetTestState(EVERY_STATE);

  std::wstring command = std::wstring(L"CheckForFileHandles Y");
  for (const wchar_t* kExtension : kFileExtensions) {
    base::win::ScopedHandle marker(GetMarkerFile(kExtension));
    CHECK(marker.is_valid());
    auto handle_name = GetPathFromHandle(marker.get());
    CHECK(handle_name);
    command += (L" ");
    command += handle_name.value();
  }

  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(command.c_str()))
      << "Failed: " << command;
}

TEST(HandleCloserTest, CloseMarkerFiles) {
  TestRunner runner;
  runner.SetTimeout(2000);
  runner.SetTestState(EVERY_STATE);
  sandbox::TargetPolicy* policy = runner.GetPolicy();

  std::wstring command = std::wstring(L"CheckForFileHandles N");
  for (const wchar_t* kExtension : kFileExtensions) {
    base::win::ScopedHandle marker(GetMarkerFile(kExtension));
    CHECK(marker.is_valid());
    auto handle_name = GetPathFromHandle(marker.get());
    CHECK(handle_name);
    CHECK_EQ(policy->GetConfig()->AddKernelObjectToClose(L"File",
                                                         handle_name->c_str()),
             SBOX_ALL_OK);
    command += (L" ");
    command += handle_name.value();
  }

  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(command.c_str()))
      << "Failed: " << command;
}

TEST(HandleCloserTest, CheckStuffedHandle) {
  TestRunner runner;
  runner.SetTimeout(2000);
  runner.SetTestState(EVERY_STATE);
  sandbox::TargetPolicy* policy = runner.GetPolicy();

  for (const wchar_t* kExtension : kFileExtensions) {
    base::win::ScopedHandle marker(GetMarkerFile(kExtension));
    CHECK(marker.is_valid());
    auto handle_name = GetPathFromHandle(marker.get());
    CHECK(handle_name);
    CHECK_EQ(policy->GetConfig()->AddKernelObjectToClose(L"File",
                                                         handle_name->c_str()),
             SBOX_ALL_OK);
  }

  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(L"CheckForEventHandles"));
}

void WINAPI ThreadPoolTask(void* event, BOOLEAN timeout) {
  static volatile LONG waiters_remaining = kWaitCount;
  CHECK(!timeout);
  CHECK(::CloseHandle(event));
  if (::InterlockedDecrement(&waiters_remaining) == 0)
    CHECK(::SetEvent(finish_event));
}

// Run a thread pool inside a sandbox without a CSRSS connection.
SBOX_TESTS_COMMAND int RunThreadPool(int argc, wchar_t** argv) {
  HANDLE wait_list[20];
  finish_event = ::CreateEvent(nullptr, true, false, nullptr);
  CHECK(finish_event);

  // Set up a bunch of waiters.
  HANDLE pool = nullptr;
  for (int i = 0; i < kWaitCount; ++i) {
    HANDLE event = ::CreateEvent(nullptr, true, false, nullptr);
    CHECK(event);
    CHECK(::RegisterWaitForSingleObject(&pool, event, ThreadPoolTask, event,
                                        INFINITE, WT_EXECUTEONLYONCE));
    wait_list[i] = event;
  }

  // Signal all the waiters.
  for (int i = 0; i < kWaitCount; ++i)
    CHECK(::SetEvent(wait_list[i]));

  CHECK_EQ(::WaitForSingleObject(finish_event, INFINITE), WAIT_OBJECT_0);
  CHECK(::CloseHandle(finish_event));

  return SBOX_TEST_SUCCEEDED;
}

TEST(HandleCloserTest, RunThreadPool) {
  TestRunner runner;
  runner.SetTimeout(2000);
  runner.SetTestState(AFTER_REVERT);

  // Sandbox policy will determine which platforms to disconnect CSRSS and when
  // to close the CSRSS handle.

  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(L"RunThreadPool"));
}

}  // namespace sandbox
