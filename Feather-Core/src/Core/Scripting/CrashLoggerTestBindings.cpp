#include "CrashLoggerTestBindings.h"

#include "Core/ECS/MainRegistry.h"
#include "Logger/CrashLogger.h"

namespace Feather {

	void FunctionC()
	{
		// Crash happens here
		int* ptr{ nullptr };
		*ptr = 194;
	}

	void FunctionB()
	{
		FunctionC();
	}

	void FunctionA()
	{
		FunctionB();
	}

	// Bind this function to Lua
	void CrashFromLua()
	{
		FunctionA();
	}

	void CrashLoggerTests::CreateLuaBind(sol::state& lua)
	{
		lua_State* L = lua.lua_state();
		FEATHER_CRASH_LOGGER().SetLuaState(L);

		lua.set_function("F_CrashLoggerTest", &CrashFromLua);

		lua.script(R"(
        function F_CrashTest_level5()
            print("Entering Level 5...")
			-- Calls the C++ function that crashes
            F_CrashLoggerTest()  
        end

        function F_CrashTest_level4()
            print("Entering Level 4...")
            F_CrashTest_level5()
        end

        function F_CrashTest_level3()
            print("Entering Level 3...")
            F_CrashTest_level4()
        end

        function F_CrashTest_level2()
            print("Entering Level 2...")
            F_CrashTest_level3()
        end

        function F_CrashTest_level1()
            print("Entering Level 1...")
            F_CrashTest_level2()
        end

        function F_CrashTest_Start()
            print("Starting Lua Crash Test Execution...")
            F_CrashTest_level1()
        end
    )");
	}

}
