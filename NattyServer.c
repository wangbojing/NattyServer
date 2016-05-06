

//#include "NattyUtils.h"
#include "NattyUdpServer.h"

int main() {
	void* ntyServerInfo = New(ntyUdpServerInstance());
	ntyDisplay();

	return ntyUdpServerRun(ntyServerInfo);
}




