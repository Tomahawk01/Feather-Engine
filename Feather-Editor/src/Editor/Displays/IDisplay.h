#pragma once
#include <vector>
#include <memory>

namespace Feather {

	class IDisplay
	{
	public:
		virtual ~IDisplay() = default;
		virtual void Draw() = 0;
		virtual void Update() {}

	protected:
		virtual void DrawToolbar() {}
	};

	struct DisplayHolder
	{
		std::vector<std::unique_ptr<IDisplay>> displays;
	};

}