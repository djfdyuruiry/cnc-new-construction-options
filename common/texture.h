#pragma once

#include "graphicsviewport.h"

#include <memory>
#include <optional>
#include <string>

/**
 * Stores a texture in memory and provides convenience methods for drawing the texture onto a viewport.
 *
 * Call Load at least once before calling Draw; Load must return true.
 *
 * OOP version of CC_Texture_Fill from Tiberian Dawn with caching.
 */
class Texture
{
public:
    Texture(std::string shape_filename, unsigned short shape_number);

    /**
     * Attempt to load the data for this texture from Texture::ShapeFilename.
     *
     * This method is idempotent.
     *
     * @param source_shape_width Pass this to override the width of the source shape
     * @param source_shape_height Pass this to override the height of the source shape
     * @return Is the texture Loaded?
     */
    bool Load(std::optional<int> source_shape_width = std::nullopt, std::optional<int> source_shape_height = std::nullopt);

    /**
     * Draw this texture to a rectangle on the given viewport, with the target dimensions.
     *
     * Texture::Load must have been called and return true, otherwise this method will do nothing.
     */
    void Draw_Rectangle(GraphicViewPortClass& viewport, int x, int y, int width, int height) const;

private:
    // source info
    std::string ShapeFilename;
    unsigned short ShapeNumber;

    // texture state
    bool Loaded;

    // shape file data
    std::unique_ptr<unsigned char[]> ShapeData;
    unsigned char* ShapeHeader;

    // shape header data
    int ShapeWidth;
    int ShapeHeight;
};
