#include "texture.h"

#include "ccfile.h"
#include "keyframe.h"

#define SHAPE_BUFFER_SIZE 131072L // 128KB

void* Get_Shape_Header_Data(void* ptr);

static inline const auto& Logger = CncLogger::For(Texture);

Texture::Texture(std::string shape_filename, const unsigned short shape_number)
    : ShapeFilename(std::move(shape_filename))
    , ShapeNumber(shape_number)
    , Loaded(false)
    , ShapeData(std::make_unique<unsigned char[]>(SHAPE_BUFFER_SIZE))
    , ShapeHeader(nullptr)
    , ShapeWidth(0)
    , ShapeHeight(0)
{
}

bool Texture::Load(std::optional<int> source_shape_width, std::optional<int> source_shape_height)
{
    if (Loaded) {
        return true;
    }

    CNC_LOGGER_DEBUG("Loading texture from file: {} (index={})", ShapeFilename, ShapeNumber);

    const auto shape_file = MixFileClass<CCFileClass>::Retrieve(ShapeFilename.c_str());
    const auto shape_size = Build_Frame(shape_file, ShapeNumber, ShapeData.get());

    const auto shape_buffer_overflow = Get_Last_Frame_Length() > SHAPE_BUFFER_SIZE;

    if (shape_buffer_overflow) {
        CNC_LOGGER_WARN(
            "Attempted to load shape bigger than buffer: {} > {}",
            Get_Last_Frame_Length(),
            SHAPE_BUFFER_SIZE
        );
    }

    if (shape_buffer_overflow || shape_size == 0) {
        CNC_LOGGER_ERROR("Failed to load shape from file: {} (index={})", ShapeFilename, ShapeNumber);
        return false;
    }

    ShapeHeader = static_cast<unsigned char*>(Get_Shape_Header_Data(reinterpret_cast<void*>(shape_size)));
    ShapeWidth = source_shape_width.value_or(Get_Build_Frame_Width(shape_file));
    ShapeHeight = source_shape_height.value_or(Get_Build_Frame_Height(shape_file));

    Loaded = true;

    return true;
}

void Texture::Draw_Rectangle(
    GraphicViewPortClass& viewport,
    const int x,
    const int y,
    const int width,
    const int height
) const
{
    if (!Loaded) {
        CNC_LOGGER_WARN("Attempted to draw texture before it was successfully loaded");
        return;
    }

    viewport.Texture_Fill_Rect(
        x,
        y,
        width,
        height,
        ShapeHeader,
        ShapeWidth,
        ShapeHeight
    );
}
