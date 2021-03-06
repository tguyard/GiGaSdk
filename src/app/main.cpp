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

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <giga/Application.h>
#include <giga/core/FileUploader.h>
#include <giga/core/Node.h>
#include <giga/api/data/Node.h>
#include <giga/rest/JsonSerializer.h>
#include <giga/rest/HttpErrors.h>
#include <giga/core/Uploader.h>
#include <giga/core/Downloader.h>
#include <giga/utils/Utils.h>
#include <giga/version.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <cpprest/json.h>
#include <algorithm>

using boost::exception_detail::error_info_container;
using utility::string_t;

using namespace giga;
namespace po = boost::program_options;

void printNodeTree(core::Node& n, string_t space = U(""))
{
    ucout << space << n.name() << "\n";
    space += U(" ");
    for(const auto& child : n.getChildren())
    {
        printNodeTree(*child, space);
    }
}

template <typename T>
void printUsers(const char* name, T users)
{
    ucout << U("\n") << name << std::endl;
    for(auto& user : users)
    {
        ucout << user.login() << "\t" << user.id() << "\t" << user.hasRelation() << "\n";
    }
}

template <typename T>
void printNodes(const char* name, T& nodes)
{
    ucout << "\n" << name << std::endl;
    for(auto& node : nodes)
    {
        ucout << core::Node::typeCvrt.toStr(node->type()) << "\t" << utils::str2wstr(node->id()) << "\t" << node->name() << "\n";
    }
}

void
searchNodes (const po::variables_map& vm, giga::Application& app, const char* name, core::Node::MediaType type)
{
    if (vm.count(name))
    {
        auto nodes = app.searchNode(vm[name].as<string_t>(), type);
        printNodes(name, nodes);
    }
}


int main(int argc, const char* argv[]) {
    try {
#ifdef _UTF16_STRINGS
#define VALUE wvalue
#else
#define VALUE value
#endif
        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h",    "produce help message")
        ("version,v", "gets the version")

        ("login",        po::VALUE<string_t>()->required(), "Current user login")
        ("password",     po::VALUE<string_t>(), "Current user password")

        ("list-contact",         po::value<bool>()->implicit_value(true), "print contacts")
        ("list-received-invits", po::value<bool>()->implicit_value(true), "print received invitation")
        ("list-sent-invits",     po::value<bool>()->implicit_value(true), "print sent invitation")
        ("list-blocked",         po::value<bool>()->implicit_value(true), "print blocked users")

        ("search-user",     po::VALUE<string_t>(), "search users")
        ("search-video",    po::VALUE<string_t>(), "search video files")
        ("search-audio",    po::VALUE<string_t>(), "search audio files")
        ("search-other",    po::VALUE<string_t>(), "search other files")
        ("search-document", po::VALUE<string_t>(), "search document files")
        ("search-image",    po::VALUE<string_t>(), "search image files")
        ("search-folder",   po::VALUE<string_t>(), "search folder files")

        ("node",    po::value<std::string>(), "select the current node by id")
        ("tree",    po::value<bool>()->implicit_value(true), "print the current node tree")
        ("ls",      po::value<bool>()->implicit_value(true), "print the current node children")
        ("mkdir",   po::VALUE<string_t>(), "create a directory under the current node")
        ("upload",  po::VALUE<string_t>(), "file path")
        ("download",po::VALUE<string_t>(), "folder path")
        ("continue",po::value<bool>()->implicit_value(false), "continue the download")

        ("user",    po::value<uint64_t>(), "select the current user by id")
        ("invite",  po::value<bool>()->implicit_value(true), "invite the current user")
        ("block",   po::value<bool>()->implicit_value(true), "block the current user")
        ("accept",  po::value<bool>()->implicit_value(true), "accept the current user invitation")
        ;

#undef VALUE

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        if (vm.count("version")) {
            std::cout << "v" << GIGA_VERSION << "\tbuild-" << GIGA_BUILD << "\n";
            return 0;
        }
        po::notify(vm);

        auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

        auto login = vm["login"].as<string_t>();
        string_t password;
        ucout << U("login: ") << login << U("\npassword ?") << std::endl;
        if (!vm.count("password"))
        {
            ucin >> password;
        }
        else
        {
            password = vm["password"].as<string_t>();
        }

        auto owner = app.authenticate(login, password);
        ucout << U("Logged as: ") << owner.login()
                << U(", root: ") << utils::str2wstr(owner.contactData().node().id())
                << std::endl;

        //
        // NETWORK
        //

        if (vm.count("list-contact"))
        {
            auto users = app.getContacts();
            printUsers("list-contact", users);
        }
        if (vm.count("list-received-invits"))
        {
            auto users = app.getInvitingUsers();
            printUsers("list-received-invits", users);
        }
        if (vm.count("list-sent-invits"))
        {
            auto users = app.getInvitedUsers();
            printUsers("list-sent-invits", users);
        }
        if (vm.count("list-blocked"))
        {
            auto users = app.getBlockedUsers();
            printUsers("list-blocked", users);
        }

        //
        // SEARCH
        //

        if (vm.count("search-user"))
        {
            auto users = app.searchUser(vm["search-user"].as<string_t>());
            printUsers("search-user", users);
        }

        searchNodes(vm, app, "search-video", core::Node::MediaType::video);
        searchNodes(vm, app, "search-audio", core::Node::MediaType::audio);
        searchNodes(vm, app, "search-other", core::Node::MediaType::unknown);
        searchNodes(vm, app, "search-document", core::Node::MediaType::document);
        searchNodes(vm, app, "search-image", core::Node::MediaType::image);
        searchNodes(vm, app, "search-folder", core::Node::MediaType::folder);

        //
        // NODE OP
        //

        if (vm.count("node"))
        {
            auto node = app.getNodeById(vm["node"].as<std::string>());
            if (vm.count("tree"))
            {
                ucout << "tree" << std::endl;
                printNodeTree(*node);
                ucout << std::endl;
            }
            if (vm.count("ls"))
            {
                printNodes("ls", node->getChildren());
            }
            if (vm.count("mkdir"))
            {
                auto dir = node->addChildFolder(vm["mkdir"].as<string_t>());
                auto arr = std::array<core::Node*, 1>{&dir};
                printNodes("mkdir", arr);
            }
            if (vm.count("upload"))
            {
                if (node->type() == core::Node::Type::file)
                {
                    BOOST_THROW_EXCEPTION(ErrorException{ U("Node must be a folder")});
                }
                auto currentNodeName = string_t{};
                core::Uploader uploader {
                    *static_cast<core::FolderNode*>(node.get()),
                    vm["upload"].as<string_t>(),
                    [&currentNodeName](core::FileUploader& fd, uint64_t count, uint64_t) {
                        if (currentNodeName != fd.nodeName())
                        {
                            currentNodeName = fd.nodeName();
                            ucout << std::endl;
                        }
                        else
                        {
                            (ucout << "                                                      \r").flush();
                        }
                        ucout << count << " "
                                  << std::setprecision(3) << fd.progress().percent() << "% - "
                                  << fd.nodeName();
                    }
                };
                uploader.start().get();

                auto uploads = uploader.uploadingFiles();
                std::vector<std::shared_ptr<core::Node>> arr(uploads.size());
                std::transform(uploads.begin(), uploads.end(), arr.begin(), [](const std::shared_ptr<core::FileUploader> u) {
                    return u->task().get();
                });
                printNodes("uploaded", arr);
            }
            if (vm.count("download"))
            {
                auto nbFiles   = node->nbFiles() + (node->type() == core::Node::Type::file ? 1 : 0);
                auto totalSize = node->size();

                ucout << std::endl;
                core::Downloader dl {
                    std::move(node),
                    vm["download"].as<string_t>(),
                    [nbFiles, totalSize](core::FileDownloader& fd, uint64_t count, uint64_t size) {
                        auto percent = ((double) size * 100) / (double) totalSize;
                        (ucout << "                                                      \r").flush();
                        ucout << count << "/" << nbFiles << " "
                                  << std::setprecision(3) << percent << "% - "
                                  << fd.destinationFile().filename().native();
                    }
                };
                dl.start().wait();
                ucout << "\nFile downloaded: " << dl.downloadingFileNumber() << std::endl;
            }
        }

        //
        // USER OP
        //

        if (vm.count("user"))
        {
            auto user = app.getUserById(vm["user"].as<uint64_t>());
            if (vm.count("invite"))
            {
                ucout << "invite" << std::endl;
                user.invite();
            }
            if (vm.count("block"))
            {
                ucout << "block" << std::endl;
                user.block();
            }
            if (vm.count("accept"))
            {
                ucout << "accept" << std::endl;
                user.acceptInvitation();
            }
        }
        ucout << "DONE" << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception!" << std::endl <<
        boost::current_exception_diagnostic_information();
        return 1;
    }
    return 0;
}
