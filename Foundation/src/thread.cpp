// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/thread.hpp"
#include "TTauri/Foundation/strings.hpp"

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#elif __has_include(<pthread.h>)
#include <pthread.h>
#endif

namespace TTauri {

void set_thread_name(std::string_view name)
{
#if OPERATING_SYSTEM == OS_WINDOWS
    let wname = translateString<std::wstring>(name);
    SetThreadDescription(GetCurrentThread(), wname.data());
#elif OPERATING_SYSTEM == OS_MACOS
    pthread_setname_np(name.data());
#elif OPERATING_SYSTEM == OS_LINUX
    pthread_setname_np(pthread_self(), name.data());
#endif
}

void run_on_main_thread(std::function<void()> f)
{
    if (is_main_thread()) {
        return f();

    } else if (Foundation_globals->main_thread_runner) {
        Foundation_globals->main_thread_runner(f);

    } else {
        // We could not run the thread on the main thread.
        no_default;
    }
}

}