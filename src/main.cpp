#include <iostream>
#include <tclap/CmdLine.h>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
struct stat sb;
struct passwd *pw = getpwuid(getuid());

std::string USER_HOME = std::string(pw->pw_dir);
std::string BEAM_DIR = USER_HOME + "/.beam/";
std::string BEAM_DB = "locations.bm";
std::string Q_BEAM_DB = BEAM_DIR + BEAM_DB;

std::string get_time_now() { time_t t = time(0); struct tm * now = localtime(&t);
    std::stringstream s; s << now->tm_hour << "-" << now->tm_min; return s.str(); }
bool does_dir_exist(std::string path) { return stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode); }
bool does_file_exist(std::string path) { return stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode); }

std::pair<std::string, bool> lookup_dest(std::string name) {
    std::string line;
    std::ifstream db (Q_BEAM_DB);
    if( db.is_open() ) {
        while( getline(db, line) )
            if ( line.substr(0, line.find("=")) == name ) { db.close();
                return std::make_pair(line.substr(line.find("=") + 1), true); }
        db.close();
        return std::make_pair("could not find destination in beam db", false);
    } else 
        return std::make_pair("could not open file", false);
}

void add_dest(std::string name, std::string path) {
    std::ofstream db (Q_BEAM_DB, std::ios::app);
    if ( db.is_open() ) {
        db << name.c_str() << "=" << path.c_str() << '\n';
        db.close();
    } else
        std::cerr << "could not open beam database file" << std::endl;
}

//thanks, david g.

inline std::string trim(const std::string &s) {    
    auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
    auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
    return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback)); }

std::string sys_and_read(const char* c_str) {
    FILE *f;
    std::stringstream stream;
    char buff[512];
    if(!(f = popen(c_str, "r")))
        return std::string("");
    while(fgets(buff, sizeof(buff), f) != NULL)
        stream << buff;
    pclose(f);
    return trim(stream.str());
}

std::string get_current_directory() {
    return sys_and_read("pwd");
}

bool contains(std::string base, std::string token) {
    return base.find(token) != std::string::npos;
}

void replace_all(std::string *str, std::string look, std::string with) { while( str->find(look) != std::string::npos )
                                                                            str->replace(str->find(look), with.length(), with); }
// im lazy :P
void clear_destinations() {
    system(("rm " + Q_BEAM_DB).c_str());
    system(("touch " + Q_BEAM_DB).c_str());
}

void remove_destination(std::string dest_name) {
    std::string line;
    std::string Q_BEAM_DB_TMP = Q_BEAM_DB + ".tmp";
    std::ifstream db(Q_BEAM_DB);
    std::ofstream temp_db(Q_BEAM_DB_TMP);
    while( getline(db, line) ) { std::cout << "matching \"" << line.substr(0, line.find("=")) << "\" against \"" << dest_name <<  "\"" << std::endl;
        if( line.substr(0, line.find("=")) != dest_name )
            temp_db << line << '\n';
    }
    db.close(); temp_db.close();
    system(("rm " + Q_BEAM_DB).c_str());
    system(("mv " + Q_BEAM_DB_TMP + " " + Q_BEAM_DB).c_str());
}

// very, very lazy.
void list_destinations() {
    system(("cat " + Q_BEAM_DB).c_str());
}

int main(int argc, char ** argv) {

    if(!does_dir_exist(BEAM_DIR))
        system(("mkdir " + BEAM_DIR).c_str());

    if(!does_file_exist(Q_BEAM_DB))
        system(("touch " + Q_BEAM_DB).c_str());

    try {
        TCLAP::CmdLine cmd("beam - beam across your filesystem", ' ', "0.1");
        TCLAP::ValueArg<std::string> to("t", "to", "set a destination directory", false, ".", "string"); 
        TCLAP::SwitchArg remove("r", "remove", "remove a beam destination", cmd, false);
        TCLAP::SwitchArg set("s", "set", "flip to set a beam destination", cmd, false);
        TCLAP::UnlabeledValueArg<std::string> name("dest", "beam destination", true, ("loc-at+" + get_time_now()), "string");

        cmd.add(to);
        cmd.add(name);

        cmd.parse(argc, argv);

        std::string dest = to.getValue();

        if( name.getValue() == "list" ) { list_destinations();
            return EXIT_SUCCESS; }
        else if( name.getValue() == "clear" ) { clear_destinations();
            return EXIT_SUCCESS; }

        if( contains(dest, "~") )
            replace_all(&dest, "~", USER_HOME);
        if( contains(dest, ".") ) { std::string cur_dir = get_current_directory();
            replace_all(&dest, ".", cur_dir); }

        if( remove.getValue() ) {
            remove_destination(name.getValue());
            return EXIT_SUCCESS;
        }

        if( !set.getValue() ) {
            std::pair<std::string, bool> query = lookup_dest(name.getValue());
            if(!query.second) { std::cerr << "[" << query.first << "] could not find beam pad \"" << dest << "\"" << std::endl; return EXIT_FAILURE; }
            std::cout << query.first.c_str() << std::endl;
            return 2;
        } else 
            add_dest(name.getValue(), dest);
    } catch(TCLAP::ArgException &e) { 
        std::cerr << "argument error: \"" << e.error() << "\" for \"" << e.argId() << "\"" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}
