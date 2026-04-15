#include <Engine.h>

class Sandbox : public GE::Application
{
public:
	Sandbox();

	~Sandbox();
};

int main()
{
	Sandbox* sandbox = new Sandbox;
	sandbox->run();
	delete sandbox;
}