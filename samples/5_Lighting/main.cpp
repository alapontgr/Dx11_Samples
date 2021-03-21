#include "framework/framework.h"
#include "App.h"

int main(int argc, char** argv)
{
	static App s_App(argc, argv);
	return s_App.run();
}