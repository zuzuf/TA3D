/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_GfxTexture_H__
# define __TA3D_GfxTexture_H__

# include <QOpenGLFunctions>
# include <misc/string.h>
# include <zuzuf/smartptr.h>

namespace TA3D
{
    class GfxTexture : public zuzuf::ref_count
	{
    public:
        typedef zuzuf::smartptr<GfxTexture> Ptr;

        enum Target {
            Target1D                   = 0x0DE0,    // GL_TEXTURE_1D
            Target1DArray              = 0x8C18,    // GL_TEXTURE_1D_ARRAY
            Target2D                   = 0x0DE1,    // GL_TEXTURE_2D
            Target2DArray              = 0x8C1A,    // GL_TEXTURE_2D_ARRAY
            Target3D                   = 0x806F,    // GL_TEXTURE_3D
            TargetCubeMap              = 0x8513,    // GL_TEXTURE_CUBE_MAP
            TargetCubeMapArray         = 0x9009,    // GL_TEXTURE_CUBE_MAP_ARRAY
            Target2DMultisample        = 0x9100,    // GL_TEXTURE_2D_MULTISAMPLE
            Target2DMultisampleArray   = 0x9102,    // GL_TEXTURE_2D_MULTISAMPLE_ARRAY
            TargetRectangle            = 0x84F5,    // GL_TEXTURE_RECTANGLE
            TargetBuffer               = 0x8C2A     // GL_TEXTURE_BUFFER
        };

        enum MipMapGeneration {
            GenerateMipMaps,
            DontGenerateMipMaps
        };


        enum TextureFormat {
            NoFormat               = 0,         // GL_NONE

            // Unsigned normalized formats
            R8_UNorm               = 0x8229,    // GL_R8
            RG8_UNorm              = 0x822B,    // GL_RG8
            RGB8_UNorm             = 0x8051,    // GL_RGB8
            RGBA8_UNorm            = 0x8058,    // GL_RGBA8

            R16_UNorm              = 0x822A,    // GL_R16
            RG16_UNorm             = 0x822C,    // GL_RG16
            RGB16_UNorm            = 0x8054,    // GL_RGB16
            RGBA16_UNorm           = 0x805B,    // GL_RGBA16

            Y8_UNorm               = 0x8040,    // GL_LUMINANCE8

            // Signed normalized formats
            R8_SNorm               = 0x8F94,    // GL_R8_SNORM
            RG8_SNorm              = 0x8F95,    // GL_RG8_SNORM
            RGB8_SNorm             = 0x8F96,    // GL_RGB8_SNORM
            RGBA8_SNorm            = 0x8F97,    // GL_RGBA8_SNORM

            R16_SNorm              = 0x8F98,    // GL_R16_SNORM
            RG16_SNorm             = 0x8F99,    // GL_RG16_SNORM
            RGB16_SNorm            = 0x8F9A,    // GL_RGB16_SNORM
            RGBA16_SNorm           = 0x8F9B,    // GL_RGBA16_SNORM

            // Unsigned integer formats
            R8U                    = 0x8232,    // GL_R8UI
            RG8U                   = 0x8238,    // GL_RG8UI
            RGB8U                  = 0x8D7D,    // GL_RGB8UI
            RGBA8U                 = 0x8D7C,    // GL_RGBA8UI

            R16U                   = 0x8234,    // GL_R16UI
            RG16U                  = 0x823A,    // GL_RG16UI
            RGB16U                 = 0x8D77,    // GL_RGB16UI
            RGBA16U                = 0x8D76,    // GL_RGBA16UI

            R32U                   = 0x8236,    // GL_R32UI
            RG32U                  = 0x823C,    // GL_RG32UI
            RGB32U                 = 0x8D71,    // GL_RGB32UI
            RGBA32U                = 0x8D70,    // GL_RGBA32UI

            // Signed integer formats
            R8I                    = 0x8231,    // GL_R8I
            RG8I                   = 0x8237,    // GL_RG8I
            RGB8I                  = 0x8D8F,    // GL_RGB8I
            RGBA8I                 = 0x8D8E,    // GL_RGBA8I

            R16I                   = 0x8233,    // GL_R16I
            RG16I                  = 0x8239,    // GL_RG16I
            RGB16I                 = 0x8D89,    // GL_RGB16I
            RGBA16I                = 0x8D88,    // GL_RGBA16I

            R32I                   = 0x8235,    // GL_R32I
            RG32I                  = 0x823B,    // GL_RG32I
            RGB32I                 = 0x8D83,    // GL_RGB32I
            RGBA32I                = 0x8D82,    // GL_RGBA32I

            // Floating point formats
            R16F                   = 0x822D,    // GL_R16F
            RG16F                  = 0x822F,    // GL_RG16F
            RGB16F                 = 0x881B,    // GL_RGB16F
            RGBA16F                = 0x881A,    // GL_RGBA16F

            R32F                   = 0x822E,    // GL_R32F
            RG32F                  = 0x8230,    // GL_RG32F
            RGB32F                 = 0x8815,    // GL_RGB32F
            RGBA32F                = 0x8814,    // GL_RGBA32F

            // Packed formats
            RGB9E5                 = 0x8C3D,    // GL_RGB9_E5
            RG11B10F               = 0x8C3A,    // GL_R11F_G11F_B10F
            RG3B2                  = 0x2A10,    // GL_R3_G3_B2
            R5G6B5                 = 0x8D62,    // GL_RGB565
            RGB5A1                 = 0x8057,    // GL_RGB5_A1
            RGBA4                  = 0x8056,    // GL_RGBA4
            RGB10A2                = 0x906F,    // GL_RGB10_A2UI

            // Depth formats
            D16                    = 0x81A5,    // GL_DEPTH_COMPONENT16
            D24                    = 0x81A6,    // GL_DEPTH_COMPONENT24
            D24S8                  = 0x88F0,    // GL_DEPTH24_STENCIL8
            D32                    = 0x81A7,    // GL_DEPTH_COMPONENT32
            D32F                   = 0x8CAC,    // GL_DEPTH_COMPONENT32F
            D32FS8X24              = 0x8CAD,    // GL_DEPTH32F_STENCIL8
            S8                     = 0x8D48,    // GL_STENCIL_INDEX8

            // Compressed formats
            RGB_DXT1               = 0x83F0,    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
            RGBA_DXT1              = 0x83F1,    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
            RGBA_DXT3              = 0x83F2,    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
            RGBA_DXT5              = 0x83F3,    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
            R_ATI1N_UNorm          = 0x8DBB,    // GL_COMPRESSED_RED_RGTC1
            R_ATI1N_SNorm          = 0x8DBC,    // GL_COMPRESSED_SIGNED_RED_RGTC1
            RG_ATI2N_UNorm         = 0x8DBD,    // GL_COMPRESSED_RG_RGTC2
            RG_ATI2N_SNorm         = 0x8DBE,    // GL_COMPRESSED_SIGNED_RG_RGTC2
            RGB_BP_UNSIGNED_FLOAT  = 0x8E8F,    // GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
            RGB_BP_SIGNED_FLOAT    = 0x8E8E,    // GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
            RGB_BP_UNorm           = 0x8E8C,    // GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
            R11_EAC_UNorm          = 0x9270,    // GL_COMPRESSED_R11_EAC
            R11_EAC_SNorm          = 0x9271,    // GL_COMPRESSED_SIGNED_R11_EAC
            RG11_EAC_UNorm         = 0x9272,    // GL_COMPRESSED_RG11_EAC
            RG11_EAC_SNorm         = 0x9273,    // GL_COMPRESSED_SIGNED_RG11_EAC
            RGB8_ETC2              = 0x9274,    // GL_COMPRESSED_RGB8_ETC2
            SRGB8_ETC2             = 0x9275,    // GL_COMPRESSED_SRGB8_ETC2
            RGB8_PunchThrough_Alpha1_ETC2 = 0x9276, // GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
            SRGB8_PunchThrough_Alpha1_ETC2 = 0x9277, // GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
            RGBA8_ETC2_EAC         = 0x9278,    // GL_COMPRESSED_RGBA8_ETC2_EAC
            SRGB8_Alpha8_ETC2_EAC  = 0x9279,    // GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
            RGB8_ETC1              = 0x8D64,    // GL_ETC1_RGB8_OES
            RGBA_ASTC_4x4          = 0x93B0,    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR
            RGBA_ASTC_5x4          = 0x93B1,    // GL_COMPRESSED_RGBA_ASTC_5x4_KHR
            RGBA_ASTC_5x5          = 0x93B2,    // GL_COMPRESSED_RGBA_ASTC_5x5_KHR
            RGBA_ASTC_6x5          = 0x93B3,    // GL_COMPRESSED_RGBA_ASTC_6x5_KHR
            RGBA_ASTC_6x6          = 0x93B4,    // GL_COMPRESSED_RGBA_ASTC_6x6_KHR
            RGBA_ASTC_8x5          = 0x93B5,    // GL_COMPRESSED_RGBA_ASTC_8x5_KHR
            RGBA_ASTC_8x6          = 0x93B6,    // GL_COMPRESSED_RGBA_ASTC_8x6_KHR
            RGBA_ASTC_8x8          = 0x93B7,    // GL_COMPRESSED_RGBA_ASTC_8x8_KHR
            RGBA_ASTC_10x5         = 0x93B8,    // GL_COMPRESSED_RGBA_ASTC_10x5_KHR
            RGBA_ASTC_10x6         = 0x93B9,    // GL_COMPRESSED_RGBA_ASTC_10x6_KHR
            RGBA_ASTC_10x8         = 0x93BA,    // GL_COMPRESSED_RGBA_ASTC_10x8_KHR
            RGBA_ASTC_10x10        = 0x93BB,    // GL_COMPRESSED_RGBA_ASTC_10x10_KHR
            RGBA_ASTC_12x10        = 0x93BC,    // GL_COMPRESSED_RGBA_ASTC_12x10_KHR
            RGBA_ASTC_12x12        = 0x93BD,    // GL_COMPRESSED_RGBA_ASTC_12x12_KHR
            SRGB8_Alpha8_ASTC_4x4  = 0x93D0,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
            SRGB8_Alpha8_ASTC_5x4  = 0x93D1,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
            SRGB8_Alpha8_ASTC_5x5  = 0x93D2,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
            SRGB8_Alpha8_ASTC_6x5  = 0x93D3,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
            SRGB8_Alpha8_ASTC_6x6  = 0x93D4,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
            SRGB8_Alpha8_ASTC_8x5  = 0x93D5,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
            SRGB8_Alpha8_ASTC_8x6  = 0x93D6,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
            SRGB8_Alpha8_ASTC_8x8  = 0x93D7,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
            SRGB8_Alpha8_ASTC_10x5 = 0x93D8,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
            SRGB8_Alpha8_ASTC_10x6 = 0x93D9,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
            SRGB8_Alpha8_ASTC_10x8 = 0x93DA,    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
            SRGB8_Alpha8_ASTC_10x10 = 0x93DB,   // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
            SRGB8_Alpha8_ASTC_12x10 = 0x93DC,   // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
            SRGB8_Alpha8_ASTC_12x12 = 0x93DD,   // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR

            // sRGB formats
            SRGB8                  = 0x8C41,    // GL_SRGB8
            SRGB8_Alpha8           = 0x8C43,    // GL_SRGB8_ALPHA8
            SRGB_DXT1              = 0x8C4C,    // GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
            SRGB_Alpha_DXT1        = 0x8C4D,    // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
            SRGB_Alpha_DXT3        = 0x8C4E,    // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
            SRGB_Alpha_DXT5        = 0x8C4F,    // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
            SRGB_BP_UNorm          = 0x8E8D,    // GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB

            // ES 2 formats
            DepthFormat            = 0x1902,    // GL_DEPTH_COMPONENT
            AlphaFormat            = 0x1906,    // GL_ALPHA
            RGBFormat              = 0x1907,    // GL_RGB
            RGBAFormat             = 0x1908,    // GL_RGBA
            LuminanceFormat        = 0x1909,    // GL_LUMINANCE
            LuminanceAlphaFormat   = 0x190A

        };

        enum PixelFormat {
            NoSourceFormat = 0,         // GL_NONE
            Red            = 0x1903,    // GL_RED
            RG             = 0x8227,    // GL_RG
            RGB            = 0x1907,    // GL_RGB
            BGR            = 0x80E0,    // GL_BGR
            RGBA           = 0x1908,    // GL_RGBA
            BGRA           = 0x80E1,    // GL_BGRA
            Red_Integer    = 0x8D94,    // GL_RED_INTEGER
            RG_Integer     = 0x8228,    // GL_RG_INTEGER
            RGB_Integer    = 0x8D98,    // GL_RGB_INTEGER
            BGR_Integer    = 0x8D9A,    // GL_BGR_INTEGER
            RGBA_Integer   = 0x8D99,    // GL_RGBA_INTEGER
            BGRA_Integer   = 0x8D9B,    // GL_BGRA_INTEGER
            Stencil        = 0x1901,    // GL_STENCIL_INDEX
            Depth          = 0x1902,    // GL_DEPTH_COMPONENT
            DepthStencil   = 0x84F9,    // GL_DEPTH_STENCIL
            Alpha          = 0x1906,    // GL_ALPHA
            Luminance      = 0x1909,    // GL_LUMINANCE
            LuminanceAlpha = 0x190A     // GL_LUMINANCE_ALPHA
        };

        enum PixelType {
            NoPixelType        = 0,         // GL_NONE
            Int8               = 0x1400,    // GL_BYTE
            UInt8              = 0x1401,    // GL_UNSIGNED_BYTE
            Int16              = 0x1402,    // GL_SHORT
            UInt16             = 0x1403,    // GL_UNSIGNED_SHORT
            Int32              = 0x1404,    // GL_INT
            UInt32             = 0x1405,    // GL_UNSIGNED_INT
            Float16            = 0x140B,    // GL_HALF_FLOAT
            Float16OES         = 0x8D61,    // GL_HALF_FLOAT_OES
            Float32            = 0x1406,    // GL_FLOAT
            UInt32_RGB9_E5     = 0x8C3E,    // GL_UNSIGNED_INT_5_9_9_9_REV
            UInt32_RG11B10F    = 0x8C3B,    // GL_UNSIGNED_INT_10F_11F_11F_REV
            UInt8_RG3B2        = 0x8032,    // GL_UNSIGNED_BYTE_3_3_2
            UInt8_RG3B2_Rev    = 0x8362,    // GL_UNSIGNED_BYTE_2_3_3_REV
            UInt16_RGB5A1      = 0x8034,    // GL_UNSIGNED_SHORT_5_5_5_1
            UInt16_RGB5A1_Rev  = 0x8366,    // GL_UNSIGNED_SHORT_1_5_5_5_REV
            UInt16_R5G6B5      = 0x8363,    // GL_UNSIGNED_SHORT_5_6_5
            UInt16_R5G6B5_Rev  = 0x8364,    // GL_UNSIGNED_SHORT_5_6_5_REV
            UInt16_RGBA4       = 0x8033,    // GL_UNSIGNED_SHORT_4_4_4_4
            UInt16_RGBA4_Rev   = 0x8365,    // GL_UNSIGNED_SHORT_4_4_4_4_REV
            UInt32_RGBA8       = 0x8035,    // GL_UNSIGNED_INT_8_8_8_8
            UInt32_RGBA8_Rev   = 0x8367,    // GL_UNSIGNED_INT_8_8_8_8_REV
            UInt32_RGB10A2     = 0x8036,    // GL_UNSIGNED_INT_10_10_10_2
            UInt32_RGB10A2_Rev = 0x8368,    // GL_UNSIGNED_INT_2_10_10_10_REV
            UInt32_D24S8       = 0x84FA,    // GL_UNSIGNED_INT_24_8
            Float32_D32_UInt32_S8_X24 = 0x8DAD // GL_FLOAT_32_UNSIGNED_INT_24_8_REV
        };

        enum WrapMode {
            Repeat         = 0x2901, // GL_REPEAT
            MirroredRepeat = 0x8370, // GL_MIRRORED_REPEAT
            ClampToEdge    = 0x812F, // GL_CLAMP_TO_EDGE
            ClampToBorder  = 0x812D  // GL_CLAMP_TO_BORDER
        };

        enum Filter {
            Nearest                 = 0x2600,   // GL_NEAREST
            Linear                  = 0x2601,   // GL_LINEAR
            NearestMipMapNearest    = 0x2700,   // GL_NEAREST_MIPMAP_NEAREST
            NearestMipMapLinear     = 0x2702,   // GL_NEAREST_MIPMAP_LINEAR
            LinearMipMapNearest     = 0x2701,   // GL_LINEAR_MIPMAP_NEAREST
            LinearMipMapLinear      = 0x2703    // GL_LINEAR_MIPMAP_LINEAR
        };

        enum ComparisonFunction {
            CompareLessEqual    = 0x0203,   // GL_LEQUAL
            CompareGreaterEqual = 0x0206,   // GL_GEQUAL
            CompareLess         = 0x0201,   // GL_LESS
            CompareGreater      = 0x0204,   // GL_GREATER
            CompareEqual        = 0x0202,   // GL_EQUAL
            CommpareNotEqual    = 0x0205,   // GL_NOTEQUAL
            CompareAlways       = 0x0207,   // GL_ALWAYS
            CompareNever        = 0x0200    // GL_NEVER
        };
        enum ComparisonMode {
            CompareRefToTexture = 0x884E,   // GL_COMPARE_REF_TO_TEXTURE
            CompareNone         = 0x0000    // GL_NONE
        };

    public:
        GfxTexture(Target target);
        GfxTexture(const QImage &image, MipMapGeneration genMipMaps = GenerateMipMaps);
		~GfxTexture();

        int height() const;
        int width() const;
        void setWrapMode(WrapMode mode);
        void setAutoMipMapGenerationEnabled(MipMapGeneration genMipMaps = GenerateMipMaps);
        void setMinificationFilter(Filter filter);
        void setMagnificationFilter(Filter filter);
        void setData(PixelFormat pixel_format, PixelType pixel_type, const void *data);
        void setFormat(TextureFormat format);
        void setMaximumAnisotropy(float max_anisotropy);
        void allocateStorage(PixelFormat pixel_format, PixelType pixel_type);
        void allocateStorage();

        void setComparisonFunction(ComparisonFunction function);
        void setComparisonMode(ComparisonMode mode);

        void setSize(int w, int h);

        GLint textureId() const;

        bool bind();

		void draw(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawRotated(const float x1, const float y1, const float angle, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawCentered(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);

    private:
        Target m_target;
        GLuint m_handle;
        int m_width;
        int m_height;
        TextureFormat m_format;
    }; // class GfxTexture
}

#endif // __TA3D_GfxTexture_H__
