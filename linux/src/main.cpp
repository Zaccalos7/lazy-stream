#include "core.hpp"
#include "native_window.hpp"
#include "version.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sys/file.h>
#include <unistd.h>

namespace {
// Valida l’intero testo e l’intervallo TCP prima della conversione, evitando porte troncate o
// valori fuori limite.
unsigned short parsePort(const std::string& value) {
    size_t used = 0;
    long port = std::stol(value, &used);
    if (used != value.size() || port < 1 || port > 65535)
        throw std::runtime_error("La porta deve essere tra 1 e 65535");
    return static_cast<unsigned short>(port);
}

// Genera un token da 32 byte del sistema e lo codifica in esadecimale per il bootstrap della
// sessione browser.
std::string randomToken() {
    std::ifstream random("/dev/urandom", std::ios::binary);
    std::string out;
    const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        unsigned char b = 0;
        random.read(reinterpret_cast<char*>(&b), 1);
        if (!random)
            throw std::runtime_error("Generatore casuale non disponibile");
        out += hex[b >> 4];
        out += hex[b & 15];
    }
    return out;
}
} // namespace

// Configura percorsi, istanza unica e segnali prima dei worker, poi avvia la finestra Qt.
// Il servizio HTTP viene creato soltanto quando è richiesta esplicitamente la modalità headless.
int main(int argc, char** argv) {
    // Una pipe chiusa deve diventare un errore di scrittura, non terminare l'intera applicazione.
    signal(SIGPIPE, SIG_IGN);
    sigset_t termination;
    sigemptyset(&termination);
    sigaddset(&termination, SIGINT);
    sigaddset(&termination, SIGTERM);
    // I worker ereditano la maschera: un solo thread sigwait gestisce l'arresto delle sessioni.
    pthread_sigmask(SIG_BLOCK, &termination, nullptr);
    try {
        auto env = std::getenv("LAZY_STREAM_DATA_DIR");
        auto xdg = std::getenv("XDG_DATA_HOME");
        auto home = std::getenv("HOME");
        orbis::fs::path root = env ? env
                                   : ((xdg ? orbis::fs::path(xdg)
                                           : orbis::fs::path(home ? home : ".") / ".local/share") /
                                      "orbis-stream");
        unsigned short port = 1200;
        bool headless = false;
        orbis::fs::path web = ORBIS_WEB_ROOT;
        auto installed = orbis::fs::canonical("/proc/self/exe").parent_path().parent_path() /
                         "share/orbis-stream/web";
        if (orbis::fs::exists(installed))
            web = installed;
        if (auto p = std::getenv("LAZY_STREAM_PORT"))
            port = parsePort(p);
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--version") {
                std::cout << ORBIS_VERSION << '\n';
                return 0;
            } else if (a == "--no-browser" || a == "--headless")
                headless = true;
            else if (a == "--data-dir" && i + 1 < argc)
                root = argv[++i];
            else if (a.starts_with("--data-dir="))
                root = a.substr(11);
            else if (a == "--port" && i + 1 < argc)
                port = parsePort(argv[++i]);
            else if (a.starts_with("--port="))
                port = parsePort(a.substr(7));
            else if (a == "--web-root" && i + 1 < argc)
                web = argv[++i];
            else if (a == "--help") {
                std::cout << "orbis-stream [--version] [--data-dir DIR] [--headless] [--port 1200] "
                             "[--web-root DIR]\n";
                return 0;
            } else
                throw std::runtime_error("Opzione sconosciuta: " + a);
        }
        orbis::fs::create_directories(root);
        int lock = open((root / "instance.lock").c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
        if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB))
            throw std::runtime_error("Questa cartella dati è già aperta da un'altra istanza");
        orbis::Studio studio(root);
        if (!headless)
            return orbis::runNativeApp(argc, argv, studio, web / "resources");
        auto token = randomToken();
        auto url = "http://127.0.0.1:" + std::to_string(port) + "/?token=" + token;
        std::cout << "OrbisStream Linux " ORBIS_VERSION "\n"
                  << url << "\nDati: " << root << std::endl;
        std::thread shutdownThread([&studio] {
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGINT);
            sigaddset(&set, SIGTERM);
            int value;
            sigwait(&set, &value);
            studio.stopAll();
            std::_Exit(0);
        });
        shutdownThread.detach();
        orbis::serve(studio, port, web, token);
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
}
