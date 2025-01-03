#pragma once

namespace Feather {

	struct Button
	{
		bool IsPressed{ false };
		bool JustPressed{ false };
		bool JustReleased{ false };

		void Update(bool pressed);
		void Reset();
	};

}
