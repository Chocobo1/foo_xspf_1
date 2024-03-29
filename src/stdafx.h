/*
Copyright (c) 2016, Mike Tzou
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of foo_xspf_1 nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <ctime>
#include <future>
#include <list>
#include <map>
#include <set>
#include <string>

#include "../lib/foobar2000_sdk/foobar2000/SDK/foobar2000-lite.h"

#include "../lib/foobar2000_sdk/foobar2000/SDK/advconfig_impl.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/componentversion.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/file_info_impl.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/library_manager.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/main_thread_callback.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/playlist.h"
#include "../lib/foobar2000_sdk/foobar2000/SDK/playlist_loader.h"

#include "../lib/tinyxml2/tinyxml2.h"

#define CONSOLE_HEADER "foo_xspf_1: "
