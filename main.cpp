#include <boost/process.hpp>
#include <boost/process/async.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <algorithm>
#include <functional>
#include <exception>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>


#include "endian/bree_endian.h"
#include "hash/sha256.h"

namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace po = boost::program_options;

enum platform_var
{
    none = 0,
    linux,
    mac,
    windows
};
enum file_type
{
    unknown = 0,
    header,
    cpp,
    cc
};
struct args_container
{
public:
    std::string m_section_path;
    std::string m_platform;
    std::string m_cflags;
    std::string m_cxxflags;
    std::string m_cppflags;
    std::string m_ldflags;
    std::string m_ldlibs;
    args_container() : m_section_path(""), m_platform(""), m_cflags(""), m_cxxflags(""),
        m_cppflags(""), m_ldflags(""), m_ldlibs("") {}
    ~args_container() {}
};
struct file_config
{
    file_type m_file_type;
    fs::path m_path;
    fs::path m_obj_path;
    file_config(file_type a_file_type,
                fs::path &a_path,
                fs::path &a_obj_path) : m_file_type(a_file_type), m_path(a_path),
        m_obj_path(a_obj_path)
    {
    }
    ~file_config() {}
};
struct config_container
{
    platform_var platform_variant;
    fs::path m_workspace_root;
    fs::path m_section_root;
    fs::path m_config_path;
    fs::path m_object_path;
    fs::path m_cxx;
    fs::path m_cc;
    uint8_t m_max_threads;
    std::string m_section_name;
    std::string m_build_variant;
    std::vector<std::string> m_cflags;
    std::vector<std::string> m_cxxflags;
    std::vector<std::string> m_cppflags;
    std::vector<std::string> m_ldflags;
    std::vector<std::string> m_ldlibs;
    std::string m_out;
    std::vector<file_config> m_file_list;
    config_container() : platform_variant(platform_var::none),
        m_workspace_root(), m_section_root(), m_config_path(), m_object_path(), m_cxx(), m_cc(),
        m_max_threads(1), m_section_name(""), m_build_variant("default"), m_cflags(), m_cxxflags(),
        m_cppflags(), m_ldflags(), m_ldlibs(), m_out(""), m_file_list() {}
    ~config_container() {}
};

enum output_type
{
    output_type_out = 0,
    output_type_err = 1
};

struct compile_object
{
    std::vector<std::string> m_args;
    pr::async_pipe out;
    pr::async_pipe err;
    boost::asio::streambuf out_buffer;
    boost::asio::streambuf err_buffer;
    fs::path&source_path;
    pr::child m_process;
    std::vector<std::pair<output_type,std::string>> m_log;
    compile_object(boost::asio::io_context&ioc,fs::path&a_source_path,std::error_code &a_ec, fs::path &a_compiler_path,
                   std::vector<std::string> &&a_args) : m_args(std::move(a_args)),
        out(ioc),err(ioc),out_buffer(),err_buffer(),source_path(a_source_path),
        m_process(a_compiler_path, pr::args(m_args),pr::std_err > err,
                  pr::std_out > out, a_ec),m_log() {}
    virtual ~compile_object() {}
};

static void parse_flags(std::string &a_cmd_line, std::vector<std::string> &a_cfa);
static bool configure_objects(config_container &a_cc, std::map<std::string, std::string> &file_mod_list,std::map<std::string, std::string> &file_mod_list_update,
                              std::map<std::string, std::map<std::string, std::string>> &dependencty_list, bool remove_objects = true, bool only_modified = false,bool silent = false,std::string*single_file_compile = nullptr);
static bool compile_begin(boost::asio::io_context& ioc
                          ,config_container &a_cc,std::vector<compile_object *>&process_list,bool diagnostics_color);

static void init_options(po::options_description &, po::options_description &,
                         po::options_description &, po::options_description &,
                         args_container &, config_container &);

static bool ls_path(fs::path &, std::function<bool(fs::path &)>);
static void on_string_arg(std::string, std::string &);
static void on_path_arg(std::string, fs::path &);
static void on_uint8_arg(int, uint8_t &);

static bool check_file(fs::path &a_path, std::string a_err_1, std::string a_err_2);
static bool check_directory(fs::path &a_path, std::string a_err_1, std::string a_err_2, bool a_create = true);

static void fill_mod_list(config_container &a_cc, std::map<std::string, std::string> &file_mod_list)
{
    fs::path mods_path = a_cc.m_config_path;
    mods_path.remove_filename().append("file_mods.zn");

    if(fs::exists(fs::status(mods_path)))
    {
        std::ifstream stream(mods_path,std::ios_base::binary);
        if(!stream)
        {
            std::cout << "Unable to open mods file: " << mods_path << std::endl;
        }
        stream.seekg(0,stream.end);
        uint32_t file_size = stream.tellg();
        stream.seekg(0.,stream.beg);

        if(file_size < 4)
        {
            std::cout << "Corrupted mod file: " << mods_path << std::endl;
            return;
        }

        uint32_t pairs_count = 0;
        bree_readUInt32LE(stream,pairs_count);
        if(file_size - 4 != pairs_count * 64)
        {
            std::cout << "Corrupted mod file: " << mods_path << std::endl;
            return;
        }
        for(uint32_t i=0; i<pairs_count; i++)
        {
            std::string path_hash = "";
            std::string content_hash = "";
            path_hash.resize(32);
            content_hash.resize(32);
            if(!stream.read((char*)path_hash.data(),32))
            {
                stream.close();
                std::cout << "unable to read mods file properly" << std::endl;
                return;
            }
            if(!stream.read((char*)content_hash.data(),32))
            {
                stream.close();
                std::cout << "unable to read mods file properly" << std::endl;
                return;
            }
            file_mod_list.insert(std::pair<std::string,std::string>(path_hash,content_hash));
        }
        stream.close();
///a_cc.m_config_path
    }
}
static bool get_file_hash(fs::path &a_path,sha256_ctx&ctx,sha256_hash&a_hash)
{
    sha256_init(ctx);
    std::ifstream stream(a_path,std::ios_base::binary);
    if(!stream)
    {
        std::cout << "Could not open read access to file: " << a_path << std::endl;
        return false;
    }
    stream.seekg(0,stream.end);
    uint32_t l_ssize = stream.tellg();
    stream.seekg(0,stream.beg);
    uint8_t*l_array = new uint8_t[l_ssize+1];
    if(!stream.read((char*)l_array,l_ssize))
    {
        std::cout << "Could not read file: " << a_path << std::endl;
        stream.close();
        delete[]l_array;
        return false;
    }

    stream.close();
    sha256_update(&ctx,l_array,l_ssize);
    sha256_finalize(&ctx,a_hash);
    delete[]l_array;
    return true;
}
static bool fill_dependency_list(config_container &a_cc, std::map<std::string,std::string>&file_mod_list_update,std::map<std::string, std::map<std::string, std::string>> &dependency_list);

static void read_output(compile_object*a_co,const boost::system::error_code &ec, std::size_t size)
{

    if(!ec)
    {
        if(size)
        {
            std::string line = "";
            std::istream is(&a_co->out_buffer);
            getline(is,line);
            a_co->m_log.emplace_back(output_type_out,line);
        }

        boost::asio::async_read_until(a_co->out, a_co->out_buffer,"\n",std::bind(&read_output,a_co,std::placeholders::_1,std::placeholders::_2));
    }
    else
    {
        if(a_co->out_buffer.size())
        {
            std::string line = "";
            std::istream is(&a_co->out_buffer);
            while(getline(is,line))
            {
                a_co->m_log.emplace_back(output_type_out,line);
            }
        }
    }

}
static void read_error(compile_object*a_co,const boost::system::error_code &ec, std::size_t size)
{


    if(!ec)
    {
        if(size)
        {
            std::string line = "";
            std::istream is(&a_co->err_buffer);
            getline(is,line);
            a_co->m_log.emplace_back(output_type_err,line);
        }

        boost::asio::async_read_until(a_co->err, a_co->err_buffer,"\n",std::bind(&read_error,a_co,std::placeholders::_1,std::placeholders::_2));
    }
    else
    {
        if(a_co->err_buffer.size())
        {

            std::string line = "";
            std::istream is(&a_co->err_buffer);
            while(getline(is,line))
            {
                a_co->m_log.emplace_back(output_type_err,line);
            }
        }
    }
}
int main(int argc, char const *argv[])


{

    po::variables_map vm;
    config_container cc;
    {
        args_container ac;
        std::map<std::string, platform_var> platforms{{"linux", platform_var::linux}, {"mac", platform_var::mac}, {"win32", platform_var::windows}};
        try
        {
            /// platform, release type, init
            po::options_description required{"Required Arguments"};
            po::options_description config{"Configuration"};
            po::options_description compile{"Compile"};
            po::options_description generic{"Generic Arguments"};

            init_options(required, config, compile, generic, ac, cc);
            generic.add(required).add(config).add(compile);
            po::store(po::parse_command_line(argc, argv, generic), vm);

            if (vm.count("help"))
            {
                std::cout << generic << '\n';
                return EXIT_SUCCESS;
            }

            if (vm.count("version"))
            {
                std::cout << "1.0.0" << '\n';
                return EXIT_SUCCESS;
            }

            notify(vm);
        }
        catch (const po::error &ex)
        {
            std::cerr << ex.what() << '\n';
            return EXIT_FAILURE;
        }
        cc.m_config_path = cc.m_workspace_root;

        cc.m_config_path.append(".zninja");

        {
            /// extract section name
            size_t pos = ac.m_section_path.find(cc.m_config_path.separator);
            if (pos != std::string::npos)
            {
                cc.m_section_name = ac.m_section_path.substr(0, pos);
            }
            else
            {
                cc.m_section_name = ac.m_section_path;
            }

            if (cc.m_section_name.size() == 0)
            {

                std::cout << "--section argument is invalid, should contain section name or relative path from --workspace-root: " << ac.m_section_path << std::endl;
                return EXIT_FAILURE;
            }
        }
        {
            std::transform(ac.m_platform.begin(), ac.m_platform.end(), ac.m_platform.begin(), [&](unsigned char c) -> unsigned char
            { return std::tolower(c); });
            auto plat = platforms.find(ac.m_platform);
            if (platforms.end() == plat)
            {

                std::cout << "Unknown platform " << ac.m_platform << std::endl;
                std::cout << "Allowed platforms are: " << std::endl;
                for (auto it = platforms.begin(); it != platforms.end(); it++)
                {
                    std::cout << "- " << it->first << std::endl;
                }

                return EXIT_FAILURE;
            }
            else
            {

                cc.platform_variant = plat->second;
            }
        }

        if (!check_directory(cc.m_workspace_root, "--workspace-root  must exists", "--workspace-root must be directory"))
        {

            return EXIT_FAILURE;
        }

        if (!cc.m_workspace_root.is_absolute())
        {

            std::cout << "--workspace-root must be an absolute path" << std::endl;
            return EXIT_FAILURE;
        }

        if (!check_directory(cc.m_config_path, "Unable to create config root directory ",
                             "Config path must be an directory"))
        {

            return EXIT_FAILURE;
        }

        {
            cc.m_config_path.append(cc.m_section_name);
            for (auto it = platforms.begin(); it != platforms.end(); it++)
            {
                fs::path platform_conf_dir(cc.m_config_path);
                platform_conf_dir.append(it->first);

                if (!check_directory(platform_conf_dir, "Unable to create config directory for platform ", "Platform config location must be an directory: "))
                {
                    return EXIT_FAILURE;
                }
            }
        }
        cc.m_config_path.append(ac.m_platform);

        cc.m_section_root = cc.m_workspace_root;
        cc.m_section_root.append(cc.m_section_name);

        if (!check_directory(cc.m_section_root, "Section Directory not found: ",
                             "Section path must be a directory", false))
        {

            return EXIT_FAILURE;
        }

        cc.m_object_path = cc.m_config_path;
        cc.m_object_path.append("obj");
        fs::path bv_path = vm["build-variant"].as<std::string>();
        cc.m_build_variant = bv_path.filename().string();
        cc.m_object_path.append(cc.m_build_variant);
        if (!check_directory(cc.m_object_path, "Could not create object directory: ",
                             "Object path must be a directory: "))
        {
            return EXIT_FAILURE;
        }
        cc.m_config_path.append(cc.m_build_variant);

        if (!check_directory(cc.m_config_path, "Unable to create config directory for build variant ", "Build variant config path must be an directory: "))
        {
            return EXIT_FAILURE;
        }
        cc.m_config_path.append("config.zn");

        if (fs::exists(fs::status(cc.m_config_path)) && !fs::is_regular_file(cc.m_config_path))
        {

            std::cout << "Config file must not be a directory: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }

        if (vm.count("cxx"))
        {
            if (!cc.m_cxx.is_absolute())
            {
                fs::path cxx_path = pr::search_path(cc.m_cxx);
                if (fs::exists(fs::status(cxx_path)))
                {
                    cc.m_cxx = cxx_path;
                }
            }
            if (!check_file(cc.m_cxx, "Could not find C++ compiler at path: ", "Compailer path, must be an executable: "))
            {

                return EXIT_FAILURE;
            }
        }
        if (vm.count("cc"))
        {
            if (!cc.m_cc.is_absolute())
            {
                fs::path cc_path = pr::search_path(cc.m_cc);
                if (fs::exists(fs::status(cc_path)))
                {
                    cc.m_cc = cc_path;
                }
            }
            if (!check_file(cc.m_cc, "Could not find C compiler at path: ", "Compailer path, must be an executable: "))
            {
                return EXIT_FAILURE;
            }
        }

        if (vm.count("cflags"))
        {
            parse_flags(ac.m_cflags, cc.m_cflags);
        }
        if (vm.count("cxxflags"))
        {
            parse_flags(ac.m_cxxflags, cc.m_cxxflags);
        }
        if (vm.count("cppflags"))
        {
            parse_flags(ac.m_cppflags, cc.m_cppflags);
        }
        if (vm.count("ldflags"))
        {
            parse_flags(ac.m_ldflags, cc.m_ldflags);
        }
        if (vm.count("ldlibs"))
        {
            parse_flags(ac.m_ldlibs, cc.m_ldlibs);
        }
        if (!vm.count("out"))
        {
            fs::path out_path = cc.m_workspace_root;
            out_path.append("bin");
            out_path.append(ac.m_platform);
            out_path.append(cc.m_build_variant);
            if (!check_directory(out_path, "Unable to create output directory at location: ", "Output location must be a directory"))
            {

                return EXIT_FAILURE;
            }
            /// check for platform dependent extension name
            out_path.append(cc.m_section_name); // executable name
            cc.m_out = out_path.string();
        }
        else
        {
            fs::path out_path = cc.m_out;
            if(!out_path.has_filename())/// check if filename will be positive for dot
            {
                if (!check_directory(out_path, "Unable to create output directory at location: ", "Output location must be a directory"))
                {

                    return EXIT_FAILURE;
                }
                out_path.append(cc.m_section_name);
                cc.m_out = out_path.string();
            }

        }

    } // end of initial scope


    if (vm.count("save"))
    {
        std::cout << "Saving configuration" << std::endl;
        std::ofstream stream(cc.m_config_path, std::ios_base::binary|std::ios_base::trunc);
        if (!stream)
        {
            std::cout << "Unable to open config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }

        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_cc.string().size()));// 4
        stream << cc.m_cc.string();
        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_cxx.string().size()));// 4
        stream << cc.m_cxx.string();
        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_cflags.size()));// 4
        for(auto it = cc.m_cflags.begin(); it!=cc.m_cflags.end(); it++)
        {
            bree_writeUInt32LE(stream,static_cast<uint32_t>((*it).size()));
            stream << (*it);

        }

        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_cxxflags.size()));// 4
        for(auto it = cc.m_cxxflags.begin(); it!=cc.m_cxxflags.end(); it++)
        {
            bree_writeUInt32LE(stream,static_cast<uint32_t>((*it).size()));
            stream << (*it);
        }
        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_cppflags.size()));// 4
        for(auto it = cc.m_cppflags.begin(); it!=cc.m_cppflags.end(); it++)
        {
            bree_writeUInt32LE(stream,static_cast<uint32_t>((*it).size()));
            stream << (*it);
        }

        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_ldflags.size()));// 4
        for(auto it = cc.m_ldflags.begin(); it!=cc.m_ldflags.end(); it++)
        {
            bree_writeUInt32LE(stream,static_cast<uint32_t>((*it).size()));
            stream << (*it);
        }

        bree_writeUInt32LE(stream,static_cast<uint32_t>(cc.m_ldlibs.size()));/// 4
        for(auto it = cc.m_ldlibs.begin(); it!=cc.m_ldlibs.end(); it++)
        {
            bree_writeUInt32LE(stream,static_cast<uint32_t>((*it).size()));
            stream << (*it);
        }

        /// do save config, move impl to function
        stream.close();
    }
    if (vm.count("load"))
    {
        std::cout << "Loading configuration..." << std::endl << std::endl;
        std::ifstream stream(cc.m_config_path, std::ios_base::binary);
        if (!stream)
        {
            std::cout << "Unable to open config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        stream.seekg(0,stream.end);
        uint32_t l_file_size = stream.tellg();
        stream.seekg(0,stream.beg);
        if(l_file_size < 28)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        uint32_t l_size = 0;
        std::string l_str = "";
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }

        l_str.resize(l_size);

        if(l_size > 0)
        {
            if(!stream.read((char*)l_str.data(),l_size))
            {
                stream.close();
                std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                return EXIT_FAILURE;
            }
        }
        {
            fs::path l_cc = l_str;
            if (!l_cc.is_absolute())
            {
                fs::path cc_path = pr::search_path(l_cc);
                if (fs::exists(fs::status(cc_path)))
                {
                    l_cc = cc_path;
                }
            }
            if (!check_file(l_cc, "Could not find configureed C compiler at path: ", "Compailer path, must be an executable: "))
            {
                if(vm.count("cc"))
                {
                    std::cout << "Using supplied compiler by --cc argument: " << cc.m_cc << std::endl;

                }
                else
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                cc.m_cc = l_cc;
            }

        }

        l_str = "";
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        l_str.resize(l_size);

        if(l_size > 0)
        {
            if(!stream.read((char*)l_str.data(),l_size))
            {
                stream.close();
                std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                return EXIT_FAILURE;
            }
        }

        {
            fs::path l_cxx = l_str;
            if (!l_cxx.is_absolute())
            {
                fs::path cxx_path = pr::search_path(l_cxx);
                if (fs::exists(fs::status(cxx_path)))
                {
                    l_cxx = cxx_path;
                }
            }
            if (!check_file(l_cxx, "Could not find configureed C++ compiler at path: ", "Compailer path, must be an executable: "))
            {
                if(vm.count("cxx"))
                {
                    std::cout << "Using supplied compiler by --cxx argument: " << cc.m_cxx << std::endl;
                }
                else
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                cc.m_cxx = l_cxx;
            }
        }
        cc.m_cflags.clear();
        cc.m_cxxflags.clear();
        cc.m_cppflags.clear();
        cc.m_ldflags.clear();
        cc.m_ldlibs.clear();
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        for(uint32_t i = 0; i<l_size; i++)
        {
            uint32_t f_size = 0;
            bree_readUInt32LE(stream,f_size);
            if(!stream)
            {
                stream.close();
                std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                return EXIT_FAILURE;
            }
            if(f_size>0)
            {
                l_str ="";
                l_str.resize(f_size);
                stream.read((char*)l_str.data(),f_size);

                cc.m_cflags.push_back(l_str);
            }
        }

        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        for(uint32_t i = 0; i<l_size; i++)
        {
            uint32_t f_size = 0;
            bree_readUInt32LE(stream,f_size);
            if(f_size>0)
            {
                l_str ="";
                l_str.resize(f_size);
                if(!stream.read((char*)l_str.data(),f_size))
                {
                    stream.close();
                    std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                    return EXIT_FAILURE;
                }
                cc.m_cxxflags.push_back(l_str);
            }
        }
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        for(uint32_t i = 0; i<l_size; i++)
        {
            uint32_t f_size = 0;
            bree_readUInt32LE(stream,f_size);
            if(f_size>0)
            {
                l_str ="";
                l_str.resize(f_size);
                if(!stream.read((char*)l_str.data(),f_size))
                {
                    stream.close();
                    std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                    return EXIT_FAILURE;
                }
                cc.m_cppflags.push_back(l_str);
            }
        }
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        for(uint32_t i = 0; i<l_size; i++)
        {
            uint32_t f_size = 0;
            bree_readUInt32LE(stream,f_size);
            if(f_size>0)
            {
                l_str ="";
                l_str.resize(f_size);
                if(!stream.read((char*)l_str.data(),f_size))
                {
                    stream.close();
                    std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                    return EXIT_FAILURE;
                }
                cc.m_ldflags.push_back(l_str);
            }
        }
        bree_readUInt32LE(stream,l_size);
        if(!stream)
        {
            stream.close();
            std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
            return EXIT_FAILURE;
        }
        for(uint32_t i = 0; i<l_size; i++)
        {
            uint32_t f_size = 0;
            bree_readUInt32LE(stream,f_size);
            if(f_size>0)
            {
                l_str ="";
                l_str.resize(f_size);
                if(!stream.read((char*)l_str.data(),f_size))
                {
                    stream.close();
                    std::cout << "Corrupted config file: " << cc.m_config_path << std::endl;
                    return EXIT_FAILURE;
                }
                cc.m_ldlibs.push_back(l_str);
            }
        }
        stream.close();
    }
    else
    {
        if (!vm.count("cxx"))
        {
            if (!check_file(cc.m_cxx, "Could not find C++ compiler at path: ", "Compailer path, must be an executable: "))
            {

                return EXIT_FAILURE;
            }
        }
        if (!vm.count("cc"))
        {
            if (!check_file(cc.m_cc, "Could not find C compiler at path: ", "Compailer path, must be an executable: "))
            {
                return EXIT_FAILURE;
            }
        }
    }
    if (vm.count("show"))
    {
        std::cout << "COMPILERS" << std::endl;
        std::cout << "C:   " << cc.m_cc << std::endl;
        std::cout << "C++: " << cc.m_cxx << std::endl
                  << std::endl;

        std::cout << "CFLAGS ( " << cc.m_cflags.size() << " )" << std::endl;
        for (auto it = cc.m_cflags.begin(); it != cc.m_cflags.end(); it++)
        {
            std::cout << (*it) << std::endl;
        }
        std::cout << std::endl
                  << "CXXFLAGS ( " << cc.m_cxxflags.size() << " )" << std::endl;
        for (auto it = cc.m_cxxflags.begin(); it != cc.m_cxxflags.end(); it++)
        {
            std::cout << (*it) << std::endl;
        }

        std::cout << std::endl
                  << "CPPFLAGS ( " << cc.m_cppflags.size() << " )" << std::endl;
        for (auto it = cc.m_cppflags.begin(); it != cc.m_cppflags.end(); it++)
        {
            std::cout << (*it) << std::endl;
        }

        std::cout << std::endl
                  << "LDFLAGS ( " << cc.m_ldflags.size() << " )" << std::endl;
        for (auto it = cc.m_ldflags.begin(); it != cc.m_ldflags.end(); it++)
        {
            std::cout << (*it) << std::endl;
        }

        std::cout << std::endl
                  << "LDLIBS ( " << cc.m_ldlibs.size() << " )" << std::endl;
        for (auto it = cc.m_ldlibs.begin(); it != cc.m_ldlibs.end(); it++)
        {
            std::cout << (*it) << std::endl;
        }

        std::cout << std::endl
                  << std::endl;
    }
    std::map<std::string, std::string> file_mod_list;
    std::map<std::string, std::string> file_mod_list_new;
    std::map<std::string, std::map<std::string, std::string>> dependency_list;

    fill_mod_list(cc, file_mod_list);
    fill_dependency_list(cc, file_mod_list_new,dependency_list);

    if (vm.count("clean"))
    {
        /// delete all object file for section
        if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list,true,false,true))
        {
            return EXIT_FAILURE;
        }
        cc.m_file_list.clear();
    }


    if (vm.count("build-file"))
    {
        std::string build_file = vm["section"].as<std::string>();
        std::size_t pos = build_file.find(cc.m_section_name);
        if(pos != std::string::npos)
        {
            build_file = build_file.substr(pos+cc.m_section_name.size());
            if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list,true,false,false,&build_file))
            {
                return EXIT_FAILURE;
            }

        }
    }
    else if (vm.count("build"))
    {
        if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list, true, true))
        {
            return EXIT_FAILURE;
        }
    }
    else if (vm.count("rebuild"))
    {
        if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list,true,false))
        {
            return EXIT_FAILURE;
        }
    }

    /// acutally building objects if there is condition to build any file
    auto start = std::chrono::steady_clock::now();
    {

        if (cc.m_file_list.size())
        {
            if (!check_file(cc.m_cc, "Could not found C compiler on path: ", "Compailer path, must be an executable: "))
            {
                return EXIT_FAILURE;
            }
            if (!check_file(cc.m_cxx, "Could not found C++ compiler on path: ", "Compailer path, must be an executable: "))
            {
                return EXIT_FAILURE;
            }

            std::vector<compile_object *>process_list;
            boost::asio::io_context ioc;

            if (!compile_begin(ioc,cc,process_list,vm.count("diagnostics-color")))
            {
                while (process_list.size() > 0)
                {
                    while(ioc.poll()) {}
                    for (auto itp = process_list.begin(); itp != process_list.end();)
                    {
                        std::error_code ec;
                        if ((*itp)->m_process.running(ec))
                        {
                            ++itp;
                        }
                        else
                        {
                            if (ec)
                            {
                                std::cout << "Process running status error: " << ec.message() << std::endl;/// should never happen?
                                return EXIT_FAILURE;
                            }
                            ec = std::error_code();
                            (*itp)->m_process.wait(ec);
                            if (ec)
                            {
                                std::cout << "Wait process error: " << ec.message() << std::endl;/// should never happen?
                                return EXIT_FAILURE;
                            }
                            (*itp)->m_process.exit_code();
                            for(auto it = (*itp)->m_log.begin(); it!= (*itp)->m_log.end(); it++)
                            {
                                if(it->first == output_type_err)
                                {
                                    std::cerr << it->second << std::endl;
                                }
                                else
                                {
                                    std::cout << it->second << std::endl;
                                }
                            }
                            delete (*itp);
                            itp = process_list.erase(itp);
                        }
                    }
                }
                while(ioc.poll()) {}
                auto end = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                std::cout << "Compilation failed, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
                return EXIT_FAILURE;
            }
            {
                auto end = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                std::cout << "Compilation successfull, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
                cc.m_config_path.remove_filename().append("file_mods.zn");

                std::ofstream stream(cc.m_config_path,std::ios_base::binary|std::ios_base::trunc);
                if(!stream)
                {
                    std::cout << "Unable to open mods file: " << cc.m_config_path << std::endl;
                    return EXIT_FAILURE;
                }
                bree_writeUInt32LE(stream,static_cast<uint32_t>(file_mod_list_new.size()));
                for(auto it = file_mod_list_new.begin(); it != file_mod_list_new.end(); it++)
                {
                    stream << it->first << it->second;
                }
                stream.close();
                std::cout<< std::endl;
            }
        }
    }

    if (vm.count("link"))
    {
        if (!vm.count("rebuild"))
        {

            if(vm.count("build")||vm.count("build-file"))
            {
                if(cc.m_file_list.size() == 0)
                {
                    std::cout << "Link target is up to date!!" << std::endl;
                    return EXIT_SUCCESS;
                }
            }
            cc.m_file_list.clear();
            if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list, false,false,true))
            {
                return EXIT_FAILURE;
            }
            /// get all object names
        }
        for(auto ss = cc.m_ldflags.begin(); ss != cc.m_ldflags.end(); ss++)
        {
            if((*ss).compare("-shared") == 0)
            {
                fs::path out_path = cc.m_out;
                cc.m_out = out_path.filename().string();
                out_path.remove_filename().append("lib"+cc.m_out+".so");
                cc.m_out = out_path.string();
                break;
            }
        }
        auto start_link = std::chrono::steady_clock::now();

        std::cout << "Linking target: " << cc.m_out << std::endl << std::endl;
        std::vector<std::string> link_args;
        for(auto it = cc.m_ldflags.begin(); it!= cc.m_ldflags.end(); it++)
        {
            link_args.push_back((*it));
        }
        link_args.push_back("-o");
        link_args.push_back(cc.m_out);
        for(auto it = cc.m_file_list.begin(); it!= cc.m_file_list.end(); it++)
        {
            link_args.push_back((*it).m_obj_path.string());
        }
        for(auto it = cc.m_ldlibs.begin(); it!= cc.m_ldlibs.end(); it++)
        {
            link_args.push_back((*it));
        }
        std::cout << cc.m_cxx.string();
        for(auto it = link_args.begin(); it!=link_args.end(); it++)
        {
            std::cout <<" " << (*it);
        }
        if(vm.count("diagnostics-color"))
        {
            link_args.push_back("-fdiagnostics-color=always");
        }
        std::cout << std::endl <<  std::endl;;
        std::error_code ec;
        pr::child c(cc.m_cxx,pr::args(link_args),ec);

        if(ec)
        {
            std::cout << "Link process start error: " << ec.message() << std::endl;
        }
        c.wait();
        int exit_code = c.exit_code();

        if(exit_code != EXIT_SUCCESS)
        {
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_link);
            auto elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Link failed, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
            std::cout << "Total time elapsed: " << elapsed_total.count() << " milliseconds" << std::endl;
            return EXIT_FAILURE;
        }


        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_link);
        auto elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Link successfull, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
        std::cout << "Total time elapsed: " << elapsed_total.count() << " milliseconds" << std::endl;
        fs::path out_file = cc.m_out;
        uintmax_t  file_size = fs::file_size(out_file);
        uintmax_t kb = file_size / 1024;
        uintmax_t mb = 0;
        if(kb > 1024)
        {
            mb = kb / 1024;
            kb %= 1024;
        }
        std::cout << "Output file is: " << cc.m_out << " with size " << mb << " MB " << kb <<" KB" << std::endl;
        /// cxx LDFLAGS -o executable  objects list.... LDLIBS

        /// or for static ar rcs libout.a out.o
        /* microsoft

        https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-static-library-cpp?view=msvc-160

        When you build on the Visual Studio command line,
        you must build the program in two steps.
        First, run cl /c /EHsc MathLibrary.cpp to compile the
        code and create an object file that's named MathLibrary.obj.
        (The cl command invokes the compiler, Cl.exe, and the /c option
        specifies compile without linking. For more information,
        see /c (Compile Without Linking).) Second, run lib MathLibrary.obj
        to link the code and create the static library MathLibrary.lib.
        (The lib command invokes the Library Manager, Lib.exe. For more
        information, see LIB Reference.)

         */
    }
    else if(vm.count("build-static-lib"))
    {
        if (!vm.count("rebuild"))
        {

            if(vm.count("build")||vm.count("build-file"))
            {
                if(cc.m_file_list.size() == 0)
                {
                    std::cout << "Static target is up to date!!" << std::endl;
                    return EXIT_SUCCESS;
                }
            }
            cc.m_file_list.clear();
            if (!configure_objects(cc, file_mod_list,file_mod_list_new, dependency_list, false,false,true))
            {
                return EXIT_FAILURE;
            }
            /// get all object names
        }
        {
            fs::path out_path = cc.m_out;
            cc.m_out = out_path.filename().string();
            out_path.remove_filename().append("lib"+cc.m_out+".a");
            cc.m_out = out_path.string();
        }

        cc.m_cc.remove_filename().append("ar");
        if(!fs::exists(fs::status(cc.m_cc)))
        {
            std::cout <<"Archiver for static library not found at: " << cc.m_cc << std::endl;
            return EXIT_FAILURE;
        }

        auto start_link = std::chrono::steady_clock::now();

        //std::cout << "Linking target: " << cc.m_out << std::endl << std::endl;
        std::vector<std::string> link_args;

        link_args.push_back("rvs");
        link_args.push_back(cc.m_out);
        for(auto it = cc.m_file_list.begin(); it!= cc.m_file_list.end(); it++)
        {
            link_args.push_back((*it).m_obj_path.string());
        }

        std::cout << cc.m_cc.string();
        for(auto it = link_args.begin(); it!=link_args.end(); it++)
        {
            std::cout <<" " << (*it);
        }

        std::cout << std::endl <<  std::endl;;
        std::error_code ec;
        pr::child c(cc.m_cc,pr::args(link_args),ec);

        if(ec)
        {
            std::cout << "Static Build process start error: " << ec.message() << std::endl;
        }
        c.wait();
        int exit_code = c.exit_code();

        if(exit_code != EXIT_SUCCESS)
        {
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_link);
            auto elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Link failed, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
            std::cout << "Total time elapsed: " << elapsed_total.count() << " milliseconds" << std::endl;
            return EXIT_FAILURE;
        }


        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_link);
        auto elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Link successfull, time consumed: " << elapsed.count() << " milliseconds" << std::endl;
        std::cout << "Total time elapsed: " << elapsed_total.count() << " milliseconds" << std::endl;
        fs::path out_file = cc.m_out;
        uintmax_t  file_size = fs::file_size(out_file);
        uintmax_t kb = file_size / 1024;
        uintmax_t mb = 0;
        if(kb > 1024)
        {
            mb = kb / 1024;
            kb %= 1024;
        }
        std::cout << "Output file is: " << cc.m_out << " with size " << mb << " MB " << kb <<" KB" << std::endl;

        /// or for static ar rcs libout.a out.o

    }

    return EXIT_SUCCESS;
}
static void parse_flags(std::string &a_cmd_line, std::vector<std::string> &a_cfa)
{
    std::string current_arg;
    std::stringstream stream(a_cmd_line);
    std::string arg;
    while (stream >> std::quoted(arg))
    {
        a_cfa.push_back(arg);
    }
}
static bool  check_file_mod(std::string file_hash,std::map<std::string, std::string> &file_mod_list,std::map<std::string, std::string> &file_mod_list_new, std::map<std::string,
                            std::map<std::string, std::string>> &dependency_list,std::map<std::string, bool>&checked_list)
{
    auto cl = checked_list.find(file_hash);
    if(cl != checked_list.end())
    {
        return cl->second;
    }
    auto fml = file_mod_list.find(file_hash);
    if(fml == file_mod_list.end())
    {
        checked_list.insert(std::pair<std::string,bool>(file_hash,true));
        return true;
    }
    auto fmln = file_mod_list_new.find(file_hash);
    if(fmln != file_mod_list_new.end())
    {

        if(fmln->second.compare(fml->second)==0)
        {
            checked_list.insert(std::pair<std::string,bool>(file_hash,false));

            auto dp = dependency_list.find(file_hash);
            if(dp != dependency_list.end())
            {
                for(auto it = dp->second.begin(); it != dp->second.end(); it++)
                {

                    if(check_file_mod(it->first,file_mod_list,file_mod_list_new,dependency_list,checked_list))
                    {
                        return true;
                    }

                }

            }

            return false;
        }

    }
    checked_list.insert(std::pair<std::string,bool>(file_hash,true));
    return true;
}

static bool configure_objects(config_container &a_cc, std::map<std::string, std::string> &file_mod_list,
                              std::map<std::string, std::string> &file_mod_list_update,
                              std::map<std::string, std::map<std::string, std::string>> &dependency_list, bool remove_objects, bool only_modified, bool silent,std::string*single_file_compile)
{

    std::map<std::string, file_type> ext_map{{".c", file_type::cc}, {".cc", file_type::cpp}, {".cxx", file_type::cpp}, {".cpp", file_type::cpp}, {".h", file_type::header}, {".hpp", file_type::header},{".hxx", file_type::header}};
    if(!silent)
    {
        std::cout << "Workspace Root: " << a_cc.m_workspace_root << std::endl;
        std::cout << "Configuring Section: \"" << a_cc.m_section_name << "\" for build variant ( " << a_cc.m_build_variant << " )" << std::endl
                  << std::endl;
    }
    if (!ls_path(a_cc.m_section_root, [&](fs::path &a_path) -> bool
{
    if (fs::is_regular_file(a_path))
        {
            auto it = ext_map.find(a_path.extension().string());
            if (it != ext_map.end())
            {

                std::size_t pos = a_path.string().find(a_cc.m_section_root.string());

                if (pos != std::string::npos)
                {
                    if (it->second == file_type::header)
                    {

                        return true;
                    }
                    std::string rel_path = a_path.string().substr(pos + a_cc.m_section_root.string().size());
                    if(single_file_compile != nullptr)
                    {
                        if(rel_path.compare(*single_file_compile) != 0)
                        {
                            return true;
                        }
                    }


                    fs::path obj_path(a_cc.m_object_path);
                    obj_path.append(a_path.string().substr(pos + a_cc.m_section_root.string().size()));
                    obj_path.replace_extension(".o");

                    if (obj_path.parent_path() != a_cc.m_object_path)
                    {
                        fs::path parent_path = obj_path.parent_path();
                        if (!check_directory(parent_path, "Could not create object directory: ",
                                             "Object path must be a directory: "))
                        {
                            return false;
                        }
                    }


                    if (only_modified)
                    {

                        sha256_hash l_hash;
                        sha256_ctx ctx;
                        sha256_init(ctx);
                        sha256_update(&ctx,(uint8_t*)rel_path.data(),rel_path.size());
                        sha256_finalize(&ctx,l_hash);
                        rel_path = "";
                        rel_path.assign((char*)l_hash,32);
                        std::map<std::string, bool> checked_list;
                        if(!check_file_mod(rel_path,file_mod_list,file_mod_list_update,dependency_list,checked_list))
                        {
                            if (fs::exists(fs::status(obj_path)))
                            {
                                return true;
                            }
                        }

                    }
                    if(!silent)
                    {
                        std::cout << a_path << " will be compiled" << std::endl;
                    }
                    if (remove_objects)
                    {
                        if (fs::exists(fs::status(obj_path)))
                        {
                            boost::system::error_code ec;
                            fs::remove(obj_path, ec);
                            if (ec)
                            {
                                std::cout << "Could not remove old object file: " << obj_path << " ( " << ec.message() << " )" << std::endl;
                                return EXIT_FAILURE;
                            }
                        }
                    }
                    a_cc.m_file_list.emplace_back(it->second, a_path, obj_path);
                }
            }
        }
        return true;
    }))
    {

        return false;
    }

    if(!silent)
    {
        std::cout << std::endl;
    }
    return true;
}
static bool compile_begin(boost::asio::io_context& ioc
                          ,config_container &a_cc,std::vector<compile_object *>&process_list,bool diagnostics_color)
{
    uint8_t cpu_cores = static_cast<uint8_t>(std::thread::hardware_concurrency());
    if (cpu_cores < a_cc.m_max_threads)
    {
        std::cout << "Warning: Hardware suport ( " << static_cast<int>(cpu_cores) << " ) concurrent operations, you requested (" << static_cast<int>(a_cc.m_max_threads) << std::endl;
    }

    std::cout << "Compiling " << a_cc.m_file_list.size() <<" source files, threads limit: " << (int)a_cc.m_max_threads << std::endl << std::endl;
    /// c++ compiler
    /// c compiler


    std::size_t task_launched = 0;
    std::size_t task_finished = 0;
    std::string line = "";
    for (auto it = a_cc.m_file_list.begin(); it != a_cc.m_file_list.end(); it++)
    {
        if (a_cc.m_max_threads > static_cast<uint8_t>(process_list.size()))
        {
            ++task_launched;
            if (it->m_file_type == file_type::cc)
            {
                std::cout <<  "[ "<<task_launched << "/" << a_cc.m_file_list.size() << " ]" << a_cc.m_cc.string();
                std::vector<std::string> c_args;
                for(auto carg=a_cc.m_cflags.begin(); carg != a_cc.m_cflags.end(); carg++)
                {
                    c_args.push_back(*carg);
                }
                for(auto cpparg=a_cc.m_cppflags.begin(); cpparg != a_cc.m_cppflags.end(); cpparg++)
                {
                    c_args.push_back(*cpparg);
                }
                c_args.push_back("-c");
                c_args.push_back(it->m_path.string());
                c_args.push_back("-o");
                c_args.push_back(it->m_obj_path.string());
                for (auto ita = c_args.begin(); ita != c_args.end(); ita++)
                {
                    std::cout << " " << (*ita);
                }
                std::cout << std::endl << std::endl;
                if(diagnostics_color)
                {
                    c_args.push_back("-fdiagnostics-color=always");
                }
                std::error_code ec;
                compile_object *l_co = new compile_object(ioc,it->m_path,ec, a_cc.m_cc, std::move(c_args));
                if (ec)
                {
                    delete l_co; /// ?!
                    std::cout << "Process start error: " << ec.message() << std::endl;
                    return false;
                }
                //process_list.emplace_back(std::ref(ec), std::ref(cc.m_cc), std::move(c_args));
                boost::system::error_code bec;
                read_output(l_co,bec,0);
                read_error(l_co,bec,0);
                process_list.push_back(l_co);
            }
            else if (it->m_file_type == file_type::cpp)
            {

                std::cout << "[ "<<task_launched << "/" << a_cc.m_file_list.size() << " ] " << a_cc.m_cxx.string();
                std::vector<std::string> c_args;
                for(auto cxxarg=a_cc.m_cxxflags.begin(); cxxarg != a_cc.m_cxxflags.end(); cxxarg++)
                {
                    c_args.push_back(*cxxarg);
                }
                for(auto cpparg=a_cc.m_cppflags.begin(); cpparg != a_cc.m_cppflags.end(); cpparg++)
                {
                    c_args.push_back(*cpparg);
                }
                c_args.push_back("-c");
                c_args.push_back(it->m_path.string());
                c_args.push_back("-o");
                c_args.push_back(it->m_obj_path.string());
                for (auto ita = c_args.begin(); ita != c_args.end(); ita++)
                {
                    std::cout << " " << (*ita);
                }
                std::cout << std::endl << std::endl;
                if(diagnostics_color)
                {
                    c_args.push_back("-fdiagnostics-color=always");
                }
                std::error_code ec;
                compile_object *l_co = new compile_object(ioc,it->m_path,ec, a_cc.m_cxx, std::move(c_args));
                if (ec)
                {
                    std::cout << "process start error: " << ec.message() << std::endl;
                    delete l_co;
                    return false;
                }
                boost::system::error_code bec;
                read_output(l_co,bec,0);
                read_error(l_co,bec,0);
                process_list.push_back(l_co);
            }
            if (a_cc.m_file_list.size() != task_launched && a_cc.m_max_threads > static_cast<uint8_t>(process_list.size()))
            {
                continue;
            }
        }
        {

            while (process_list.size() > 0)
            {
                while(ioc.poll());
                for (auto itp = process_list.begin(); itp != process_list.end();)
                {
                    std::error_code ec;
                    if ((*itp)->m_process.running(ec))
                    {
                        (*itp)->m_process.wait_for(std::chrono::milliseconds(10));
                        ++itp;
                    }
                    else
                    {
                        if (ec)
                        {
                            std::cout << "Process running status error: " << ec.message() << std::endl;/// should never happen?
                            return false;
                        }
                        ec = std::error_code();
                        (*itp)->m_process.wait(ec);
                        if (ec)
                        {
                            std::cout << "Wait process error: " << ec.message() << std::endl;/// should never happen?
                            return false;
                        }
                        int exit_code = (*itp)->m_process.exit_code();
                        for(auto it = (*itp)->m_log.begin(); it!= (*itp)->m_log.end(); it++)
                        {
                            if(it->first == output_type_err)
                            {
                                std::cerr << it->second << std::endl;
                            }
                            else
                            {
                                std::cout << it->second << std::endl;
                            }
                        }
                        task_finished++;
                        if (exit_code == EXIT_SUCCESS)
                        {
                            std::cout << "[ "<<task_finished << "/" << a_cc.m_file_list.size() << " ] Successfull compilation of source file: " << (*itp)->source_path << std::endl;
                        }
                        else
                        {
                            std::cout << "[ "<<task_finished << "/" << a_cc.m_file_list.size() << " ] Compilation fail, source file : " << (*itp)->source_path << std::endl;
                        }
                        delete (*itp);
                        itp = process_list.erase(itp);
                        if (exit_code != EXIT_SUCCESS)
                        {
                            return false;
                        }
                    }
                }

                if (a_cc.m_file_list.size() != task_launched && a_cc.m_max_threads > static_cast<uint8_t>(process_list.size()))
                {
                    break;
                }
            }
        }
    }

    assert(process_list.size() == 0);
    assert(task_launched == a_cc.m_file_list.size());
    std::cout << std::endl;
    return true;
}
static void init_options(po::options_description &a_required, po::options_description &a_config, po::options_description &a_compile, po::options_description &a_generic, args_container &a_ac, config_container &a_cc)
{
    {
        po::options_description_easy_init options = a_required.add_options();
        options("workspace-root,w",
                po::value<std::string>()->required()->notifier(std::bind(&on_path_arg, std::placeholders::_1, std::ref(a_cc.m_workspace_root))),
                "Absolute Workspace Path");
        options("section,s",
                po::value<std::string>()->required()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_section_path))),
                "Section Name or Section Relative Path From Workspace Root (Example section_one/)");
        options("platform,p",
                po::value<std::string>()->required()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_platform))),
                "Platform (All,Android,iOS,Linux,OSX,Windows,WSL)");/// make this portable
    }

    {
        po::options_description_easy_init options = a_config.add_options();
        options("save",
                "Save configuration for this section to file");
        options("load",
                "Load configuration for this section from file");
        options("show",
                "Show loaded configuration for this section");
        options("cc",
                po::value<std::string>()->notifier(std::bind(&on_path_arg, std::placeholders::_1, std::ref(a_cc.m_cc))),
                "Set C compiler path");
        options("cxx",
                po::value<std::string>()->notifier(std::bind(&on_path_arg, std::placeholders::_1, std::ref(a_cc.m_cxx))),
                "Set C++ compiler path");
        options("cflags",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_cflags))),
                "Set flags for C compiler");
        options("cxxflags",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_cxxflags))),
                "Set flags for C++ compiler");
        options("cppflags",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_cppflags))),
                "Set flags for both, C & C++ compiler");
        options("ldflags",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_ldflags))),
                "Set flags for linker");
        options("ldlibs",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_ac.m_ldlibs))),
                "Set libs for linker");
    }
    {
        /// build variant
        po::options_description_easy_init options = a_compile.add_options();
        options("build",
                "Compile all modified files for section");
        options("rebuild",
                "Rebuild whole section");
        options("clean",
                "Clean active section object files");
        options("link",
                "Link all objects of section to executable");
        options("build-static-lib",
                "Link static library");
        options("build-variant", po::value<std::string>()->default_value("default"), "Build configuration variant (debug, release)");
        options("out",
                po::value<std::string>()->notifier(std::bind(&on_string_arg, std::placeholders::_1, std::ref(a_cc.m_out))),
                "Name of Executable to generate to (default is section name)");
        options("max-threads,t",
                po::value<int>()->notifier(std::bind(&on_uint8_arg, std::placeholders::_1, std::ref(a_cc.m_max_threads))),
                "Maximum threads to be used for compiling");

        options("build-file",
                "Compile file (compile file path is --section argument)");

        options("diagnostics-color",
                "Turn on diagnostics color of compiler");
    }
    {

        po::options_description_easy_init options = a_generic.add_options();
        options("version", "Outputs Program Version");
        options("help,h", "Help screen");
    }
}

static void on_string_arg(std::string a_val, std::string &a_dest)
{
    a_dest = a_val;
}
static void on_path_arg(std::string a_val, fs::path &a_path)
{
    a_path = a_val;
}
static void on_uint8_arg(int a_val, uint8_t &a_dest)
{
    a_dest = static_cast<uint8_t>(a_val);
}

static bool ls_path(fs::path &a_path, std::function<bool(fs::path &)> a_cb)
{

    fs::recursive_directory_iterator ls_beg(a_path);
    fs::recursive_directory_iterator ls_end;
    for (; ls_beg != ls_end; ls_beg++)
    {
        fs::path l_path = (*ls_beg).path();
        if (!a_cb(l_path))
        {
            return false;
        }
    }
    return true;
}
static bool check_file(fs::path &a_path, std::string a_err_1, std::string a_err_2)
{

    if (!fs::exists(fs::status(a_path)))
    {
        std::cout << a_err_1 << a_path << std::endl;
        return false;
    }
    else if (fs::is_directory(fs::status(a_path)))
    {
        std::cout << a_err_2 << a_path << std::endl;
        return false;
    }

    return true;
}
static bool check_directory(fs::path &a_path, std::string a_err_1, std::string a_err_2, bool a_create)
{

    if (!fs::exists(fs::status(a_path)))
    {
        if (!a_create)
        {
            std::cout << a_err_1 << a_path << std::endl;
            return false;
        }
        if (!fs::create_directories(a_path))
        {
            std::cout << a_err_1 << a_path << std::endl;
            return false;
        }
    }
    else if (!fs::is_directory(fs::status(a_path)))
    {
        std::cout << a_err_2 << a_path << std::endl;
        return false;
    }
    return true;
}
static bool fill_dependency_list(config_container &a_cc,std::map<std::string,std::string>&file_mod_list_update, std::map<std::string, std::map<std::string, std::string>> &dependency_list)
{
    std::vector<std::string> project_path_list;
    fs::path l_path;
    sha256_hash l_hash;
    sha256_ctx ctx;

    std::map<std::string, file_type> ext_map{{".c", file_type::cc}, {".cc", file_type::cpp}, {".cxx", file_type::cpp}, {".cpp", file_type::cpp}, {".h", file_type::header}, {".hpp", file_type::header},{".hxx", file_type::header}};
    project_path_list.push_back(a_cc.m_section_root.string());
    project_path_list.push_back(a_cc.m_workspace_root.string());

    for (auto it = a_cc.m_cxxflags.begin(); it != a_cc.m_cxxflags.end(); it++)
    {
        if (it->size() > 2)
        {
            if ((*it)[0] == '-')
            {
                if ((*it)[1] == 'I')
                {
                    /// system directive
                    // std::cout << "system directive: " << (*it).substr(2) << std::endl;
                    project_path_list.push_back((*it).substr(2));
                }
               /* else if ((*it)[1] == 'i')
                {
                    /// project directive
                    // std::cout << "local directive: " << (*it).substr(2) << std::endl;
                    project_path_list.push_back((*it).substr(2));
                }*/
            }
        }
    }

    for (auto it = a_cc.m_cflags.begin(); it != a_cc.m_cflags.end(); it++)
    {
        if (it->size() > 2)
        {
            if ((*it)[0] == '-')
            {
                /// for msvc is different, so hard to make standard :/
                if ((*it)[1] == 'I')
                {
                    /// system directive
                    // std::cout << "system directive: " << (*it).substr(2) << std::endl;
                    project_path_list.push_back((*it).substr(2));
                }
              /*  else if ((*it)[1] == 'i')
                {
                    /// project directive
                    // std::cout << "local directive: " << (*it).substr(2) << std::endl;
                    project_path_list.push_back((*it).substr(2));
                }*/
            }
        }
    }

    ///bool have_project_path = false;
    for (auto it = a_cc.m_cppflags.begin(); it != a_cc.m_cppflags.end(); it++)
    {
        if (it->size() > 2)
        {
            if ((*it)[0] == '-')
            {
                /// for msvc is different, standard was always a good side :/
                if ((*it)[1] == 'I')
                {
                    /// system directive
                    // std::cout << "system directive: " << (*it).substr(2) << std::endl;
                    /*  if(a_cc.m_section_root.string().compare((*it).substr(2))== 0){
                      have_project_path = true;
                      }*/
                    project_path_list.push_back((*it).substr(2));
                }
               /* else if ((*it)[1] == 'i')
                {
                    /// project directive
                    //std::cout << "local directive: " << (*it).substr(2) << std::endl;
                    project_path_list.push_back((*it).substr(2));
                }*/
            }
        }
    }


    if (!ls_path(a_cc.m_section_root, [&](fs::path &a_path) -> bool
{
    if (fs::is_regular_file(a_path))
        {

            // std::time_t write_access_time = fs::last_write_time(a_path);

            auto it = ext_map.find(a_path.extension().string());
            if (it != ext_map.end())
            {
                std::string file_hash = "";
                {
                    std::size_t pos = a_path.string().find(a_cc.m_section_root.string());
                    if(pos != std::string::npos)
                    {
                        file_hash = a_path.string().substr(a_cc.m_section_root.string().size());
                    }
                }
                //std::cout << "checking dependency for file: " << file_hash << std::endl;


                sha256_init(ctx);
                sha256_update(&ctx,(uint8_t*)file_hash.data(),file_hash.size());
                sha256_finalize(&ctx,l_hash);
                file_hash = "";
                file_hash.assign((char*)l_hash,32);
                auto sfh = dependency_list.find(file_hash);
                if(file_mod_list_update.find(file_hash) == file_mod_list_update.end())
                {
                    if(!get_file_hash(a_path,ctx,l_hash))
                    {
                        return false;
                    }
                    std::string content_hash = "";
                    content_hash.assign((char*)l_hash,32);
                    file_mod_list_update.insert(std::pair<std::string,std::string>(file_hash,content_hash));
                }
                if(sfh == dependency_list.end())
                {
                    dependency_list.insert(std::pair<std::string, std::map<std::string, std::string>>(file_hash,std::map<std::string,std::string>()));
                    sfh = dependency_list.find(file_hash);
                }

                /// file_path


                std::ifstream stream(a_path, std::ios_base::binary);
                if (!stream)
                {
                    std::cout << "Could not read the file: " << a_path << std::endl;
                    return false;
                }



                //auto fp = file_mod_list_update.find()
                std::string data;
                std::string include_prefix = "#include";
                while (stream >> std::quoted(data))
                {
                    if (data[0] == '/')/// comment probably
                    {
                        if (!std::getline(stream, data))
                        {
                            break;
                        }
                    }
                    else if (data[0] == '#' && data.size() >= include_prefix.size())
                    {
                        if (data.substr(0, include_prefix.size()) == include_prefix)
                        {
                            if (data.size() == include_prefix.size())
                            {
                                if (stream >> std::quoted(data))
                                {
                                    std::size_t pos = data.find("<");
                                    if (pos != std::string::npos)
                                    {

                                        data = data.substr(pos + 1);
                                        pos = data.find(">");
                                        if (pos != std::string::npos)
                                        {
                                            data = data.substr(0, pos);
                                            l_path = data;
                                            if (!l_path.is_absolute())
                                            {
                                                data = "/" + l_path.string();
                                                l_path = data;
                                            }
                                            l_path = a_path;
                                            l_path = l_path.remove_filename().append(data);


                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                                if(pos != std::string::npos)
                                                {

                                                    l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                    data = l_path.string();

                                                }

                                            }
                                            else
                                            {
                                                l_path = a_cc.m_workspace_root;
                                                l_path = l_path.append(data);
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                    if(pos != std::string::npos)
                                                    {

                                                        l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                        data = l_path.string();
                                                    }
                                                }

                                            }
                                            //std::cout << "extracted include <> directive: \"" << data << "\"" << std::endl;
                                            sha256_init(ctx);
                                            sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                            sha256_finalize(&ctx,l_hash);
                                            std::string path_hash = "";

                                            path_hash.assign((char*)l_hash,32);
                                            auto fh = file_mod_list_update.find(file_hash);
                                            if(fh == file_mod_list_update.end())
                                            {
                                                if(sfh->second.find(path_hash) == sfh->second.end())
                                                {
                                                    sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                                }
                                            }
                                            else
                                            {
                                                for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                                {
                                                    l_path = (*it);
                                                    l_path.append(data);
                                                    if(fs::exists(fs::status(l_path)))
                                                    {
                                                        break;
                                                    }
                                                }
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    if(sfh->second.find(path_hash) == sfh->second.end())
                                                    {
                                                        if(!get_file_hash(l_path,ctx,l_hash))
                                                        {
                                                            return false;
                                                        }
                                                        std::string content_hash = "";
                                                        content_hash.assign((char*)l_hash,32);
                                                        sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                    }
                                                }
                                                else
                                                {
                                                    // std::cout << "could not find include directive: " << data << std::endl;
                                                }
                                            }


                                        }
                                    }
                                    else
                                    {
                                        pos = data.find("\"");
                                        if (pos == std::string::npos)
                                        {
                                            l_path = data;
                                            if (!l_path.is_absolute())
                                            {
                                                data = "/" + l_path.string();
                                            }
                                            l_path = a_path;
                                            l_path = l_path.remove_filename().append(data);


                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                                if(pos != std::string::npos)
                                                {

                                                    l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                    data = l_path.string();

                                                }

                                            }
                                            else
                                            {
                                                l_path = a_cc.m_workspace_root;
                                                l_path = l_path.append(data);
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                    if(pos != std::string::npos)
                                                    {

                                                        l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                        data = l_path.string();
                                                    }
                                                }

                                            }

                                            //  std::cout << "extracted include \"\" directive: \"" << data << "\" " << l_path << std::endl;

                                            sha256_init(ctx);
                                            sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                            sha256_finalize(&ctx,l_hash);
                                            std::string path_hash = "";

                                            path_hash.assign((char*)l_hash,32);
                                            auto fh = file_mod_list_update.find(file_hash);
                                            if(fh == file_mod_list_update.end())
                                            {
                                                if(sfh->second.find(path_hash) == sfh->second.end())
                                                {
                                                    sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                                }
                                            }
                                            else
                                            {
                                                for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                                {
                                                    l_path = (*it);
                                                    l_path.append(data);
                                                    if(fs::exists(fs::status(l_path)))
                                                    {
                                                        break;
                                                    }
                                                }
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    if(sfh->second.find(path_hash) == sfh->second.end())
                                                    {
                                                        if(!get_file_hash(l_path,ctx,l_hash))
                                                        {
                                                            return false;
                                                        }
                                                        std::string content_hash = "";
                                                        content_hash.assign((char*)l_hash,32);
                                                        sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                    }
                                                }
                                                else
                                                {
                                                    // std::cout << "could not find include directive: " << data << std::endl;
                                                }
                                            }

                                        }
                                        else
                                        {
                                            data = data.substr(pos + 1);
                                            pos = data.find("\"");
                                            if (pos != std::string::npos)
                                            {
                                                data = data.substr(0, pos);
                                                l_path = data;
                                                if (!l_path.is_absolute())
                                                {
                                                    data = "/" + l_path.string();
                                                }

                                                l_path = a_path;
                                                l_path = l_path.remove_filename().append(data);


                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                                    if(pos != std::string::npos)
                                                    {

                                                        l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                        data = l_path.string();

                                                    }

                                                }
                                                else
                                                {
                                                    l_path = a_cc.m_workspace_root;
                                                    l_path = l_path.append(data);
                                                    if(fs::exists(fs::status(l_path)))
                                                    {
                                                        pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                        if(pos != std::string::npos)
                                                        {

                                                            l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                            data = l_path.string();
                                                        }
                                                    }

                                                }
                                                //std::cout << "extracted include \"\" directive: \"" << data << "\"" << std::endl;
                                                sha256_init(ctx);
                                                sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                                sha256_finalize(&ctx,l_hash);
                                                std::string path_hash = "";

                                                path_hash.assign((char*)l_hash,32);
                                                auto fh = file_mod_list_update.find(file_hash);
                                                if(fh == file_mod_list_update.end())
                                                {
                                                    if(sfh->second.find(path_hash) == sfh->second.end())
                                                    {
                                                        sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                                    }
                                                }
                                                else
                                                {
                                                    for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                                    {
                                                        l_path = (*it);
                                                        l_path.append(data);
                                                        if(fs::exists(fs::status(l_path)))
                                                        {
                                                            break;
                                                        }
                                                    }
                                                    if(fs::exists(fs::status(l_path)))
                                                    {
                                                        if(sfh->second.find(path_hash) == sfh->second.end())
                                                        {
                                                            if(!get_file_hash(l_path,ctx,l_hash))
                                                            {
                                                                return false;
                                                            }
                                                            std::string content_hash = "";
                                                            content_hash.assign((char*)l_hash,32);
                                                            sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                        }
                                                    }
                                                    else
                                                    {
                                                        //   std::cout << "could not find include directive: " << data << std::endl;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                data = data.substr(include_prefix.size());
                                std::size_t pos = data.find("<");
                                if (pos != std::string::npos)
                                {
                                    data = data.substr(pos + 1);
                                    pos = data.find(">");
                                    if (pos != std::string::npos)
                                    {
                                        data = data.substr(0, pos);
                                        l_path = data;
                                        if (!l_path.is_absolute())
                                        {
                                            data = "/" + l_path.string();
                                        }
                                        l_path = a_path;
                                        l_path = l_path.remove_filename().append(data);


                                        if(fs::exists(fs::status(l_path)))
                                        {
                                            std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                            if(pos != std::string::npos)
                                            {

                                                l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                data = l_path.string();

                                            }

                                        }
                                        else
                                        {
                                            l_path = a_cc.m_workspace_root;
                                            l_path = l_path.append(data);
                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                if(pos != std::string::npos)
                                                {

                                                    l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                    data = l_path.string();
                                                }
                                            }

                                        }
                                        //std::cout << "extracted include T <> directive: \"" << data << "\"" << std::endl;

                                        sha256_init(ctx);
                                        sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                        sha256_finalize(&ctx,l_hash);
                                        std::string path_hash = "";

                                        path_hash.assign((char*)l_hash,32);
                                        auto fh = file_mod_list_update.find(file_hash);
                                        if(fh == file_mod_list_update.end())
                                        {
                                            if(sfh->second.find(path_hash) == sfh->second.end())
                                            {
                                                sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                            }
                                        }
                                        else
                                        {
                                            for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                            {
                                                l_path = (*it);
                                                l_path.append(data);
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    break;
                                                }
                                            }
                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                if(sfh->second.find(path_hash) == sfh->second.end())
                                                {
                                                    if(!get_file_hash(l_path,ctx,l_hash))
                                                    {
                                                        return false;
                                                    }
                                                    std::string content_hash = "";
                                                    content_hash.assign((char*)l_hash,32);
                                                    sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                }
                                            }
                                            else
                                            {
                                                // std::cout << "could not find include directive: " << data << std::endl;
                                            }
                                        }


                                    }
                                }
                                else
                                {
                                    pos = data.find("\"");
                                    if (pos == std::string::npos)
                                    {
                                        l_path = data;
                                        if (!l_path.is_absolute())
                                        {
                                            data = "/" + l_path.string();
                                        }

                                        l_path = a_path;
                                        l_path = l_path.remove_filename().append(data);


                                        if(fs::exists(fs::status(l_path)))
                                        {
                                            std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                            if(pos != std::string::npos)
                                            {

                                                l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                data = l_path.string();

                                            }

                                        }
                                        else
                                        {
                                            l_path = a_cc.m_workspace_root;
                                            l_path = l_path.append(data);
                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                if(pos != std::string::npos)
                                                {

                                                    l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                    data = l_path.string();
                                                }
                                            }

                                        }
                                        //std::cout << "extracted include T \"\" directive: \"" << data << "\"" << std::endl;

                                        sha256_init(ctx);
                                        sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                        sha256_finalize(&ctx,l_hash);
                                        std::string path_hash = "";

                                        path_hash.assign((char*)l_hash,32);
                                        auto fh = file_mod_list_update.find(file_hash);
                                        if(fh == file_mod_list_update.end())
                                        {
                                            if(sfh->second.find(path_hash) == sfh->second.end())
                                            {
                                                sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                            }
                                        }
                                        else
                                        {
                                            for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                            {
                                                l_path = (*it);
                                                l_path.append(data);
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    break;
                                                }
                                            }
                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                if(sfh->second.find(path_hash) == sfh->second.end())
                                                {
                                                    if(!get_file_hash(l_path,ctx,l_hash))
                                                    {
                                                        return false;
                                                    }
                                                    std::string content_hash = "";
                                                    content_hash.assign((char*)l_hash,32);
                                                    sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                }
                                            }
                                            else
                                            {
                                                // std::cout << "could not find include directive: " << data << std::endl;
                                            }
                                        }

                                    }
                                    else
                                    {
                                        data = data.substr(pos + 1);
                                        pos = data.find("\"");
                                        if (pos != std::string::npos)
                                        {
                                            data = data.substr(0, pos);
                                            l_path = data;
                                            if (!l_path.is_absolute())
                                            {
                                                data = "/" + l_path.string();
                                            }
                                            l_path = a_path;
                                            l_path = l_path.remove_filename().append(data);


                                            if(fs::exists(fs::status(l_path)))
                                            {
                                                std::size_t pos = l_path.string().find(a_cc.m_section_root.string());
                                                if(pos != std::string::npos)
                                                {

                                                    l_path = l_path.string().substr(a_cc.m_section_root.string().size());
                                                    data = l_path.string();

                                                }

                                            }
                                            else
                                            {
                                                l_path = a_cc.m_workspace_root;
                                                l_path = l_path.append(data);
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    pos = l_path.string().find(a_cc.m_workspace_root.string());
                                                    if(pos != std::string::npos)
                                                    {

                                                        l_path = l_path.string().substr(a_cc.m_workspace_root.string().size());

                                                        data = l_path.string();
                                                    }
                                                }

                                            }
                                            //std::cout << "extracted include T \"\" directive: \"" << data << "\"" << std::endl;
                                            sha256_init(ctx);
                                            sha256_update(&ctx,(uint8_t*)data.data(),data.size());
                                            sha256_finalize(&ctx,l_hash);
                                            std::string path_hash = "";

                                            path_hash.assign((char*)l_hash,32);
                                            auto fh = file_mod_list_update.find(file_hash);
                                            if(fh == file_mod_list_update.end())
                                            {
                                                if(sfh->second.find(path_hash) == sfh->second.end())
                                                {
                                                    sfh->second.insert(std::pair<std::string,std::string>(path_hash,fh->second));
                                                }
                                            }
                                            else
                                            {
                                                for(auto it = project_path_list.begin(); it!=project_path_list.end(); it++)
                                                {
                                                    l_path = (*it);
                                                    l_path.append(data);
                                                    if(fs::exists(fs::status(l_path)))
                                                    {
                                                        break;
                                                    }
                                                }
                                                if(fs::exists(fs::status(l_path)))
                                                {
                                                    if(sfh->second.find(path_hash) == sfh->second.end())
                                                    {
                                                        if(!get_file_hash(l_path,ctx,l_hash))
                                                        {
                                                            return false;
                                                        }
                                                        std::string content_hash = "";
                                                        content_hash.assign((char*)l_hash,32);
                                                        sfh->second.insert(std::pair<std::string,std::string>(path_hash,content_hash));
                                                    }
                                                }
                                                else
                                                {
                                                    //   std::cout << "could not find include directive: " << data << std::endl;
                                                }
                                            }

                                        }
                                    }
                                }
                            }
                        }
                    }

                    {
                        if (!std::getline(stream, data))
                        {
                            break;
                        }
                    }
                }
                stream.close();
            }
        }
        return true;
    }))
    {

        return false;
    }

    return true;
}
