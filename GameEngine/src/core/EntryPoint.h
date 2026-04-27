#pragma once

#ifdef GE_PLATFORM_WINDOWS

extern GE::Application* GE::CreateApplication();

int main(int argc, char** argv)
{
	GE::Log::Init();
	GE_CORE_WARN("Initialized Log!");

	int a = 10;
	GE_INFO("Hello! Var = {0}", a);

	auto app = GE::CreateApplication();
	app->Run();
	delete app;
}
#endif