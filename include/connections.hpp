/**
 * @file web.hpp
 * @author Nino Maruszewski (nino.maruszewski@gmail.com)
 * @brief Web utilities.
 * @version 1.0
 * @date 2022-12-22
 *
 * MIT License
 *
 * Copyright (c) 2022 Nino Maruszewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#pragma once

/**
 * @brief Attempt to connect to WiFi.
 *
 * @param reset Flag to reset WiFi settings.
 * @return bool If the connection was successful.
 */
bool wifi_connect(bool reset);

/**
 * @brief Get the WiFi manager version.
 *
 * @return A string with the version info.
 */
const char* wifi_manager_version();

/**
 * @brief Print stats about the connected WiFi network.
 */
void wifi_print_status();

/**
 * @brief Setup the mDNS responder
 *
 * @return bool If the setup was successful.
 */
bool mdns_setup();
