/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <array>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <imgui/imgui_impl_bgfx.h>

#include "imgui/fs_imgui_image.bin.h"
#include "imgui/fs_ocornut_imgui.bin.h"
#include "imgui/vs_imgui_image.bin.h"
#include "imgui/vs_ocornut_imgui.bin.h"

// #include "roboto_regular.ttf.h"
// #include "robotomono_regular.ttf.h"
// #include "icons_kenney.ttf.h"
// #include "icons_font_awesome.ttf.h"

static const std::array<bgfx::EmbeddedShader, 5> embeddedShaders = {{
	{.name = "vs_ocornut_imgui",
	 .data
	 = {{.type = bgfx ::RendererType ::Direct3D11,
		 .data = vs_ocornut_imgui_dxil, // NOLINT
		 .size
		 = (sizeof(vs_ocornut_imgui_dxil) / sizeof(vs_ocornut_imgui_dxil[0]))},
		{.type = bgfx ::RendererType ::OpenGLES,
		 .data = vs_ocornut_imgui_essl, // NOLINT
		 .size
		 = (sizeof(vs_ocornut_imgui_essl) / sizeof(vs_ocornut_imgui_essl[0]))},
		{.type = bgfx ::RendererType ::OpenGL,
		 .data = vs_ocornut_imgui_glsl, // NOLINT
		 .size
		 = (sizeof(vs_ocornut_imgui_glsl) / sizeof(vs_ocornut_imgui_glsl[0]))},
		{.type = bgfx ::RendererType ::Vulkan,
		 .data = vs_ocornut_imgui_spv, // NOLINT
		 .size
		 = (sizeof(vs_ocornut_imgui_spv) / sizeof(vs_ocornut_imgui_spv[0]))},
		{.type = bgfx ::RendererType ::WebGPU,
		 .data = vs_ocornut_imgui_wgsl, // NOLINT
		 .size
		 = (sizeof(vs_ocornut_imgui_wgsl) / sizeof(vs_ocornut_imgui_wgsl[0]))},
		{.type = bgfx ::RendererType ::Noop,
		 .data = (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0",
		 .size = 10},
		{.type = bgfx::RendererType::Count, .data = nullptr, .size = 0}}},

	{.name = "fs_ocornut_imgui",
	 .data
	 = {{.type = bgfx ::RendererType ::Direct3D11,
		 .data = fs_ocornut_imgui_dxil, // NOLINT
		 .size
		 = (sizeof(fs_ocornut_imgui_dxil) / sizeof(fs_ocornut_imgui_dxil[0]))},
		{.type = bgfx ::RendererType ::OpenGLES,
		 .data = fs_ocornut_imgui_essl, // NOLINT
		 .size
		 = (sizeof(fs_ocornut_imgui_essl) / sizeof(fs_ocornut_imgui_essl[0]))},
		{.type = bgfx ::RendererType ::OpenGL,
		 .data = fs_ocornut_imgui_glsl, // NOLINT
		 .size
		 = (sizeof(fs_ocornut_imgui_glsl) / sizeof(fs_ocornut_imgui_glsl[0]))},
		{.type = bgfx ::RendererType ::Vulkan,
		 .data = fs_ocornut_imgui_spv, // NOLINT
		 .size
		 = (sizeof(fs_ocornut_imgui_spv) / sizeof(fs_ocornut_imgui_spv[0]))},
		{.type = bgfx ::RendererType ::WebGPU,
		 .data = fs_ocornut_imgui_wgsl, // NOLINT
		 .size
		 = (sizeof(fs_ocornut_imgui_wgsl) / sizeof(fs_ocornut_imgui_wgsl[0]))},
		{.type = bgfx ::RendererType ::Noop,
		 .data = (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0",
		 .size = 10},
		{.type = bgfx ::RendererType ::Count, .data = nullptr, .size = 0}}},

	{.name = "vs_imgui_image",
	 .data
	 = {{.type = bgfx ::RendererType ::Direct3D11,
		 .data = vs_imgui_image_dxil, // NOLINT
		 .size
		 = (sizeof(vs_imgui_image_dxil) / sizeof(vs_imgui_image_dxil[0]))},
		{.type = bgfx ::RendererType ::OpenGLES,
		 .data = vs_imgui_image_essl, // NOLINT
		 .size
		 = (sizeof(vs_imgui_image_essl) / sizeof(vs_imgui_image_essl[0]))},
		{.type = bgfx ::RendererType ::OpenGL,
		 .data = vs_imgui_image_glsl, // NOLINT
		 .size
		 = (sizeof(vs_imgui_image_glsl) / sizeof(vs_imgui_image_glsl[0]))},
		{.type = bgfx ::RendererType ::Vulkan,
		 .data = vs_imgui_image_spv, // NOLINT
		 .size = (sizeof(vs_imgui_image_spv) / sizeof(vs_imgui_image_spv[0]))},
		{.type = bgfx ::RendererType ::WebGPU,
		 .data = vs_imgui_image_wgsl, // NOLINT
		 .size
		 = (sizeof(vs_imgui_image_wgsl) / sizeof(vs_imgui_image_wgsl[0]))},
		{.type = bgfx ::RendererType ::Noop,
		 .data = (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0",
		 .size = 10},
		{.type = bgfx ::RendererType ::Count, .data = nullptr, .size = 0}}},

	{.name = "fs_imgui_image",
	 .data
	 = {{.type = bgfx ::RendererType ::Direct3D11,
		 .data = fs_imgui_image_dxil, // NOLINT
		 .size
		 = (sizeof(fs_imgui_image_dxil) / sizeof(fs_imgui_image_dxil[0]))},
		{.type = bgfx ::RendererType ::OpenGLES,
		 .data = fs_imgui_image_essl, // NOLINT
		 .size
		 = (sizeof(fs_imgui_image_essl) / sizeof(fs_imgui_image_essl[0]))},
		{.type = bgfx ::RendererType ::OpenGL,
		 .data = fs_imgui_image_glsl, // NOLINT
		 .size
		 = (sizeof(fs_imgui_image_glsl) / sizeof(fs_imgui_image_glsl[0]))},
		{.type = bgfx ::RendererType ::Vulkan,
		 .data = fs_imgui_image_spv, // NOLINT
		 .size = (sizeof(fs_imgui_image_spv) / sizeof(fs_imgui_image_spv[0]))},
		{.type = bgfx ::RendererType ::WebGPU,
		 .data = fs_imgui_image_wgsl, // NOLINT
		 .size
		 = (sizeof(fs_imgui_image_wgsl) / sizeof(fs_imgui_image_wgsl[0]))},
		{.type = bgfx ::RendererType ::Noop, // NOLINTNEXTLINE
		 .data = reinterpret_cast<const uint8_t*>("VSH\x5\x0\x0\x0\x0\x0\x0"),
		 .size = 10},
		{.type = bgfx ::RendererType ::Count, .data = nullptr, .size = 0}}},

	{.name = nullptr,
	 .data
	 = {{.type = bgfx ::RendererType ::Count, .data = nullptr, .size = 0}}},
}};

static void* memAlloc(size_t _size, void* _userData);
static void memFree(void* _ptr, void* _userData);

struct OcornutImguiContext
{
	void render(ImDrawData* _drawData)
	{
		if (NULL != _drawData->Textures)
		{
			for (ImTextureData* texData : *_drawData->Textures)
			{
				switch (texData->Status)
				{
				case ImTextureStatus_WantCreate:
				{
					ImGui::TextureBgfx tex = {
						.handle = bgfx::createTexture2D(
							(uint16_t)texData->Width, (uint16_t)texData->Height,
							false, 1, bgfx::TextureFormat::BGRA8, 0),
						.flags = IMGUI_FLAGS_ALPHA_BLEND,
						.mip = 0,
						.unused = 0,
					};

					bgfx::setName(tex.handle, "ImGui Font Atlas");
					bgfx::updateTexture2D(
						tex.handle, 0, 0, 0, 0,
						bx::narrowCast<uint16_t>(texData->Width),
						bx::narrowCast<uint16_t>(texData->Height),
						bgfx::copy(texData->GetPixels(),
								   texData->GetSizeInBytes()));

					texData->SetTexID(bx::bitCast<ImTextureID>(tex));
					texData->SetStatus(ImTextureStatus_OK);
				}
				break;

				case ImTextureStatus_WantDestroy:
				{
					ImGui::TextureBgfx tex
						= bx::bitCast<ImGui::TextureBgfx>(texData->GetTexID());
					bgfx::destroy(tex.handle);
					texData->SetTexID(ImTextureID_Invalid);
					texData->SetStatus(ImTextureStatus_Destroyed);
				}
				break;

				case ImTextureStatus_WantUpdates:
				{
					ImGui::TextureBgfx tex
						= bx::bitCast<ImGui::TextureBgfx>(texData->GetTexID());

					for (ImTextureRect& rect : texData->Updates)
					{
						const uint32_t bpp = texData->BytesPerPixel;
						const bgfx::Memory* pix
							= bgfx::alloc(rect.h * rect.w * bpp);
						bx::gather(pix->data,
								   texData->GetPixelsAt(rect.x, rect.y),
								   texData->GetPitch(), rect.w * bpp, rect.h);
						bgfx::updateTexture2D(tex.handle, 0, 0, rect.x, rect.y,
											  rect.w, rect.h, pix);
					}
				}
				break;

				default:
					break;
				}
			}
		}

		// Avoid rendering when minimized, scale coordinates for retina displays
		// (screen coordinates != framebuffer coordinates)
		auto dispWidth = static_cast<int32_t>(_drawData->DisplaySize.x
											  * _drawData->FramebufferScale.x);
		auto dispHeight = static_cast<int32_t>(_drawData->DisplaySize.y
											   * _drawData->FramebufferScale.y);
		if (dispWidth <= 0 || dispHeight <= 0)
		{
			return;
		}

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);
		// bgfx::setViewClear(m_viewId, BGFX_CLEAR_COLOR, 0x00000000, 1.0f, 0);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = _drawData->DisplayPos.x;
			float y = _drawData->DisplayPos.y;
			float width = _drawData->DisplaySize.x;
			float height = _drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f,
						 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_viewId, NULL, ortho);
			bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width),
							  uint16_t(height));
		}

		const ImVec2 clipPos
			= _drawData->DisplayPos; // (0,0) unless using multi-viewports
		const ImVec2 clipScale
			= _drawData->FramebufferScale; // (1,1) unless using retina display
										   // which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb{};
			bgfx::TransientIndexBuffer tib{};

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			auto numVertices
				= static_cast<uint32_t>(drawList->VtxBuffer.size());
			auto numIndices = static_cast<uint32_t>(drawList->IdxBuffer.size());

			bool checkAvailTransientBuffers{
				numVertices
				== bgfx::getAvailTransientVertexBuffer(numVertices, m_layout)
				&& (0
					== numIndices
					|| numIndices
					== bgfx::getAvailTransientIndexBuffer(numIndices))};
			if (!checkAvailTransientBuffers)
			{
				// not enough space in transient buffer just quit drawing the
				// rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
			bgfx::allocTransientIndexBuffer(&tib, numIndices,
											sizeof(ImDrawIdx) == 4);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			bx::memCopy(verts, drawList->VtxBuffer.begin(),
						numVertices * sizeof(ImDrawVert));

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			bx::memCopy(indices, drawList->IdxBuffer.begin(),
						numIndices * sizeof(ImDrawIdx));

			bgfx::Encoder* encoder = bgfx::begin();

			for (const ImDrawCmd *cmd = drawList->CmdBuffer.begin(),
								 *cmdEnd = drawList->CmdBuffer.end();
				 cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
									 | BGFX_STATE_WRITE_RGB
									 | BGFX_STATE_WRITE_A
									 | BGFX_STATE_BLEND_ALPHA
									 | BGFX_STATE_MSAA;

					bgfx::TextureHandle th = BGFX_INVALID_HANDLE;
					bgfx::ProgramHandle program = m_program;

					const ImTextureID texId = cmd->GetTexID();

					if (ImTextureID_Invalid != texId)
					{
						ImGui::TextureBgfx tex
							= bx::bitCast<ImGui::TextureBgfx>(texId);

						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & tex.flags)
									 ? BGFX_STATE_BLEND_FUNC(
										   BGFX_STATE_BLEND_SRC_ALPHA,
										   BGFX_STATE_BLEND_INV_SRC_ALPHA)
									 : BGFX_STATE_NONE;
						th = tex.handle;

						if (0 != tex.mip)
						{
							const float lodEnabled[4]
								= {float(tex.mip), 1.0f, 0.0f, 0.0f};
							bgfx::setUniform(u_imageLodEnabled, lodEnabled);
							program = m_imageProgram;
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(
							BGFX_STATE_BLEND_SRC_ALPHA,
							BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					// Project scissor/clipping rectangles into framebuffer
					// space
					ImVec4 clipRect;
					clipRect.x = (cmd->ClipRect.x - clipPos.x) * clipScale.x;
					clipRect.y = (cmd->ClipRect.y - clipPos.y) * clipScale.y;
					clipRect.z = (cmd->ClipRect.z - clipPos.x) * clipScale.x;
					clipRect.w = (cmd->ClipRect.w - clipPos.y) * clipScale.y;

					if (clipRect.x
						< dispWidth
						&& clipRect.y
						< dispHeight
						&& clipRect.z
						>= 0.0f
						&& clipRect.w
						>= 0.0f)
					{
						const uint16_t xx = uint16_t(bx::max(clipRect.x, 0.0f));
						const uint16_t yy = uint16_t(bx::max(clipRect.y, 0.0f));
						encoder->setScissor(
							xx, yy,
							uint16_t(bx::min(clipRect.z, 65535.0f) - xx),
							uint16_t(bx::min(clipRect.w, 65535.0f) - yy));

						encoder->setState(state);
						encoder->setTexture(0, s_tex, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset,
												 numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset,
												cmd->ElemCount);
						encoder->submit(m_viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}

	void create(float _fontSize, bx::AllocatorI* _allocator)
	{
		IMGUI_CHECKVERSION();

		m_allocator = _allocator;

		if (NULL == _allocator)
		{
			static bx::DefaultAllocator allocator;
			m_allocator = &allocator;
		}

		m_viewId = 255;
		m_lastScroll = 0;
		m_last = bx::getHPCounter();

		ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

		m_imgui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = nullptr;

		setupStyle(true);

		io.BackendFlags |= 0
						   | ImGuiBackendFlags_RendererHasVtxOffset
						   | ImGuiBackendFlags_RendererHasTextures;
		io.ConfigDebugHighlightIdConflicts = !!BX_CONFIG_DEBUG;

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		m_program = bgfx::createProgram(
			bgfx::createEmbeddedShader(embeddedShaders.data(), type,
									   "vs_ocornut_imgui"),
			bgfx::createEmbeddedShader(embeddedShaders.data(), type,
									   "fs_ocornut_imgui"),
			true);

		u_imageLodEnabled
			= bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		m_imageProgram = bgfx::createProgram(
			bgfx::createEmbeddedShader(embeddedShaders.data(), type,
									   "vs_imgui_image"),
			bgfx::createEmbeddedShader(embeddedShaders.data(), type,
									   "fs_imgui_image"),
			true);

		m_layout.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		// ImGuizmo::Create();

		// ImGui::InitDockContext();
	}

	void destroy()
	{
		// ImGuizmo::Destroy();

		for (ImTextureData* texData : ImGui::GetPlatformIO().Textures)
		{
			if (1 == texData->RefCount)
			{
				auto tex = bx::bitCast<ImGui::TextureBgfx>(texData->GetTexID());
				bgfx::destroy(tex.handle);
				texData->SetTexID(ImTextureID_Invalid);
				texData->SetStatus(ImTextureStatus_Destroyed);
			}
		}

		// ImGui::ShutdownDockContext();
		ImGui::DestroyContext(m_imgui);

		bgfx::destroy(s_tex);

		bgfx::destroy(u_imageLodEnabled);
		bgfx::destroy(m_imageProgram);
		bgfx::destroy(m_program);

		m_allocator = NULL;
	}

	void setupStyle(bool _dark)
	{
		// Doug Binks' darl color scheme
		// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
		ImGuiStyle& style = ImGui::GetStyle();
		if (_dark)
		{
			ImGui::StyleColorsDark(&style);
		}
		else
		{
			ImGui::StyleColorsLight(&style);
		}

		style.FrameRounding = 4.0f;
	}

	void beginFrame()
	{
		m_viewId = 1;

		ImGui::NewFrame();

		// ImGuizmo::BeginFrame();
	}

	void endFrame()
	{
		ImGui::Render();
		render(ImGui::GetDrawData());
	}

	ImGuiContext* m_imgui{nullptr};
	bx::AllocatorI* m_allocator{nullptr};
	bgfx::VertexLayout m_layout;
	bgfx::ProgramHandle m_program;
	bgfx::ProgramHandle m_imageProgram;
	bgfx::UniformHandle s_tex;
	bgfx::UniformHandle u_imageLodEnabled;
	// ImFont* m_font[ImGui::Font::Count];
	int64_t m_last{};
	int32_t m_lastScroll{};
	bgfx::ViewId m_viewId;
};

static OcornutImguiContext s_ctx;

static auto memAlloc(size_t _size, void* _userData) -> void*
{
	BX_UNUSED(_userData);
	return bx::alloc(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BX_UNUSED(_userData);
	bx::free(s_ctx.m_allocator, _ptr);
}

void ImGui_ImplBGFX_Init(float _fontSize, bx::AllocatorI* _allocator)
{
	s_ctx.create(_fontSize, _allocator);
}

void ImGui_ImplBGFX_Shutdown() { s_ctx.destroy(); }

void ImGui_ImplBGFX_NewFrame() { s_ctx.beginFrame(); }

void ImGui_ImplBGFX_EndFrame() { s_ctx.endFrame(); }

namespace ImGui
{
// void PushFont(Font::Enum _font, float _fontSizeBaseUnscaled)
// {
// 	PushFont(s_ctx.m_font[_font], _fontSizeBaseUnscaled);
// }

} // namespace ImGui
