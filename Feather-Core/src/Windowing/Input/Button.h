#pragma once

namespace Feather {

	struct Button
	{
		bool IsPressed{ false }, JustPressed{ false }, JustReleased{ false };

		void Update(bool pressed);
		void Reset();
	};

}
