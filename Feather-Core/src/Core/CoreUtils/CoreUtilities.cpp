#include "CoreUtilities.h"

#include "Core/ECS/Components/AllComponents.h"
#include "Core/Scene/Scene.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Registry.h"
#include "Utils/MathUtilities.h"

namespace Feather {

    bool EntityInView(const TransformComponent& transform, float width, float height, const Camera2D& camera)
    {
		const glm::vec2 cameraPos = camera.GetPosition() - camera.GetScreenOffset();
		const int cameraWidth = camera.GetWidth();
		const int cameraHeight = camera.GetHeight();
		const float cameraScale = camera.GetScale();
		const float invCameraScale = 1.0f / cameraScale;

		const float cameraLeft = (cameraPos.x - (width * transform.scale.x * cameraScale)) * invCameraScale;
		const float cameraRight = (cameraPos.x + cameraWidth) * invCameraScale;
		const float cameraTop = (cameraPos.y - (height * transform.scale.y * cameraScale)) * invCameraScale;
		const float cameraBottom = (cameraPos.y + cameraHeight) * invCameraScale;

		if ((transform.position.x <= cameraLeft || transform.position.x >= cameraRight) ||
			(transform.position.y <= cameraTop || transform.position.y >= cameraBottom))
		{
			return false;
		}

		// Entity is at least partially in view
        return true;
    }

    glm::mat4 TRSModel(const TransformComponent& transform, float width, float height)
    {
		glm::mat4 model{ 1.0f };

		if (transform.rotation > 0.0f || transform.rotation < 0.0f ||
			transform.scale.x > 1.0f  || transform.scale.x < 1.0f  ||
			transform.scale.y > 1.0f  || transform.scale.y < 1.0f)
		{
			model = glm::translate(model, glm::vec3{ transform.position, 0.0f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * 0.5f, (height * transform.scale.y) * 0.5f, 0.0f });
			model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.0f, 0.0f, 1.0f });
			model = glm::translate(model, glm::vec3{ (width * transform.scale.x) * -0.5f, (height * transform.scale.y) * -0.5f, 0.0f });
			model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });
			model = glm::translate(model, glm::vec3{ -transform.position, 0.0f });
		}

		return model;
    }

	void GenerateUVs(SpriteComponent& sprite, int textureWidth, int textureHeight)
	{
		sprite.uvs.uv_width = sprite.width / textureWidth;
		sprite.uvs.uv_height = sprite.height / textureHeight;

		sprite.uvs.u = sprite.start_x * sprite.uvs.uv_width;
		sprite.uvs.v = sprite.start_y * sprite.uvs.uv_height;
	}

	void GenerateUVsExt(SpriteComponent& sprite, int textureWidth, int textureHeight, float u, float v)
	{
		sprite.uvs.uv_width = sprite.width / textureWidth;
		sprite.uvs.uv_height = sprite.height / textureHeight;

		sprite.uvs.u = u;
		sprite.uvs.v = v;
	}

	std::tuple<int, int> ConvertWorldPosToIsoCoords(const glm::vec2& position, const Canvas& canvas)
	{
		// TODO: Enforce width being double height. Double width hack for now. 
		float doubleWidth = canvas.tileWidth * 2.0f;

		// Move the x position back the halfWidth of one tile.
		// We are not currently using any offset for we have a camera that can go into the negatives.
		float xPos = (position.x /*- canvas.offset.x*/) - doubleWidth * 0.5f;
		// Stretch the Y position by 2 in the negative to make the tile a square. Allows for easier rotations.
		float yPos = position.y * -2.0f;

		// Rotate both the x and the y positions by 45 degrees
		float px = xPos * cos(PIOver4) - yPos * sin(PIOver4);
		float py = xPos * sin(PIOver4) + yPos * cos(PIOver4);

		float diagonal = canvas.tileHeight * sqrt(2.0f);

		int cellX = static_cast<int>(px / diagonal);
		int cellY = static_cast<int>(-py / diagonal);

		return std::make_tuple(cellX, cellY);
	}

	std::tuple<float, float> GetTextBlockSize(const TextComponent& textComp, const TransformComponent& transform, AssetManager& assetManager)
	{
		const auto& pFont = assetManager.GetFont(textComp.fontName);
		if (!pFont)
		{
			F_ERROR("Failed to get font '{}': Does not exist in asset manager!", textComp.fontName);
			return std::make_tuple(-1.0f, -1.0f);
		}
		float fontSize = pFont->GetFontSize();
		// There is no wrap, find width, height is not needed
		if (textComp.wrap < 0.0f || textComp.wrap < fontSize)
		{
			glm::vec2 position{ 0.0f }, temp_pos{ position };
			for (const auto& character : textComp.textStr)
			{
				pFont->GetNextCharPos(character, temp_pos);
			}

			const auto& paddingInfo = pFont->AveragePaddingInfo();
			return std::make_tuple(std::abs((position - temp_pos).x - (paddingInfo.paddingX * 0.5f)), 1 * fontSize + (paddingInfo.paddingY * 0.5f));
		}

		// Calculate Text boxSize
		int numRows{ 0 };
		glm::vec2 pos{ transform.position }, temp_pos{ pos };
		std::string text_holder{};

		float wrap{ textComp.wrap };

		int infiniteLoopCheck{ 0 };

		for (int i = 0; i < textComp.textStr.size(); i++)
		{
			if (infiniteLoopCheck >= SANITY_LOOP_CHECK)
			{
				F_ERROR("Failed to get Text Box size correctly. Please check you text wrap, padding, textStr, etc");
				return std::make_tuple(-1.0f, -1.0f);
			}

			char character = textComp.textStr[i];

			text_holder += character;
			bool bNewLine = character == '\n';
			size_t text_size = text_holder.size();
			pFont->GetNextCharPos(character, temp_pos);

			if (text_size > 0 && (temp_pos.x > (wrap + pos.x) || character == '\0' || bNewLine))
			{
				if (!bNewLine)
				{
					while (textComp.textStr[i] != ' ' && textComp.textStr[i] != '.' &&
						   textComp.textStr[i] != '!' && textComp.textStr[i] != '?' && text_size > 0)
					{
						i--;
						text_holder.pop_back();
						text_size = text_holder.size();
						infiniteLoopCheck++;
						if (i < 0)
						{
							F_ERROR("Text wrap is too small!");
							return std::make_tuple(-1.0f, -1.0f);
						}
					}
				}
				else
				{
					text_holder.pop_back();
					text_size = text_holder.size();
				}

				if (text_size > 0)
				{
					if (std::isalpha(text_holder[0]))
					{
						temp_pos = pos;
						text_holder.clear();
						infiniteLoopCheck = 0;
						numRows++;
					}
					else
					{
						text_holder.erase(0, 1);
						temp_pos.x -= fontSize;
					}
				}
			}
		}

		if (!text_holder.empty())
			numRows++;

		float height = numRows * fontSize;
		if (height < fontSize)
			height = fontSize;

		const auto& paddingInfo = pFont->AveragePaddingInfo();
		return std::make_tuple(wrap - paddingInfo.paddingX * 0.5f, height + paddingInfo.paddingY * 0.5f);
	}

	void UpdateDirtyEntities(Registry& registry)
	{
		auto& reg = registry.GetRegistry();
		auto view = reg.view<TransformComponent>();
		for (auto entity : view)
		{
			auto& transform = view.get<TransformComponent>(entity);
			if (transform.isDirty)
			{
				transform.isDirty = false;
			}

			if (reg.all_of<TextComponent>(entity))
			{
				auto& text = reg.get<TextComponent>(entity);
				text.isDirty = false;
			}
		}
	}

}
