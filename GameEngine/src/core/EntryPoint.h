#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern GE::Application* GE::CreateApplication();

int main(int argc, char** argv)
{
	auto app = GE::CreateApplication();
	app->run();
	delete app;
}

#endif