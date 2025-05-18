#pragma once

#include "Core/ECS/Components/AllComponents.h"
#include "Renderer/Core/Camera2D.h"

namespace Feather {

	class AssetManager;
	class Registry;

	/**
	* @brief Checks if an entity is fully within the camera's view before culling.
	*
	* This function performs an **axis-aligned bounding box (AABB) check** to determine
	* if an entity is **completely out of view** before marking it for culling.
	* It ensures that objects are not removed too early when only partially outside
	* the viewport.
	* @param transform The entity's transform component (position, scale).
	* @param width The entity's width in world space.
	* @param height The entity's height in world space.
	* @param camera The camera used for visibility checking.
	* @return True if the entity is at least partially visible, false if fully out of view.
	*/
	bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera);

	/**
	* @brief Constructs an TRS (Translation, Rotation, Scale) transformation matrix.
	*
	* Generates a transformation matrix based on the given position, rotation, and scale,
	* applying translation, rotation around the Z-axis, and scaling.
	*
	* @param transform The transform component containing position, rotation, and scale.
	* @param width The object's width, used for pivot adjustments.
	* @param height The object's height, used for pivot adjustments.
	* @return The computed transformation matrix.
	*/
	glm::mat4 TRSModel(const TransformComponent& transform, float width, float height);

	/**
	* @brief Generates UV coordinates for a sprite based on its dimensions and texture size.
	*
	* Computes normalized UV coordinates by mapping the sprite's position and size within the texture.
	*
	* @param sprite The sprite component to update with UV coordinates.
	* @param textureWidth The width of the texture.
	* @param textureHeight The height of the texture.
	*/
	void GenerateUVs(SpriteComponent& sprite, int textureWidth, int textureHeight);

	void GenerateUVsExt(SpriteComponent& sprite, int textureWidth, int textureHeight, float u, float v);

	/**
	* @brief Converts world coordinates to isometric grid coordinates.
	*
	* Applies transformations to map a world position to an isometric tile grid,
	* considering tile size and rotation.
	*
	* @param position The world position to convert.
	* @param canvas The canvas defining tile dimensions.
	* @return A tuple containing the isometric grid coordinates (x, y).
	*/
	std::tuple<int, int> ConvertWorldPosToIsoCoords(const glm::vec2& position, const struct Canvas& canvas);

	/**
	* @brief Returns the pixel size of a text block based on font metrics and wrapping settings.
	*
	* Calculates the width and height required to render the text from a TextComponent,
	* using font data from the AssetManager and optional word wrapping.
	*
	* @param textComp Text data including string, font, and wrap width.
	* @param transform Starting position for text rendering.
	* @param assetManager Asset manager used to retrieve the font.
	* @return A tuple (width, height) representing the text block size in pixels.
	*/
	std::tuple<float, float> GetTextBlockSize(const TextComponent& textComp, const TransformComponent& transform, AssetManager& assetManager);

	/**
	 * @brief Resets the dirty flags on all necessary components in the registry.
	 * Marks updated entities as clean by clearing their `bDirty` flags.
	 */
	void UpdateDirtyEntities(Registry& registry);

	/* Target time per frame. Used to help clamp delta time */
	constexpr double TARGET_FRAME_TIME = 1.0 / 60.0;
	/* Target time per frame. Used for Box2D step */
	constexpr float TARGET_FRAME_TIME_F = 1.0f / 60.0f;
	/* Used to prevent specific loops from looping forever */
	constexpr int SANITY_LOOP_CHECK = 100;

}
