#include "common.h"

extern stAppConfig g_appConfig;

string get_current_time(string format)
{
    std::tm local_time;
    std::time_t now = std::time(0);
    localtime_r(&now, &local_time);

    char buffer[128] = { 0 };
    strftime(buffer, sizeof(buffer), format.data(), &local_time);
    return string(buffer);
}

void check_app_running()
{
    char szAppName[1024] = { 0 };
	ssize_t nRet = readlink("/proc/self/exe", szAppName, sizeof(szAppName));
	if(nRet < 0) {
		exit(1);
	}

	string strAppName = basename(szAppName);
	string strAppPath = dirname(szAppName);

	string strAppLockFile = getenv("HOME");
    strAppLockFile = strAppLockFile + "/." + strAppName + ".Lock";
	//S_IRUSR:user has read permission; S_IWUSR:user has write permission
	int fd = open(strAppLockFile.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	if(fd  < 0) {
		std::cout << "Open " << strAppLockFile << " failed, Reason: " << strerror(errno) << endl;
		exit(1);
	}

	//0:indicate lock whole file
	if(lockf(fd, F_TLOCK, 0L) < 0) {
		close(fd);
		cout << strAppName << " have running !" << endl;
		exit(1);
	}
}

set<int> parse_listen_ports(const string& portStr)
{
    set<int> ports;
    stringstream ss(portStr);
    string token;

    while (getline(ss, token, ',')) {
        // Check if token contains a range (~)
        size_t tildePos = token.find('~');
        if (tildePos != string::npos) {
            try {
                int start = stoi(token.substr(0, tildePos));
                int end = stoi(token.substr(tildePos + 1));

                // Ensure valid range
                if (start <= end) {
                    for (int i = start; i <= end; ++i) {
                        ports.insert(i);
                    }
                }
            } catch (...) {
                // Ignore invalid numbers
                continue;
            }
        } else {
            try {
                ports.insert(stoi(token));
            } catch (...) {
                // Ignore invalid numbers
                continue;
            }
        }
    }

    return ports;
}