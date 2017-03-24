/*
 * LunarGravity
 *
 * Copyright (C) 2017 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "lglogger.hpp"
#include "lgwindow.hpp"
#include "lggfxengine.hpp"

#define APPLICATION_NAME "Lunar Gravity Demo"
#define APPLICATION_VERSION 1

int main(int argc, char *argv[]) {
    bool fullscreen = false;
    bool validate = true;
    LgWindow window(320, 240, fullscreen);
    LgGraphicsEngine engine(APPLICATION_NAME, APPLICATION_VERSION, validate, window);
    return 0;
}
