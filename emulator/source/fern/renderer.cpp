#include <fern.h>
#include <fern_common.h>
#include <SDL2/SDL.h>

namespace fern {
	// color --------------------------------------------@/
	const std::array<fern::CColor,4> CRenderer::MONOPALET_GRAY = {
		fern::CColor(255,255,255),
		fern::CColor(192,192,192),
		fern::CColor(112,112,112),
		fern::CColor(12,12,12)
	};
	const std::array<fern::CColor,4> CRenderer::MONOPALET_ORANGE = {
		fern::CColor(0xff,0xf6,0xd3),
		fern::CColor(0xf9,0xa8,0x75),
		fern::CColor(0xeb,0x6b,0x6f),
		fern::CColor(0x7c,0x3f,0x58)
	};
	auto CColor::from_rgb15(int clrdat) -> CColor {
		int r = (clrdat) & 31;
		int g = (clrdat >> 5) & 31;
		int b = (clrdat >> 10) & 31;
		return CColor(r<<3,g<<3,b<<3);
	}

	// renderer -----------------------------------------@/
	CRenderer::CRenderer() :
	 m_screen(fern::SCREEN_X,fern::SCREEN_Y),
     m_screenVRAM(fern::SCREENVRAM_X,fern::SCREENVRAM_Y),
	 m_screenPalet(fern::SCREENPAL_X,fern::SCREENPAL_Y) {
		m_timeLastFrame = 0;
		m_window = nullptr;
		m_windowVRAM = nullptr;
		m_windowPalet = nullptr;
		m_vsyncEnabled = false;
	}
	CRenderer::~CRenderer() {
	}

	auto CRenderer::window_close() -> void {
		// TODO: should renderer be deleted, too?
		// apparently it should in older SDL2 versions...
		// t. https://github.com/libsdl-org/SDL/issues/9540
		auto del_window = [&](SDL_Window* winhandle) {
			if(winhandle) SDL_DestroyWindow(winhandle);
		};
		del_window(m_window);
		del_window(m_windowPalet);
		del_window(m_windowVRAM);

		m_window = nullptr;
		m_windowPalet = nullptr;
		m_windowVRAM = nullptr;
	}
	auto CRenderer::window_create(bool vsync) -> void {
		m_vsyncEnabled = vsync;
		m_window = SDL_CreateWindow("fern",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			fern::SCREEN_X,fern::SCREEN_Y,
			0
		);
		
		// create renderer based on vsync enable
		if(!vsync_enabled()) {
			m_renderer = SDL_CreateRenderer(
				m_window,-1,
				SDL_RENDERER_SOFTWARE
			);
		} else {
			m_renderer = SDL_CreateRenderer(
				m_window,-1,
				SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC
			);
		}

		// create other windows for debug
		// VRAM window's essentially two 8x24 screens, side by side
		std::array<int,2> winpos_main;
		SDL_GetWindowPosition(m_window,&winpos_main[0],&winpos_main[1]);

		m_windowVRAM = SDL_CreateWindow("VRAM View",
			winpos_main[0] + fern::SCREEN_X + 32,
			winpos_main[1],
			fern::SCREENVRAM_X,
			fern::SCREENVRAM_Y,
			0
		);
		m_windowPalet = SDL_CreateWindow("CRAM View",
			winpos_main[0] - fern::SCREENPAL_X - 32,
			winpos_main[1],
			fern::SCREENPAL_X,
			fern::SCREENPAL_Y,
			0
		);
	}

	auto CRenderer::render_vramwindow() -> void {
		auto& mem = emu()->mem;
		m_screenVRAM.clear(fern::CColor(255,0,255));

		int num_banks = emu()->cgb_enabled() ? 2 : 1;
		auto& dmg_palet = fern::CRenderer::MONOPALET_GRAY;

		auto palet_getTrue = [&](const auto& src_mem) {
			std::array<fern::CColor,32> palet;
			for(int i=0; i<32; i++) {
				int data = src_mem[i*2 + 0] | (src_mem[i*2 + 1] << 8);
				palet[i] = fern::CColor::from_rgb15(data);
			}
			return palet;
		};
		const auto palet_BG = palet_getTrue(mem.m_paletBG);
		const auto palet_obj = palet_getTrue(mem.m_paletObj);

		for(int bank=0; bank<num_banks; bank++)
		for(int i=0; i<0x180; i++) {
			// get tile address
			int addr = (KBSIZE(8)*bank) + i * 0x10;
			int screenX = (i & 0xF) * 8 + (bank * 128);
			int screenY = (i / 16) * 8;

			const fern::CColor* palet_line = dmg_palet.data();
			if(m_vramMarker.at(addr/0x10) >= 0) {
				int pal_code = m_vramMarker.at(addr/0x10);
				if(pal_code < 8) {
					palet_line = &palet_BG[(pal_code & 7) * 4];
				} else {
					palet_line = &palet_obj[(pal_code & 7) * 4];
				}
			}

			// read lines
			for(int y=0; y<8; y++) {
				int lineA = mem.m_vram[addr + 0 + y*2];
				int lineB = mem.m_vram[addr + 1 + y*2];
				lineB <<= 1;
				// lineA: -Fbbbbbbb
				// lineB: Fbbbbbbb0
				for(int x=0; x<8; x++) {
					int dotA = (lineA >> 7) & 0b1;
					int dotB = (lineB >> 7) & 0b10;
					int dot = dotA | dotB;
					auto color = palet_line[dot];
					m_screenVRAM.dot_access(screenX+x,screenY+y) = color;
					lineA <<= 1;
					lineB <<= 1;
				}
			}
		}
	}
	auto CRenderer::render_palwindow() -> void {
		auto& mem = emu()->mem;
		m_screenPalet.clear(fern::CColor(255,0,255));

		if(emu()->cgb_enabled()) {
			auto palet_getTrue = [&](const auto& src_mem) {
				std::array<fern::CColor,32> palet;
				for(int i=0; i<32; i++) {
					int data = src_mem[i*2 + 0] | (src_mem[i*2 + 1] << 8);
					palet[i] = fern::CColor::from_rgb15(data);
				}
				return palet;
			};
			const auto palet_BG = palet_getTrue(mem.m_paletBG);
			const auto palet_obj = palet_getTrue(mem.m_paletObj);

			// each color's 4 dots wide
			const int square_sizeX = 16;
			const int square_sizeY = 8;
			auto palet_render = [&](auto palet, int x_offset) {
				for(int i=0; i<palet.size(); i++) {
					const int draw_x = (i&3) * square_sizeX + x_offset;
					const int draw_y = (i/4) * square_sizeY;

					auto color = palet.at(i);
					for(int y=0; y<square_sizeY; y++) {
						for(int x=0; x<square_sizeX; x++) {
							m_screenPalet.dot_access(draw_x+x,draw_y+y) = color;
						}
					}
				}
			};
			palet_render(palet_BG,0);
			palet_render(palet_obj,m_screenPalet.width()/2);
		} else {
			const auto& dmg_palet = fern::CRenderer::MONOPALET_ORANGE;
			std::array<std::array<int,4>,2> obp_table;
			std::array<int,4> bgp_table;

			// create bgp lookup table ------------------@/
			int bgp_shifter = mem.m_io.m_BGP;
			std::array<int,2> obp_shifter = { mem.m_io.m_OBP[0], mem.m_io.m_OBP[1] };
			for(int i=0; i<4; i++) {
				obp_table[0][i] = obp_shifter.at(0) & 3;
				obp_table[1][i] = obp_shifter.at(1) & 3;
				bgp_table.at(i) = bgp_shifter & 3;
				obp_shifter.at(0) >>= 2;
				obp_shifter.at(1) >>= 2;
				bgp_shifter >>= 2;
			}
			
			// draw bg palette --------------------------@/
			for(int i=0; i<4; i++) {
				static const int square_sizeX = 16;
				static const int square_sizeY = 64;
				auto color = dmg_palet.at(bgp_table[i]);
				for(int y=0; y<square_sizeY; y++) {
					for(int x=0; x<square_sizeX; x++) {
						m_screenPalet.dot_access((i*square_sizeX)+x,y) = color;
					}
				}
			}

			// draw obj palette -------------------------@/
			for(int p=0; p<2; p++) {
				for(int i=0; i<4; i++) {
					static const int square_sizeX = 16;
					static const int square_sizeY = 32;
					const int draw_x = (i*square_sizeX) + 64;
					const int draw_y = (p*square_sizeY);
					auto color = dmg_palet.at(obp_table[p][i]);
					for(int y=0; y<square_sizeY; y++) {
						for(int x=0; x<square_sizeX; x++) {
							m_screenPalet.dot_access(draw_x+x,draw_y+y) = color;
						}
					}
				}
			}
		}
	}

	auto CRenderer::present() -> void {
		render_vramwindow();
		render_palwindow();
		if(auto surface = SDL_GetWindowSurface(m_window)) {
			m_screen.render_toSurface(surface);
		}
		if(auto surface = SDL_GetWindowSurface(m_windowVRAM)) {
			m_screenVRAM.render_toSurface(surface);
		}
		if(auto surface = SDL_GetWindowSurface(m_windowPalet)) {
			m_screenPalet.render_toSurface(surface);
		}

		if(!emu()->nowait_isEnabled()) {
			// wait til next frame
			if(!vsync_enabled()) {
				// busy wait for next frame...
				while(SDL_GetTicks() - m_timeLastFrame < (1000 / 60)) {
					SDL_Delay(1);
				}
			} else {
				SDL_RenderClear(m_renderer);
				SDL_RenderPresent(m_renderer);
			}
		}
		m_timeLastFrame = SDL_GetTicks();
		m_vramMarker.fill(-1);

		SDL_UpdateWindowSurface(m_window);
		SDL_UpdateWindowSurface(m_windowVRAM);
		SDL_UpdateWindowSurface(m_windowPalet);
	}
	auto CRenderer::draw_line(int draw_y) -> void {
		if(emu()->cgb_enabled()) {
			draw_lineCGB(draw_y);
		} else {
			draw_lineDMG(draw_y);
		}
	}

	auto CRenderer::draw_lineCGB(int draw_y) -> void {
		if(draw_y < 0 || draw_y >= 144) return;

		auto& mem = emu()->mem;
		const int lcdc = mem.m_io.m_LCDC;
		std::array<char,fern::SCREEN_X> bg_linebuffer;
		//std::array<char,fern::SCREEN_X> obj_linebuffer;

		auto palet_getTrue = [&](const auto& src_mem) {
			std::array<fern::CColor,32> palet;
			for(int i=0; i<32; i++) {
				int data = src_mem[i*2 + 0] | (src_mem[i*2 + 1] << 8);
				palet[i] = fern::CColor::from_rgb15(data);
			}
			return palet;
		};
		const auto palet_BG = palet_getTrue(mem.m_paletBG);
		const auto palet_obj = palet_getTrue(mem.m_paletObj);

		// draw backplane, if lcd's off -----------------@/
		if(!(lcdc & RFlagLCDC::lcdon)) {
			const auto color = fern::CColor(0,255,0);
			for(int i=0; i<fern::SCREEN_X; i++) {
				m_screen.dot_access(i,draw_y) = color;
			}
			return;
		}

		// draw background ------------------------------@/
		{
			const int bgscroll_x = mem.m_io.m_SCX;
			const int bgscroll_y = mem.m_io.m_SCY;
			
			const int fetch_y = (bgscroll_y + draw_y) & 0xFF;
			size_t addr_chrbase = 0x0800;
			size_t addr_mapbase = 0x1800;
			if(lcdc & RFlagLCDC::chr8000) addr_chrbase -= 0x0800;
			if(lcdc & RFlagLCDC::map9C00) addr_mapbase += 0x0400;
			const size_t addr_mapline = addr_mapbase + ((fetch_y/8) * 0x20);

			for(int draw_x=0; draw_x<fern::SCREEN_X; draw_x++) {
				fern::CColor color;
				int dot = 0;
				if(lcdc & RFlagLCDC::bgon) {
					int fetch_x = (bgscroll_x + draw_x) & 0xFF;
					// fetch tile
					const int mapaddr = addr_mapline + (fetch_x/8);
					const int mapaddr_attrib = mapaddr + KBSIZE(8);
					const int attrib = mem.m_vram.at(mapaddr_attrib);
					int attrib_paletnum = attrib & 7;
					int attrib_banknum = RFlagMapAttrib::bank(attrib);

					int tile = 0;
					if(lcdc & RFlagLCDC::chr8000) {
						tile = mem.m_vram.at(mapaddr);
					} else {
						tile = static_cast<int8_t>(mem.m_vram.at(mapaddr));
						tile = (tile + 0x80) & 0xFF;
					}
					int tileaddr = (KBSIZE(8) * attrib_banknum) + addr_chrbase + tile * 0x10;
					m_vramMarker.at(tileaddr / 0x10) = attrib_paletnum;
					
					// get pixel
					int tileY = (fetch_y&7);
					int tileX = (fetch_x&7);
					if(RFlagMapAttrib::flipX(attrib)) tileX = 7-tileX;
					if(RFlagMapAttrib::flipY(attrib)) tileY = 7-tileY;
					tileaddr += tileY*2;
					int lineA = mem.m_vram[tileaddr];
					int lineB = mem.m_vram[tileaddr+1];
					int dotA = (lineA >> (7-tileX)) & 1;
					int dotB = (lineB >> (7-tileX)) & 1;
					dot = dotA | (dotB<<1);
					color = palet_BG[attrib_paletnum*4 + dot];
				} else {
					color = fern::CRenderer::MONOPALET_ORANGE[0];
				}
				m_screen.dot_set(draw_x,draw_y,color);
				bg_linebuffer[draw_x] = dot;
			}
		}
	
		// draw sprites ---------------------------------@/
		const bool spr_size2x = (mem.m_io.m_LCDC & 0x04) != 0;
		const int spr_height = spr_size2x ? 16 : 8;
		
		if(lcdc & RFlagLCDC::objon) {
			for(int spr_idx=0; spr_idx<40; spr_idx++) {
				const std::array<int,4> oamdata = { 
					mem.m_oam[spr_idx*4 + 0],
					mem.m_oam[spr_idx*4 + 1],
					mem.m_oam[spr_idx*4 + 2],
					mem.m_oam[spr_idx*4 + 3]
				};

				const int oamdat_y = oamdata[0] - 16;
				const int oamdat_x = oamdata[1] - 8;
				const bool oamdat_prio = (oamdata[3] >> 7) & 1;
				const bool flipX = (oamdata[3]>>5) & 1;
				const bool flipY = (oamdata[3]>>6) & 1;
				const int attrib_palet = oamdata[3] & 7;
				const int attrib_bank = (oamdata[3]>>3) & 1;

				if(oamdat_y <= -16 || oamdat_y >= fern::SCREEN_Y) continue;
				if(oamdat_x <= -8 || oamdat_x >= fern::SCREEN_X) continue;
				if(draw_y < oamdat_y) continue;
				if((draw_y - oamdat_y) >= spr_height) continue;

				// get relative line of the sprite to draw
				int line_y = (draw_y - oamdat_y);
				if(flipY) line_y = (spr_height-1) - line_y;

				// get tile address
				int tileaddr = (attrib_bank * KBSIZE(8)) + (oamdata[2] * 0x10);
				m_vramMarker.at(tileaddr / 0x10) = attrib_palet + 8;
				tileaddr += line_y * 2;
				if(spr_size2x) tileaddr &= 0xFFFE;
				int lineA = mem.m_vram[tileaddr];
				int lineB = mem.m_vram[tileaddr+1];

				for(int ix=0; ix<8; ix++) {
					int x = ix;
					if(oamdat_x+x < 0) continue;
					if(oamdat_x+x >= fern::SCREEN_X) break;

					int dotA = (lineA >> (7-x)) & 1;
					int dotB = (lineB >> (7-x)) & 1;
					int dot = dotA | (dotB<<1);
					if(dot == 0) continue;
					if(flipX) x = (7-x);
					if(oamdat_prio && (bg_linebuffer[oamdat_x+x] > 0)) continue;
					m_screen.dot_access(oamdat_x+x,draw_y) = 
						palet_obj[attrib_palet*4 + dot];
				}
			}
		}

		// draw window layer ----------------------------@/
		if(lcdc & RFlagLCDC::winon) {
			const int bgscroll_x = mem.m_io.m_WX + 7;
			const int bgscroll_y = mem.m_io.m_WY;
			
			if(bgscroll_y <= draw_y && bgscroll_x < fern::SCREEN_X) {
				const int fetch_y = (draw_y - bgscroll_y) & 0xFF;
				size_t addr_chrbase = 0x0800;
				size_t addr_mapbase = 0x1800;
				if(lcdc & RFlagLCDC::chr8000) addr_chrbase -= 0x0800;
				if(lcdc & RFlagLCDC::win9C00) addr_mapbase += 0x0400;
				const size_t addr_mapline = addr_mapbase + ((fetch_y/8) * 0x20);

				for(int draw_x=0; draw_x<256; draw_x++) {
					if(draw_x+bgscroll_x-14 >= fern::SCREEN_X) break;
					if(draw_x+bgscroll_x-14 < 0) break;
					int dot = 0;
					// fetch tile
					const int mapaddr = addr_mapline + (draw_x/8);
					const int mapaddr_attrib = mapaddr + KBSIZE(8);
					const int attrib = mem.m_vram.at(mapaddr_attrib);
					int attrib_paletnum = attrib & 7;
					int attrib_banknum = RFlagMapAttrib::bank(attrib);

					int tile = 0;
					if(lcdc & RFlagLCDC::chr8000) {
						tile = mem.m_vram.at(mapaddr);
					} else {
						tile = static_cast<int8_t>(mem.m_vram.at(mapaddr));
						tile = (tile + 0x80) & 0xFF;
					}
					int tileaddr = (KBSIZE(8) * attrib_banknum) + addr_chrbase + tile * 0x10;
					m_vramMarker.at(tileaddr / 0x10) = attrib_paletnum;
					
					// get pixel
					int tileY = (fetch_y&7);
					int tileX = (draw_x&7);
					if(RFlagMapAttrib::flipX(attrib)) tileX = 7-tileX;
					if(RFlagMapAttrib::flipY(attrib)) tileY = 7-tileY;
					tileaddr += tileY*2;
					int lineA = mem.m_vram[tileaddr];
					int lineB = mem.m_vram[tileaddr+1];
					int dotA = (lineA >> (7-tileX)) & 1;
					int dotB = (lineB >> (7-tileX)) & 1;
					dot = dotA | (dotB<<1);

					m_screen.dot_set(draw_x+bgscroll_x-14,draw_y,
					//	palet_gray[bgp_table[dot]]
						palet_BG[attrib_paletnum*4 + dot]
					);
				}
			}
		}
	}
	auto CRenderer::draw_lineDMG(int draw_y) -> void {
		if(draw_y < 0 || draw_y >= 144) return;

		auto& mem = emu()->mem;
		const auto& dmg_palet = fern::CRenderer::MONOPALET_ORANGE;
		const int lcdc = mem.m_io.m_LCDC;
		std::array<char,fern::SCREEN_X> bg_linebuffer;
		std::array<char,fern::SCREEN_X> obj_linebuffer;
		std::array<std::array<int,4>,2> obp_table;
		std::array<int,4> bgp_table;

		// create bgp lookup table
		int bgp_shifter = mem.m_io.m_BGP;
		std::array<int,2> obp_shifter = { mem.m_io.m_OBP[0], mem.m_io.m_OBP[1] };
		for(int i=0; i<4; i++) {
			obp_table[0][i] = obp_shifter.at(0) & 3;
			obp_table[1][i] = obp_shifter.at(1) & 3;
			bgp_table.at(i) = bgp_shifter & 3;
			obp_shifter.at(0) >>= 2;
			obp_shifter.at(1) >>= 2;
			bgp_shifter >>= 2;
		}

		if(!(lcdc & RFlagLCDC::lcdon)) {
			const auto color = dmg_palet[0];
			for(int i=0; i<fern::SCREEN_X; i++) {
				m_screen.dot_access(i,draw_y) = color;
			}
			return;
		}

		// draw background ------------------------------@/
		{
			const int bgscroll_x = mem.m_io.m_SCX;
			const int bgscroll_y = mem.m_io.m_SCY;
			
			const int fetch_y = (bgscroll_y + draw_y) & 0xFF;
			size_t addr_chrbase = 0x0800;
			size_t addr_mapbase = 0x1800;
			if(lcdc & RFlagLCDC::chr8000) addr_chrbase -= 0x0800;
			if(lcdc & RFlagLCDC::map9C00) addr_mapbase += 0x0400;
			const size_t addr_mapline = addr_mapbase + ((fetch_y/8) * 0x20);

			for(int draw_x=0; draw_x<fern::SCREEN_X; draw_x++) {
				int dot = 0;
				if(lcdc & RFlagLCDC::bgon) {
					int fetch_x = (bgscroll_x + draw_x) & 0xFF;
					// fetch tile
					int tile = 0;
					if(lcdc & RFlagLCDC::chr8000) {
						tile = mem.m_vram.at(addr_mapline + (fetch_x/8));
					} else {
						tile = static_cast<int8_t>(mem.m_vram.at(addr_mapline + (fetch_x/8)));
						tile = (tile + 0x80) & 0xFF;
					}
					int tileaddr = addr_chrbase + tile * 0x10;
					// get pixel
					tileaddr += (fetch_y&7)*2;
					int lineA = mem.m_vram[tileaddr];
					int lineB = mem.m_vram[tileaddr+1];
					int dotA = (lineA >> (7-(fetch_x&7))) & 1;
					int dotB = (lineB >> (7-(fetch_x&7))) & 1;
					dot = dotA | (dotB<<1);
				} 
			//	m_screen.dot_set(draw_x,draw_y,palet_gray[dot]);
				m_screen.dot_set(draw_x,draw_y,dmg_palet[bgp_table[dot]]);
				bg_linebuffer[draw_x] = dot;
			}
		}

		// draw sprites ---------------------------------@/
		const bool spr_size2x = (mem.m_io.m_LCDC & 0x04) != 0;
		const int spr_height = spr_size2x ? 16 : 8;
		
		for(int spr_idx=0; spr_idx<40; spr_idx++) {
			const std::array<int,4> oamdata = { 
				mem.m_oam[spr_idx*4 + 0],
				mem.m_oam[spr_idx*4 + 1],
				mem.m_oam[spr_idx*4 + 2],
				mem.m_oam[spr_idx*4 + 3]
			};

			const int oamdat_y = oamdata[0] - 16;
			const int oamdat_x = oamdata[1] - 8;
			const int oamdat_palet = (oamdata[3] >> 4) & 1;
			const bool oamdat_prio = (oamdata[3] >> 7) & 1;
			const bool flipX = (oamdata[3]>>5) & 1;
			const bool flipY = (oamdata[3]>>6) & 1;

			auto& cur_paltable = obp_table.at(oamdat_palet);
			if(oamdat_y <= -16 || oamdat_y >= 144) continue;
			if(oamdat_x <= -8 || oamdat_x >= fern::SCREEN_X) continue;
			if(draw_y < oamdat_y) continue;
			if((draw_y - oamdat_y) >= spr_height) continue;

			// get relative line of the sprite to draw
			int line_y = (draw_y - oamdat_y);
			if(flipY) line_y = (spr_height-1) - line_y;

			// get tile address
			int tileaddr = 0x0000 + (oamdata[2] * 0x10);
			tileaddr += line_y * 2;
			if(spr_size2x) tileaddr &= 0xFFFE;
			int lineA = mem.m_vram[tileaddr];
			int lineB = mem.m_vram[tileaddr+1];

			for(int ix=0; ix<8; ix++) {
				int x = ix;
				if(oamdat_x+x < 0) continue;
				if(oamdat_x+x >= fern::SCREEN_X) break;

				int dotA = (lineA >> (7-x)) & 1;
				int dotB = (lineB >> (7-x)) & 1;
				int dot = dotA | (dotB<<1);
				if(dot == 0) continue;
				if(flipX) x = (7-x);
				if(oamdat_prio && (bg_linebuffer[oamdat_x+x] > 0)) continue;
				obj_linebuffer[oamdat_x+x] = dot;
				m_screen.dot_access(oamdat_x+x,draw_y) = 
					dmg_palet.at(cur_paltable.at(dot));
			}
		}

		// draw window layer ----------------------------@/
		if(lcdc & RFlagLCDC::winon) {
			const int bgscroll_x = mem.m_io.m_WX + 7;
			const int bgscroll_y = mem.m_io.m_WY;
			
			if(bgscroll_y <= draw_y && bgscroll_x < fern::SCREEN_X) {
				const int fetch_y = (draw_y - bgscroll_y) & 0xFF;
				size_t addr_chrbase = 0x0800;
				size_t addr_mapbase = 0x1800;
				if(lcdc & RFlagLCDC::chr8000) addr_chrbase -= 0x0800;
				if(lcdc & RFlagLCDC::win9C00) addr_mapbase += 0x0400;
				const size_t addr_mapline = addr_mapbase + ((fetch_y/8) * 0x20);

				for(int draw_x=0; draw_x<256; draw_x++) {
					if(draw_x+bgscroll_x-14 >= fern::SCREEN_X) break;
					if(draw_x+bgscroll_x-14 < 0) break;
					int dot = 0;
					// fetch tile
					int tile = 0;
					if(lcdc & RFlagLCDC::chr8000) {
						tile = mem.m_vram.at(addr_mapline + (draw_x/8));
					} else {
						tile = static_cast<int8_t>(mem.m_vram.at(addr_mapline + (draw_x/8)));
						tile = (tile + 0x80) & 0xFF;
					}
					int tileaddr = addr_chrbase + tile * 0x10;				

					// get pixel
					tileaddr += (fetch_y&7)*2;
					int lineA = mem.m_vram.at(tileaddr);
					int lineB = mem.m_vram.at(tileaddr+1);
					int dotA = (lineA >> (7-(draw_x&7))) & 1;
					int dotB = (lineB >> (7-(draw_x&7))) & 1;
					dot = dotA | (dotB<<1);
					m_screen.dot_set(draw_x+bgscroll_x-14,draw_y,dmg_palet[bgp_table[dot]]);
				}
			}
		}
	}

	// screen -------------------------------------------@/
	CScreen::CScreen(int new_width, int new_height) {
		m_width = new_width;
		m_height = new_height;

		if(dimensions() <= 0) {
			std::printf("CScreen::CScreen(): error: invalid dimensions (%d,%d)\n",
				new_width,new_height
			);
			std::exit(-1);
		}

		m_bmp.resize(dimensions());
		clear(fern::CColor(0,0,0));
	}

	auto CScreen::in_range(int x, int y) -> bool {
		if(x < 0 || x >= width()) return false;
		if(y < 0 || y >= height()) return false;
		return true;
	}
	auto CScreen::dot_set(int x, int y, CColor color) -> void {
		if(!in_range(x,y)) {
			std::printf("CScreen::dot_set(): error: invalid coords (%d,%d)\n",
				x,y
			);
			std::exit(-1);
		}
		dot_access(x,y) = color;
	}
	auto CScreen::clear(fern::CColor color) -> void {
		for(int i=0; i<dimensions(); i++) {
			m_bmp[i] = color;
		}
	}

	auto CScreen::render_toSurface(SDL_Surface* surface) -> void {
		SDL_LockSurface(surface);
		auto surface_bmp = static_cast<uint8_t*>(surface->pixels); {
			for(int y=0; y<height(); y++) {
				auto row = surface_bmp + (y * surface->pitch);
				for(int x=0; x<width(); x++) {
					const auto color = dot_access(x,y);
					row[4*x + 0] = color.b;
					row[4*x + 1] = color.g;
					row[4*x + 2] = color.r;
				}
			}
		}
		SDL_UnlockSurface(surface);
	}
}

