#include <cpprest/details/basic_types.h>


using utility::string_t;
/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define BOOST_TEST_MODULE upload

#include <boost/test/included/unit_test.hpp>
#include <giga/Application.h>
#include <giga/core/Uploader.h>

#include "path.h"

using namespace boost::unit_test;
using namespace giga;
using namespace giga::core;

BOOST_AUTO_TEST_CASE(test_upload_file) {
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));
    auto owner = app.authenticate(U("test_main"), U("password"));
    auto uploadFolder = owner.contactData().node();
    Uploader uploader{uploadFolder, boost::filesystem::path{U(GIGA_ROOT)} / U("src/tests/files")};
    uploader.start().wait();

    // TODO: make some assert ...
}

