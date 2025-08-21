#pragma once

namespace Feather {

	class FileProcessor
	{
	public:
		bool OpenApplicationFromFile(const std::string& filename, std::vector<const char*> params);

	private:
		std::string SeparateParams(std::vector<const char*>& params)
		{
			std::string sParams{};
			for (const auto& param : params)
				sParams += param + ' ';

			return sParams;
		}
	};

}
