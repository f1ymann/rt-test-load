#include <imgui.h>
#include <map>
#include <string>

namespace rt3gui {

	class DevicesTree {
	public:
		DevicesTree();

		~DevicesTree() {};

		void DrawNodesTree();

	private:


		std::map<std::string, uint32_t> devices;
	};
} //rt3gui