#pragma once

#include "Batcher.h"
#include "../Essentials/BatchTypes.h"
#include "../Essentials/Primitives.h"

namespace Feather {

	class LineBatchRenderer : public Batcher<LineBatch, LineGlyph>
	{
	public:
		LineBatchRenderer();
		~LineBatchRenderer() = default;

		virtual void End() override;
		virtual void Render() override;

		void AddLine(const Line& line);

	private:
		virtual void GenerateBatches() override;
		void Initialize();
	};

}
