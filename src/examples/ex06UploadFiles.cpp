#include <giga/Application.h>
#include <giga/core/Uploader.h>
#include <cpprest/details/basic_types.h>

using giga::Application;
using giga::core::Uploader;
using boost::filesystem::path;

using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));
    auto owner = app.authenticate(U("test_main"), U("password"));
    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;


    // Create a FolderNode named "upload" where we will upload some files
    auto uploadFolder = owner.contactData().node().addChildFolder(U("upload"));

    // WARNING :
    // We will upload the content of the current folder
    // make sure it contains data you want to upload !
    Uploader uploader{uploadFolder, path{U("./src")}};
    uploader.start().wait();

    return 0;
}
