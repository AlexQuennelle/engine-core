/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

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

static const bgfx::EmbeddedShader s_embeddedShaders[]
	= {{"vs_ocornut_imgui",
		{// {bgfx ::RendererType ::Direct3D11, vs_ocornut_imgui_dx11,
		 // (sizeof(vs_ocornut_imgui_dx11) / sizeof(vs_ocornut_imgui_dx11[0]))},
		 {bgfx ::RendererType ::OpenGLES, vs_ocornut_imgui_essl,
		  (sizeof(vs_ocornut_imgui_essl) / sizeof(vs_ocornut_imgui_essl[0]))},
		 {bgfx ::RendererType ::OpenGL, vs_ocornut_imgui_glsl,
		  (sizeof(vs_ocornut_imgui_glsl) / sizeof(vs_ocornut_imgui_glsl[0]))},
		 {bgfx ::RendererType ::Vulkan, vs_ocornut_imgui_spv,
		  (sizeof(vs_ocornut_imgui_spv) / sizeof(vs_ocornut_imgui_spv[0]))},
		 {bgfx ::RendererType ::WebGPU, vs_ocornut_imgui_wgsl,
		  (sizeof(vs_ocornut_imgui_wgsl) / sizeof(vs_ocornut_imgui_wgsl[0]))},
		 {bgfx ::RendererType ::Noop,
		  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10},
		 {bgfx ::RendererType ::Count, __null, 0}}},
	   {"fs_ocornut_imgui",
		{// {bgfx ::RendererType ::Direct3D11, fs_ocornut_imgui_dx11,
		 // (sizeof(fs_ocornut_imgui_dx11) / sizeof(fs_ocornut_imgui_dx11[0]))},
		 {bgfx ::RendererType ::OpenGLES, fs_ocornut_imgui_essl,
		  (sizeof(fs_ocornut_imgui_essl) / sizeof(fs_ocornut_imgui_essl[0]))},
		 {bgfx ::RendererType ::OpenGL, fs_ocornut_imgui_glsl,
		  (sizeof(fs_ocornut_imgui_glsl) / sizeof(fs_ocornut_imgui_glsl[0]))},
		 {bgfx ::RendererType ::Vulkan, fs_ocornut_imgui_spv,
		  (sizeof(fs_ocornut_imgui_spv) / sizeof(fs_ocornut_imgui_spv[0]))},
		 {bgfx ::RendererType ::WebGPU, fs_ocornut_imgui_wgsl,
		  (sizeof(fs_ocornut_imgui_wgsl) / sizeof(fs_ocornut_imgui_wgsl[0]))},
		 {bgfx ::RendererType ::Noop,
		  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10},
		 {bgfx ::RendererType ::Count, __null, 0}}},
	   {"vs_imgui_image",
		{// {bgfx ::RendererType ::Direct3D11, vs_imgui_image_dx11,
		 //  (sizeof(vs_imgui_image_dx11) / sizeof(vs_imgui_image_dx11[0]))},
		 {bgfx ::RendererType ::OpenGLES, vs_imgui_image_essl,
		  (sizeof(vs_imgui_image_essl) / sizeof(vs_imgui_image_essl[0]))},
		 {bgfx ::RendererType ::OpenGL, vs_imgui_image_glsl,
		  (sizeof(vs_imgui_image_glsl) / sizeof(vs_imgui_image_glsl[0]))},
		 {bgfx ::RendererType ::Vulkan, vs_imgui_image_spv,
		  (sizeof(vs_imgui_image_spv) / sizeof(vs_imgui_image_spv[0]))},
		 {bgfx ::RendererType ::WebGPU, vs_imgui_image_wgsl,
		  (sizeof(vs_imgui_image_wgsl) / sizeof(vs_imgui_image_wgsl[0]))},
		 {bgfx ::RendererType ::Noop,
		  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10},
		 {bgfx ::RendererType ::Count, __null, 0}}},
	   {"fs_imgui_image",
		{// {bgfx ::RendererType ::Direct3D11, fs_imgui_image_dx11,
		 //  (sizeof(fs_imgui_image_dx11) / sizeof(fs_imgui_image_dx11[0]))},
		 {bgfx ::RendererType ::OpenGLES, fs_imgui_image_essl,
		  (sizeof(fs_imgui_image_essl) / sizeof(fs_imgui_image_essl[0]))},
		 {bgfx ::RendererType ::OpenGL, fs_imgui_image_glsl,
		  (sizeof(fs_imgui_image_glsl) / sizeof(fs_imgui_image_glsl[0]))},
		 {bgfx ::RendererType ::Vulkan, fs_imgui_image_spv,
		  (sizeof(fs_imgui_image_spv) / sizeof(fs_imgui_image_spv[0]))},
		 {bgfx ::RendererType ::WebGPU, fs_imgui_image_wgsl,
		  (sizeof(fs_imgui_image_wgsl) / sizeof(fs_imgui_image_wgsl[0]))},
		 {bgfx ::RendererType ::Noop,
		  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10},
		 {bgfx ::RendererType ::Count, __null, 0}}},

	   BGFX_EMBEDDED_SHADER_END()};

struct FontRangeMerge
{
	const void* data;
	size_t size;
	ImWchar ranges[3];
};

// static FontRangeMerge s_fontRangeMerge[] = {
// 	{s_iconsKenneyTtf, sizeof(s_iconsKenneyTtf), {ICON_MIN_KI, ICON_MAX_KI, 0}},
// 	{s_iconsFontAwesomeTtf,
// 	 sizeof(s_iconsFontAwesomeTtf),
// 	 {ICON_MIN_FA, ICON_MAX_FA, 0}},
// };

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
		int32_t dispWidth
			= int32_t(_drawData->DisplaySize.x * _drawData->FramebufferScale.x);
		int32_t dispHeight
			= int32_t(_drawData->DisplaySize.y * _drawData->FramebufferScale.y);
		if (dispWidth <= 0 || dispHeight <= 0)
		{
			return;
		}

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

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
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

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

		io.DisplaySize = ImVec2(1280.0f, 720.0f);
		io.DeltaTime = 1.0f / 60.0f;
		io.IniFilename = NULL;

		setupStyle(true);

		io.BackendFlags |= 0
						   | ImGuiBackendFlags_RendererHasVtxOffset
						   | ImGuiBackendFlags_RendererHasTextures;
		io.ConfigDebugHighlightIdConflicts = !!BX_CONFIG_DEBUG;

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		m_program = bgfx::createProgram(
			bgfx::createEmbeddedShader(s_embeddedShaders, type,
									   "vs_ocornut_imgui"),
			bgfx::createEmbeddedShader(s_embeddedShaders, type,
									   "fs_ocornut_imgui"),
			true);

		u_imageLodEnabled
			= bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		m_imageProgram = bgfx::createProgram(
			bgfx::createEmbeddedShader(s_embeddedShaders, type,
									   "vs_imgui_image"),
			bgfx::createEmbeddedShader(s_embeddedShaders, type,
									   "fs_imgui_image"),
			true);

		m_layout.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		// {
		// 	ImFontConfig config;
		// 	config.FontDataOwnedByAtlas = false;
		// 	config.MergeMode = false;

		// 	const ImWchar* ranges = io.Fonts->GetGlyphRangesDefault();
		// 	m_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF(
		// 		(void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf),
		// 		_fontSize, &config, ranges);
		// 	m_font[ImGui::Font::Mono] = io.Fonts->AddFontFromMemoryTTF(
		// 		(void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf),
		// 		_fontSize - 3.0f, &config, ranges);

		// 	config.MergeMode = true;
		// 	config.DstFont = m_font[ImGui::Font::Regular];

		// 	// for (uint32_t ii = 0; ii < BX_COUNTOF(s_fontRangeMerge); ++ii)
		// 	// {
		// 	// 	const FontRangeMerge& frm = s_fontRangeMerge[ii];

		// 	// 	io.Fonts->AddFontFromMemoryTTF((void*)frm.data, (int)frm.size,
		// 	// 								   _fontSize - 3.0f, &config,
		// 	// 								   frm.ranges);
		// 	// }
		// }

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
				ImGui::TextureBgfx tex
					= bx::bitCast<ImGui::TextureBgfx>(texData->GetTexID());
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
		style.WindowBorderSize = 0.0f;
	}

	void beginFrame(int32_t _mx, int32_t _my, uint8_t _button, int32_t _scroll,
					int _width, int _height, int _inputChar,
					bgfx::ViewId _viewId)
	{
		m_viewId = _viewId;

		ImGuiIO& io = ImGui::GetIO();
		if (_inputChar >= 0)
		{
			io.AddInputCharacter(_inputChar);
		}

		io.DisplaySize = ImVec2((float)_width, (float)_height);

		const int64_t now = bx::getHPCounter();
		const int64_t frameTime = now - m_last;
		m_last = now;
		const double freq = double(bx::getHPFrequency());
		io.DeltaTime = float(frameTime / freq);

		// io.AddMousePosEvent((float)_mx, (float)_my);
		// io.AddMouseButtonEvent(ImGuiMouseButton_Left,
		// 					   0 != (_button & IMGUI_MBUT_LEFT));
		// io.AddMouseButtonEvent(ImGuiMouseButton_Right,
		// 					   0 != (_button & IMGUI_MBUT_RIGHT));
		// io.AddMouseButtonEvent(ImGuiMouseButton_Middle,
		// 					   0 != (_button & IMGUI_MBUT_MIDDLE));
		// io.AddMouseWheelEvent(0.0f, (float)(_scroll - m_lastScroll));
		m_lastScroll = _scroll;

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

static void* memAlloc(size_t _size, void* _userData)
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

void ImGui_ImplBGFX_NewFrame(int32_t _mx, int32_t _my, uint8_t _button,
							 int32_t _scroll, uint16_t _width, uint16_t _height,
							 int _inputChar, bgfx::ViewId _viewId)
{
	s_ctx.beginFrame(_mx, _my, _button, _scroll, _width, _height, _inputChar,
					 _viewId);
}

void ImGui_ImplBGFX_EndFrame() { s_ctx.endFrame(); }

namespace ImGui
{
// void PushFont(Font::Enum _font, float _fontSizeBaseUnscaled)
// {
// 	PushFont(s_ctx.m_font[_font], _fontSizeBaseUnscaled);
// }

} // namespace ImGui
