// FFmpeg dies with the application even after SIGKILL/crash. No shell is involved.
#include <cstdlib>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>

// Lega il figlio alla vita del padre con PDEATHSIG; controlla il PID per coprire la morte del padre
// prima di prctl.
int main(int argc, char** argv) {
    if (argc < 3)
        return 125;
    pid_t expected = std::strtol(argv[1], nullptr, 10);
    if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0 || getppid() != expected)
        return 125;
    execvp(argv[2], argv + 2);
    return 127;
}
