#include "globals.h"

void main() {
	mem_init();

	AVInstance ClientReplicator = get_client_replicator();
	AVInstance NetworkClient = ClientReplicator.parent();
	AVInstance DataModel = NetworkClient.parent();
	
	AVInstance Lighting = DataModel.findfirstchild("Lighting");


	printf("Type Value of Technology (https://developer.roblox.com/en-us/api-reference/enum/Technology) - (4 for FutureLighting)\n");

	std::string input;
	while (std::getline(std::cin, input)) {
		write<int>(Lighting.ptr() + 0x14C, atoi(input.c_str()));
	}
}