#pragma once

namespace Feather {

	class IconReplacer
	{
	public:
		IconReplacer(const std::string& iconFile, const std::string& exeFile);
		~IconReplacer();

		bool ReplaceIcon();

	private:
		class Impl;
		std::unique_ptr<Impl> m_Impl;
	};

}
