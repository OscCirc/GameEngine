#include "pch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace GE {
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			GE_TRACE(e.ToString());
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			GE_TRACE(e.ToString());
		}
		while (true);
	}
}
