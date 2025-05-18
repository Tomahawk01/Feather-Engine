#include "GlmLuaBindings.h"

#include "Core/Scripting/ScriptingUtilities.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace Feather {

	// glm::vec2
	static void CreateVec2Bind(sol::state& lua)
	{
		// Multiplier overloads
		auto vec2_multiply_overloads = sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) { return v1 * v2; },
			[](const glm::vec2& v1, float value) { return v1 * value; },
			[](float value, const glm::vec2& v1) { return value * v1; }
		);
		// Devider overloads
		auto vec2_divide_overloads = sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) { return v1 / v2; },
			[](const glm::vec2& v1, float value) { return v1 / value; },
			[](float value, const glm::vec2& v1) { return value / v1; }
		);
		// Addition overloads
		auto vec2_addition_overloads = sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) { return v1 + v2; },
			[](const glm::vec2& v1, float value) { return v1 + value; },
			[](float value, const glm::vec2& v1) { return value + v1; }
		);
		// Subtraction overloads
		auto vec2_subtraction_overloads = sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) { return v1 - v2; },
			[](const glm::vec2& v1, float value) { return v1 - value; },
			[](float value, const glm::vec2& v1) { return value - v1; }
		);

		// Create vec2 usertype
		lua.new_usertype<glm::vec2>(
			"vec2",
			sol::call_constructor,
			sol::constructors<glm::vec2(float), glm::vec2(float, float)>(),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y,
			sol::meta_function::multiplication, vec2_multiply_overloads,
			sol::meta_function::division, vec2_divide_overloads,
			sol::meta_function::addition, vec2_addition_overloads,
			sol::meta_function::subtraction, vec2_subtraction_overloads,
			"length", [](const glm::vec2& v) { return glm::length(v); },
			"lengthSq", [](const glm::vec2& v) { return glm::length2(v); },
			"normalize", [](const glm::vec2& v1) { return glm::normalize(v1); },
			"normalize2", [](const glm::vec2& v1, const glm::vec2& v2) { return glm::normalize(v2 - v1); },
			"nearly_zero_x", [](const glm::vec2& v) { return glm::epsilonEqual(v.x, 0.0f, 0.001f); },
			"nearly_zero_y", [](const glm::vec2& v) { return glm::epsilonEqual(v.y, 0.0f, 0.001f); }
		);
	}

	// glm::vec3
	static void CreateVec3Bind(sol::state& lua)
	{
		// Multiplier overloads
		auto vec3_multiply_overloads = sol::overload(
			[](const glm::vec3& v1, const glm::vec3& v2) { return v1 * v2; },
			[](const glm::vec3& v1, float value) { return v1 * value; },
			[](float value, const glm::vec3& v1) { return value * v1; }
		);
		// Devider overloads
		auto vec3_divide_overloads = sol::overload(
			[](const glm::vec3& v1, const glm::vec3& v2) { return v1 / v2; },
			[](const glm::vec3& v1, float value) { return v1 / value; },
			[](float value, const glm::vec3& v1) { return value / v1; }
		);
		// Addition overloads
		auto vec3_addition_overloads = sol::overload(
			[](const glm::vec3& v1, const glm::vec3& v2) { return v1 + v2; },
			[](const glm::vec3& v1, float value) { return v1 + value; },
			[](float value, const glm::vec3& v1) { return value + v1; }
		);
		// Subtraction overloads
		auto vec3_subtraction_overloads = sol::overload(
			[](const glm::vec3& v1, const glm::vec3& v2) { return v1 - v2; },
			[](const glm::vec3& v1, float value) { return v1 - value; },
			[](float value, const glm::vec3& v1) { return value - v1; }
		);

		// Create vec3 usertype
		lua.new_usertype<glm::vec3>(
			"vec3",
			sol::call_constructor,
			sol::constructors<glm::vec3(float), glm::vec3(float, float, float)>(),
			"x", &glm::vec3::x,
			"y", &glm::vec3::y,
			"z", &glm::vec3::z,
			sol::meta_function::multiplication, vec3_multiply_overloads,
			sol::meta_function::division, vec3_divide_overloads,
			sol::meta_function::addition, vec3_addition_overloads,
			sol::meta_function::subtraction, vec3_subtraction_overloads,
			"length", [](const glm::vec3& v) { return glm::length(v); },
			"lengthSq", [](const glm::vec3& v) { return glm::length2(v); },
			"normalize", [](const glm::vec3& v1) { return glm::normalize(v1); },
			"normalize2", [](const glm::vec3& v1, const glm::vec3& v2) { return glm::normalize(v2 - v1); },
			"nearly_zero_x", [](const glm::vec3& v) { return glm::epsilonEqual(v.x, 0.0f, 0.001f); },
			"nearly_zero_y", [](const glm::vec3& v) { return glm::epsilonEqual(v.y, 0.0f, 0.001f); },
			"nearly_zero_z", [](const glm::vec3& v) { return glm::epsilonEqual(v.z, 0.0f, 0.001f); }
		);
	}

	// glm::vec4
	static void CreateVec4Bind(sol::state& lua)
	{
		// Multiplier overloads
		auto vec4_multiply_overloads = sol::overload(
			[](const glm::vec4& v1, const glm::vec4& v2) { return v1 * v2; },
			[](const glm::vec4& v1, float value) { return v1 * value; },
			[](float value, const glm::vec4& v1) { return value * v1; }
		);
		// Devider overloads
		auto vec4_divide_overloads = sol::overload(
			[](const glm::vec4& v1, const glm::vec4& v2) { return v1 / v2; },
			[](const glm::vec4& v1, float value) { return v1 / value; },
			[](float value, const glm::vec4& v1) { return value / v1; }
		);
		// Addition overloads
		auto vec4_addition_overloads = sol::overload(
			[](const glm::vec4& v1, const glm::vec4& v2) { return v1 + v2; },
			[](const glm::vec4& v1, float value) { return v1 + value; },
			[](float value, const glm::vec4& v1) { return value + v1; }
		);
		// Subtraction overloads
		auto vec4_subtraction_overloads = sol::overload(
			[](const glm::vec4& v1, const glm::vec4& v2) { return v1 - v2; },
			[](const glm::vec4& v1, float value) { return v1 - value; },
			[](float value, const glm::vec4& v1) { return value - v1; }
		);

		// Create vec4 usertype
		lua.new_usertype<glm::vec4>(
			"vec4",
			sol::call_constructor,
			sol::constructors<glm::vec4(float), glm::vec4(float, float, float, float)>(),
			"x", &glm::vec4::x,
			"y", &glm::vec4::y,
			"z", &glm::vec4::z,
			"w", &glm::vec4::w,
			sol::meta_function::multiplication, vec4_multiply_overloads,
			sol::meta_function::division, vec4_divide_overloads,
			sol::meta_function::addition, vec4_addition_overloads,
			sol::meta_function::subtraction, vec4_subtraction_overloads,
			"length", [](const glm::vec4& v) { return glm::length(v); },
			"lengthSq", [](const glm::vec4& v) { return glm::length2(v); },
			"normalize", [](const glm::vec4& v1) { return glm::normalize(v1); },
			"normalize2", [](const glm::vec4& v1, const glm::vec4& v2) { return glm::normalize(v2 - v1); },
			"nearly_zero_x", [](const glm::vec4& v) { return glm::epsilonEqual(v.x, 0.0f, 0.001f); },
			"nearly_zero_y", [](const glm::vec4& v) { return glm::epsilonEqual(v.y, 0.0f, 0.001f); },
			"nearly_zero_z", [](const glm::vec4& v) { return glm::epsilonEqual(v.z, 0.0f, 0.001f); },
			"nearly_zero_w", [](const glm::vec4& v) { return glm::epsilonEqual(v.w, 0.0f, 0.001f); }
		);
	}

	static void CreateQuaternionLuaBind(sol::state& lua)
	{
		lua.new_usertype<glm::quat>(
			"quat",
			sol::call_constructor,
			sol::constructors<glm::quat(float, float, float, float)>(),
			"x",
			&glm::quat::x,
			"y",
			&glm::quat::y,
			"z",
			&glm::quat::z,
			"w",
			&glm::quat::w,
			"normalize",
			[](const glm::quat& q) { return glm::normalize(q); },
			"conjugate",
			[](const glm::quat& q) { return glm::conjugate(q); },
			"cross",
			[](const glm::quat& q, const glm::quat& q2) { return glm::cross(q, q2); },
			"dot",
			[](const glm::quat& q, const glm::quat& q2) { return glm::dot(q, q2); });
	}

	// Helper math functions
	static void MathFreeFunctions(sol::state& lua)
	{
		lua.set_function("F_distance", sol::overload(
			[](glm::vec2& a, glm::vec2& b) { return glm::distance(a, b); },
			[](glm::vec3& a, glm::vec3& b) { return glm::distance(a, b); },
			[](glm::vec4& a, glm::vec4& b) { return glm::distance(a, b); }
		));

		lua.set_function("F_lerp", [](float a, float b, float t) { return std::lerp(a, b, t); });

		lua.set_function("F_clamp", sol::overload(
			[](float value, float min, float max) { return std::clamp(value, min, max); },
			[](double value, double min, double max) { return std::clamp(value, min, max); },
			[](int value, int min, int max) { return std::clamp(value, min, max); }
		));

		lua.set_function("F_nearly_zero", sol::overload(
			[](const glm::vec2& v) {
				return glm::epsilonEqual(v.x, 0.0f, 0.001f) && glm::epsilonEqual(v.y, 0.0f, 0.001f);
			},
			[](const glm::vec3& v) {
				return glm::epsilonEqual(v.x, 0.0f, 0.001f) && glm::epsilonEqual(v.y, 0.0f, 0.001f) && glm::epsilonEqual(v.z, 0.0f, 0.001f);
			}
		));

		lua.set_function("F_dot_product", sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) {
				return glm::dot(v1, v2);
			},
			[](const glm::vec3& v1, const glm::vec3& v2) {
				return glm::dot(v1, v2);
			},
			[](const glm::vec4& v1, const glm::vec4& v2) {
				return glm::dot(v1, v2);
			}));

		lua.set_function("F_cross_product", sol::overload(
			[](const glm::vec2& v1, const glm::vec2& v2) { 
				return v1.x * v2.y - v2.x * v1.y;
			},
			[](const glm::vec3& v1, const glm::vec3& v2) {
				return glm::cross(v1, v2);
			}));
	}

	static void MathConstants(sol::state& lua)
	{
		auto constants = MakeReadOnlyTable(lua, {
			std::make_pair("PI", 3.14159265359f),
			std::make_pair("TWO_PI", 6.28318530717f),
			std::make_pair("PI_SQUARED", 9.86960440108f),
			std::make_pair("PI_OVER_2", 1.57079632679f),
			std::make_pair("PI_OVER_4", 0.78539816339f),
			std::make_pair("PHI", 1.6180339887498948482045868343656381f),
			std::make_pair("EULERS", 2.71828182845904523536f),
			std::make_pair("SQRT_2", 1.4142135623730950488016887242097f),
			std::make_pair("SQRT_3", 1.7320508075688772935274463415059f),
			std::make_pair("INV_SQRT_2", 0.70710678118654752440084436210485f),
			std::make_pair("INV_SQRT_3", 0.57735026918962576450914878050196f)
		});

		lua["F_Constants"] = constants;
	}

	void Feather::GLMBinding::CreateGLMBindings(sol::state& lua)
	{
		CreateVec2Bind(lua);
		CreateVec3Bind(lua);
		CreateVec4Bind(lua);

		MathFreeFunctions(lua);
		MathConstants(lua);
	}

}
